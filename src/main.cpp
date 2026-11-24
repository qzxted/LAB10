#include "CallCommand.h"
#include "CallContext.h"
#include "CallObserver.h"
#include "Database.h"
#include "IPriorityStrategy.h"
#include "PriorityStrategies.h"
#include "SQLiteOperatorRepository.h"

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>

int main()
{
    try
    {
        const std::string dbPath = "callcenter_lab10.db";
        std::filesystem::remove(dbPath);

        SQLiteOperatorRepository repo(dbPath);
        int opId = repo.add(Operator{0, "Olena Koval", "Support", "active"});

        auto storage = createStorage(dbPath);
        storage.on_open = [](sqlite3* db) {
            if (sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr) != SQLITE_OK)
            {
                throw std::runtime_error("Failed to enable foreign_keys");
            }
        };
        storage.open_forever();
        storage.sync_schema();

        std::cout << "=== 1. COMMAND + STATE + OBSERVER ===\n";
        CallEventPublisher publisher;
        auto journal = std::make_shared<OperatorJournalObserver>();
        auto stats = std::make_shared<WorkStatisticsObserver>();
        publisher.subscribe(journal);
        publisher.subscribe(stats);

        CommandInvoker invoker;

        auto regCmd = std::make_unique<RegisterCallCommand>(
            storage, opId, "2026-11-25", "+380501112233", &publisher);
        int callId = 0;
        {
            RegisterCallCommand* rawReg = regCmd.get();
            invoker.runCommand(std::move(regCmd));
            callId = rawReg->createdCallId();
        }

        CallContext ctx(storage, callId, &publisher);

        try
        {
            ctx.complete(100, "Invalid direct complete");
        }
        catch (const std::logic_error& ex)
        {
            std::cout << "  [State check] Rejected invalid transition: " << ex.what() << '\n';
        }

        invoker.runCommand(std::make_unique<AcceptCallCommand>(ctx, "Operator accepted call"));
        invoker.runCommand(std::make_unique<CompleteCallCommand>(ctx, 185, "Issue resolved"));

        publisher.unsubscribe(journal);
        invoker.runCommand(std::make_unique<RegisterCallCommand>(
            storage, opId, "2026-11-25", "+380504445566", &publisher));

        std::cout << "  Commands executed: " << invoker.executedCount()
                  << ", journal entries: " << journal->entries().size()
                  << ", completed calls in stats: " << stats->completedCount() << '\n';

        std::cout << "\n=== 2. PRIORITY STRATEGIES ===\n";
        CallPriorityInput input{8.0, 9.0, 120.0};

        CallPriorityCalculator calc(std::make_unique<MaxPriorityStrategy>());
        std::cout << "  " << calc.strategyName() << ": "
                  << std::fixed << std::setprecision(2) << calc.compute(input) << '\n';

        calc.setStrategy(std::make_unique<WeightedSumPriorityStrategy>(2.0, 3.0, 0.1));
        std::cout << "  " << calc.strategyName() << ": " << calc.compute(input) << '\n';

        calc.setStrategy(std::make_unique<NormalizedWeightedPriorityStrategy>(0.3, 0.5, 0.2));
        std::cout << "  " << calc.strategyName() << ": " << calc.compute(input) << '\n';

        std::cout << "\n=== 3. STATE HISTORY IN SQLITE ===\n";
        using namespace sqlite_orm;
        auto historyRows = storage.get_all<CallStateHistory>(
            where(c(&CallStateHistory::callId) == callId),
            order_by(&CallStateHistory::id).asc()
        );
        for (const auto& h : historyRows)
        {
            std::cout << "  #" << h.id << ": " << h.fromState << " -> " << h.toState
                      << " (" << h.note << ")\n";
        }
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}