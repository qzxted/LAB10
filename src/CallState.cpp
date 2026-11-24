#include "CallState.h"
#include "CallContext.h"

#include <stdexcept>

class RegisteredState final : public ICallState
{
public:
    std::string code() const override { return "Registered"; }

    void accept(CallContext& context, const std::string& note) override
    {
        context.transitionTo("Accepted", 0, note.empty() ? "Call accepted by operator" : note);
    }

    void complete(CallContext&, int, const std::string&) override
    {
        throw std::logic_error("Invalid state transition: Registered -> Completed");
    }

    void cancel(CallContext& context, const std::string& note) override
    {
        context.transitionTo("Cancelled", 0, note.empty() ? "Call cancelled while registered" : note);
    }
};

class AcceptedState final : public ICallState
{
public:
    std::string code() const override { return "Accepted"; }

    void accept(CallContext&, const std::string&) override
    {
        throw std::logic_error("Invalid state transition: Accepted -> Accepted");
    }

    void complete(CallContext& context, int durationSec, const std::string& note) override
    {
        if (durationSec < 0)
        {
            throw std::invalid_argument("Call duration cannot be negative");
        }
        context.transitionTo("Completed", durationSec, note.empty() ? "Call completed" : note);
    }

    void cancel(CallContext& context, const std::string& note) override
    {
        context.transitionTo("Cancelled", 0, note.empty() ? "Call cancelled during conversation" : note);
    }
};

class CompletedState final : public ICallState
{
public:
    std::string code() const override { return "Completed"; }

    void accept(CallContext&, const std::string&) override
    {
        throw std::logic_error("Invalid state transition: Completed -> Accepted");
    }

    void complete(CallContext&, int, const std::string&) override
    {
        throw std::logic_error("Invalid state transition: Completed -> Completed");
    }

    void cancel(CallContext&, const std::string&) override
    {
        throw std::logic_error("Invalid state transition: Completed -> Cancelled");
    }
};

class CancelledState final : public ICallState
{
public:
    std::string code() const override { return "Cancelled"; }

    void accept(CallContext&, const std::string&) override
    {
        throw std::logic_error("Invalid state transition: Cancelled -> Accepted");
    }

    void complete(CallContext&, int, const std::string&) override
    {
        throw std::logic_error("Invalid state transition: Cancelled -> Completed");
    }

    void cancel(CallContext&, const std::string&) override
    {
        throw std::logic_error("Invalid state transition: Cancelled -> Cancelled");
    }
};

std::unique_ptr<ICallState> makeStateByCode(const std::string& stateCode)
{
    if (stateCode == "Registered")
    {
        return std::make_unique<RegisteredState>();
    }
    if (stateCode == "Accepted")
    {
        return std::make_unique<AcceptedState>();
    }
    if (stateCode == "Completed")
    {
        return std::make_unique<CompletedState>();
    }
    if (stateCode == "Cancelled")
    {
        return std::make_unique<CancelledState>();
    }
    throw std::invalid_argument("Unknown call state: " + stateCode);
}