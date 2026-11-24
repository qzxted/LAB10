#include "CallContext.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>

CallContext::CallContext(
    CallCenterStorage& storage,
    int callId,
    CallEventPublisher* publisher)
    : storage_(storage), publisher_(publisher)
{
    auto found = storage_.get_optional<Call>(callId);
    if (!found.has_value())
    {
        throw std::runtime_error("Call not found with ID: " + std::to_string(callId));
    }
    call_ = *found;
    state_ = makeStateByCode(call_.status);
}

void CallContext::accept(const std::string& note)
{
    state_->accept(*this, note);
}

void CallContext::complete(int durationSec, const std::string& note)
{
    state_->complete(*this, durationSec, note);
}

void CallContext::cancel(const std::string& note)
{
    state_->cancel(*this, note);
}

std::string CallContext::currentStateCode() const
{
    return state_->code();
}

Call CallContext::currentCall() const
{
    return call_;
}

void CallContext::transitionTo(const std::string& nextStateCode, int newDurationSec, const std::string& note)
{
    std::string previousState = call_.status;
    int updatedDuration = (nextStateCode == "Completed") ? newDurationSec : call_.durationSec;
    std::string timestamp = currentTimestamp();

    bool committed = storage_.transaction([&]() -> bool {
        Call updated = call_;
        updated.status = nextStateCode;
        updated.durationSec = updatedDuration;
        storage_.update(updated);

        CallStateHistory historyEntry{
            0,
            call_.id,
            previousState,
            nextStateCode,
            timestamp,
            note
        };
        storage_.insert(historyEntry);
        return true;
    });

    if (!committed)
    {
        throw std::runtime_error("Failed to commit call state transition transaction");
    }

    call_.status = nextStateCode;
    call_.durationSec = updatedDuration;
    state_ = makeStateByCode(nextStateCode);

    if (publisher_ != nullptr && nextStateCode == "Completed")
    {
        publisher_->notify(CallEvent{
            CallEventType::Completed,
            call_.id,
            call_.operatorId,
            call_.phoneNumber,
            call_.durationSec,
            note
        });
    }
}

std::string CallContext::currentTimestamp()
{
    auto now = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm tmBuf{};
#if defined(_WIN32)
    localtime_s(&tmBuf, &tt);
#else
    localtime_r(&tt, &tmBuf);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tmBuf, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}