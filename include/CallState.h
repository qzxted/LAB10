#pragma once

#include <memory>
#include <string>

class CallContext;

class ICallState
{
public:
    virtual ~ICallState() = default;
    virtual std::string code() const = 0;
    virtual void accept(CallContext& context, const std::string& note) = 0;
    virtual void complete(CallContext& context, int durationSec, const std::string& note) = 0;
    virtual void cancel(CallContext& context, const std::string& note) = 0;
};

std::unique_ptr<ICallState> makeStateByCode(const std::string& stateCode);