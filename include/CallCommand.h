#pragma once

#include "CallContext.h"
#include "CallObserver.h"
#include "Database.h"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

class ICallCommand
{
public:
    virtual ~ICallCommand() = default;
    virtual std::string name() const = 0;
    virtual void execute() = 0;
};

class RegisterCallCommand final : public ICallCommand
{
public:
    RegisterCallCommand(
        CallCenterStorage& storage,
        int operatorId,
        std::string callDate,
        std::string phoneNumber,
        CallEventPublisher* publisher = nullptr);

    std::string name() const override;
    void execute() override;
    int createdCallId() const;

private:
    CallCenterStorage& storage_;
    int operatorId_;
    std::string callDate_;
    std::string phoneNumber_;
    CallEventPublisher* publisher_;
    int createdCallId_ = 0;
};

class AcceptCallCommand final : public ICallCommand
{
public:
    AcceptCallCommand(CallContext& context, std::string note = "");
    std::string name() const override;
    void execute() override;

private:
    CallContext& context_;
    std::string note_;
};

class CompleteCallCommand final : public ICallCommand
{
public:
    CompleteCallCommand(CallContext& context, int durationSec, std::string note = "");
    std::string name() const override;
    void execute() override;

private:
    CallContext& context_;
    int durationSec_;
    std::string note_;
};

class CancelCallCommand final : public ICallCommand
{
public:
    CancelCallCommand(CallContext& context, std::string note = "");
    std::string name() const override;
    void execute() override;

private:
    CallContext& context_;
    std::string note_;
};

class CommandInvoker
{
public:
    void runCommand(std::unique_ptr<ICallCommand> command);
    std::size_t executedCount() const;
    const std::vector<std::string>& executedNames() const;

private:
    std::vector<std::unique_ptr<ICallCommand>> history_;
    std::vector<std::string> names_;
};