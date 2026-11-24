#include "CallCommand.h"

#include <stdexcept>
#include <utility>

RegisterCallCommand::RegisterCallCommand(
    CallCenterStorage& storage,
    int operatorId,
    std::string callDate,
    std::string phoneNumber,
    CallEventPublisher* publisher)
    : storage_(storage),
      operatorId_(operatorId),
      callDate_(std::move(callDate)),
      phoneNumber_(std::move(phoneNumber)),
      publisher_(publisher)
{
}

std::string RegisterCallCommand::name() const
{
    return "RegisterCallCommand";
}

void RegisterCallCommand::execute()
{
    Call newCall{0, operatorId_, callDate_, phoneNumber_, 0, "Registered"};
    int newId = 0;

    bool ok = storage_.transaction([&]() -> bool {
        newId = storage_.insert(newCall);
        CallStateHistory hist{0, newId, "None", "Registered", "now", "Call registered"};
        storage_.insert(hist);
        return true;
    });

    if (!ok)
    {
        throw std::runtime_error("Failed to register call in transaction");
    }

    createdCallId_ = newId;

    if (publisher_ != nullptr)
    {
        publisher_->notify(CallEvent{
            CallEventType::Registered,
            createdCallId_,
            operatorId_,
            phoneNumber_,
            0,
            "Registered in queue"
        });
    }
}

int RegisterCallCommand::createdCallId() const
{
    return createdCallId_;
}

AcceptCallCommand::AcceptCallCommand(CallContext& context, std::string note)
    : context_(context), note_(std::move(note))
{
}

std::string AcceptCallCommand::name() const
{
    return "AcceptCallCommand";
}

void AcceptCallCommand::execute()
{
    context_.accept(note_);
}

CompleteCallCommand::CompleteCallCommand(CallContext& context, int durationSec, std::string note)
    : context_(context), durationSec_(durationSec), note_(std::move(note))
{
}

std::string CompleteCallCommand::name() const
{
    return "CompleteCallCommand";
}

void CompleteCallCommand::execute()
{
    context_.complete(durationSec_, note_);
}

CancelCallCommand::CancelCallCommand(CallContext& context, std::string note)
    : context_(context), note_(std::move(note))
{
}

std::string CancelCallCommand::name() const
{
    return "CancelCallCommand";
}

void CancelCallCommand::execute()
{
    context_.cancel(note_);
}

void CommandInvoker::runCommand(std::unique_ptr<ICallCommand> command)
{
    if (!command)
    {
        throw std::invalid_argument("Command cannot be null");
    }
    command->execute();
    names_.push_back(command->name());
    history_.push_back(std::move(command));
}

std::size_t CommandInvoker::executedCount() const
{
    return history_.size();
}

const std::vector<std::string>& CommandInvoker::executedNames() const
{
    return names_;
}