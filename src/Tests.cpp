#include "CallCommand.h"
#include "CallContext.h"
#include "CallObserver.h"
#include "Database.h"
#include "IPriorityStrategy.h"
#include "PriorityStrategies.h"
#include "SQLiteOperatorRepository.h"

#include <cassert>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>

namespace
{
    bool nearlyEqual(double a, double b, double eps = 1e-4)
    {
        return std::fabs(a - b) <= eps;
    }

    void testStrategies()
    {
        CallPriorityInput input{6.0, 8.0, 150.0};

        CallPriorityCalculator calc(std::make_unique<MaxPriorityStrategy>());
        assert(nearlyEqual(calc.compute(input), 150.0));

        calc.setStrategy(std::make_unique<WeightedSumPriorityStrategy>(1.5, 2.0, 0.2));
        assert(nearlyEqual(calc.compute(input), 55.0));

        calc.setStrategy(std::make_unique<NormalizedWeightedPriorityStrategy>(0.25, 0.5, 0.25, 10.0, 10.0, 300.0));
        assert(nearlyEqual(calc.compute(input), 67.5));
    }

    void testObserverSubscriptionAndUnsubscription()
    {
        CallEventPublisher pub;
        auto journal = std::make_shared<OperatorJournalObserver>();
        auto stats = std::make_shared<WorkStatisticsObserver>();

        pub.subscribe(journal);
        pub.subscribe(stats);
        assert(pub.subscriberCount() == 2);

        pub.notify(CallEvent{CallEventType::Registered, 1, 1, "+380501234567", 0, ""});
        assert(journal->entries().size() == 1);
        assert(stats->registeredCount() == 1);

        pub.unsubscribe(journal);
        assert(pub.subscriberCount() == 1);

        pub.notify(CallEvent{CallEventType::Completed, 1, 1, "+380501234567", 90, ""});
        assert(journal->entries().size() == 1);
        assert(stats->completedCount() == 1);
        assert(stats->totalCompletedDurationSec() == 90);
    }

    void testStateCommandAndHistoryPersistence()
    {
        const std::string testDb = "test_lab10_callcenter.db";
        std::filesystem::remove(testDb);

        SQLiteOperatorRepository repo(testDb);
        int opId = repo.add(Operator{0, "Test Operator", "Support", "active"});

        auto storage = createStorage(testDb);
        storage.on_open = [](sqlite3* db) {
            if (sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr) != SQLITE_OK)
            {
                throw std::runtime_error("PRAGMA foreign_keys failed");
            }
        };
        storage.open_forever();
        storage.sync_schema();

        CallEventPublisher pub;
        auto stats = std::make_shared<WorkStatisticsObserver>();
        pub.subscribe(stats);

        CommandInvoker invoker;
        auto regCmd = std::make_unique<RegisterCallCommand>(storage, opId, "2026-11-25", "+380991112233", &pub);
        RegisterCallCommand* regPtr = regCmd.get();
        invoker.runCommand(std::move(regCmd));
        int callId = regPtr->createdCallId();
        assert(callId > 0);
        assert(stats->registeredCount() == 1);

        CallContext ctx(storage, callId, &pub);
        assert(ctx.currentStateCode() == "Registered");

        bool threwInvalid = false;
        try
        {
            ctx.complete(120, "Should fail");
        }
        catch (const std::logic_error&)
        {
            threwInvalid = true;
        }
        assert(threwInvalid);
        assert(ctx.currentStateCode() == "Registered");

        invoker.runCommand(std::make_unique<AcceptCallCommand>(ctx, "Accepted"));
        assert(ctx.currentStateCode() == "Accepted");

        invoker.runCommand(std::make_unique<CompleteCallCommand>(ctx, 240, "Done"));
        assert(ctx.currentStateCode() == "Completed");
        assert(stats->completedCount() == 1);
        assert(stats->totalCompletedDurationSec() == 240);

        CallContext reloaded(storage, callId, &pub);
        assert(reloaded.currentStateCode() == "Completed");
        assert(reloaded.currentCall().durationSec == 240);

        using namespace sqlite_orm;
        auto hist = storage.get_all<CallStateHistory>(
            where(c(&CallStateHistory::callId) == callId),
            order_by(&CallStateHistory::id).asc()
        );
        assert(hist.size() == 3);
        assert(hist[0].fromState == "None" && hist[0].toState == "Registered");
        assert(hist[1].fromState == "Registered" && hist[1].toState == "Accepted");
        assert(hist[2].fromState == "Accepted" && hist[2].toState == "Completed");

        int call2Id = storage.insert(Call{0, opId, "2026-11-25", "+380990000000", 0, "Registered"});
        CallContext ctx2(storage, call2Id, &pub);
        invoker.runCommand(std::make_unique<CancelCallCommand>(ctx2, "User hung up"));
        assert(ctx2.currentStateCode() == "Cancelled");
    }
}

int main()
{
    try
    {
        testStrategies();
        testObserverSubscriptionAndUnsubscription();
        testStateCommandAndHistoryPersistence();
        std::cout << "All Lab 10 unit tests passed successfully.\n";
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Test failed: " << ex.what() << '\n';
        return 1;
    }
    return 0;
}