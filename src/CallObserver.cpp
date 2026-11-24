#include "CallObserver.h"

#include <algorithm>
#include <iostream>

void CallEventPublisher::subscribe(const std::shared_ptr<ICallObserver>& observer)
{
    if (!observer)
    {
        return;
    }

    for (const auto& weak : observers_)
    {
        if (auto locked = weak.lock(); locked && locked == observer)
        {
            return;
        }
    }

    observers_.push_back(observer);
}

void CallEventPublisher::unsubscribe(const std::shared_ptr<ICallObserver>& observer)
{
    if (!observer)
    {
        return;
    }

    std::erase_if(observers_, [&](const std::weak_ptr<ICallObserver>& weak) {
        auto locked = weak.lock();
        return !locked || locked == observer;
    });
}

void CallEventPublisher::notify(const CallEvent& event)
{
    std::erase_if(observers_, [](const std::weak_ptr<ICallObserver>& weak) {
        return weak.expired();
    });

    for (const auto& weak : observers_)
    {
        if (auto locked = weak.lock())
        {
            locked->onNotify(event);
        }
    }
}

std::size_t CallEventPublisher::subscriberCount() const
{
    std::size_t count = 0;
    for (const auto& weak : observers_)
    {
        if (!weak.expired())
        {
            ++count;
        }
    }
    return count;
}

void OperatorJournalObserver::onNotify(const CallEvent& event)
{
    std::string action = (event.type == CallEventType::Registered) ? "REGISTERED" : "COMPLETED";
    std::string line = "[Journal] " + action + " Call #" + std::to_string(event.callId) +
                       " (Operator #" + std::to_string(event.operatorId) +
                       ", phone=" + event.phoneNumber +
                       ", duration=" + std::to_string(event.durationSec) + "s)";
    if (!event.detail.empty())
    {
        line += " | " + event.detail;
    }
    entries_.push_back(line);
    std::cout << "  " << line << '\n';
}

const std::vector<std::string>& OperatorJournalObserver::entries() const
{
    return entries_;
}

void WorkStatisticsObserver::onNotify(const CallEvent& event)
{
    if (event.type == CallEventType::Registered)
    {
        ++registeredCount_;
        std::cout << "  [Stats] Registered calls total: " << registeredCount_ << '\n';
    }
    else if (event.type == CallEventType::Completed)
    {
        ++completedCount_;
        totalCompletedDurationSec_ += event.durationSec;
        std::cout << "  [Stats] Completed calls: " << completedCount_
                  << ", total completed duration: " << totalCompletedDurationSec_ << "s\n";
    }
}

int WorkStatisticsObserver::registeredCount() const
{
    return registeredCount_;
}

int WorkStatisticsObserver::completedCount() const
{
    return completedCount_;
}

int WorkStatisticsObserver::totalCompletedDurationSec() const
{
    return totalCompletedDurationSec_;
}