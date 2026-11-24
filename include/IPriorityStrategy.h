#pragma once

#include <memory>
#include <string>

struct CallPriorityInput
{
    double importance = 0.0;
    double urgency = 0.0;
    double waitTime = 0.0;
};

class IPriorityStrategy
{
public:
    virtual ~IPriorityStrategy() = default;
    virtual std::string name() const = 0;
    virtual double calculatePriority(const CallPriorityInput& input) const = 0;
};

class CallPriorityCalculator
{
public:
    explicit CallPriorityCalculator(std::unique_ptr<IPriorityStrategy> strategy);
    void setStrategy(std::unique_ptr<IPriorityStrategy> strategy);
    std::string strategyName() const;
    double compute(const CallPriorityInput& input) const;

private:
    std::unique_ptr<IPriorityStrategy> strategy_;
};