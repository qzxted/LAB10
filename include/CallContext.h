#pragma once

#include "Call.h"
#include "CallObserver.h"
#include "CallState.h"
#include "Database.h"

#include <memory>
#include <string>

class CallContext
{
public:
    CallContext(
        CallCenterStorage& storage,
        int callId,
        CallEventPublisher* publisher = nullptr);

    void accept(const std::string& note = "");
    void complete(int durationSec, const std::string& note = "");
    void cancel(const std::string& note = "");

    std::string currentStateCode() const;
    Call currentCall() const;
    void transitionTo(const std::string& nextStateCode, int newDurationSec, const std::string& note);

private:
    static std::string currentTimestamp();

    CallCenterStorage& storage_;
    Call call_;
    std::unique_ptr<ICallState> state_;
    CallEventPublisher* publisher_;
};