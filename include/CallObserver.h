#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

enum class CallEventType
{
    Registered,
    Completed
};

struct CallEvent
{
    CallEventType type;
    int callId = 0;
    int operatorId = 0;
    std::string phoneNumber;
    int durationSec = 0;
    std::string detail;
};

class ICallObserver
{
public:
    virtual ~ICallObserver() = default;
    virtual void onNotify(const CallEvent& event) = 0;
};

class CallEventPublisher
{
public:
    void subscribe(const std::shared_ptr<ICallObserver>& observer);
    void unsubscribe(const std::shared_ptr<ICallObserver>& observer);
    void notify(const CallEvent& event);
    std::size_t subscriberCount() const;

private:
    std::vector<std::weak_ptr<ICallObserver>> observers_;
};

class OperatorJournalObserver final : public ICallObserver
{
public:
    void onNotify(const CallEvent& event) override;
    const std::vector<std::string>& entries() const;

private:
    std::vector<std::string> entries_;
};

class WorkStatisticsObserver final : public ICallObserver
{
public:
    void onNotify(const CallEvent& event) override;
    int registeredCount() const;
    int completedCount() const;
    int totalCompletedDurationSec() const;

private:
    int registeredCount_ = 0;
    int completedCount_ = 0;
    int totalCompletedDurationSec_ = 0;
};