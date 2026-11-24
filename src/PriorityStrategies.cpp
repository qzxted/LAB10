#include "PriorityStrategies.h"

#include <algorithm>
#include <stdexcept>

std::string MaxPriorityStrategy::name() const
{
    return "MaxPriorityStrategy";
}

double MaxPriorityStrategy::calculatePriority(const CallPriorityInput& input) const
{
    return std::max({input.importance, input.urgency, input.waitTime});
}

WeightedSumPriorityStrategy::WeightedSumPriorityStrategy(double wImportance, double wUrgency, double wWaitTime)
    : wImportance_(wImportance), wUrgency_(wUrgency), wWaitTime_(wWaitTime)
{
}

std::string WeightedSumPriorityStrategy::name() const
{
    return "WeightedSumPriorityStrategy";
}

double WeightedSumPriorityStrategy::calculatePriority(const CallPriorityInput& input) const
{
    return input.importance * wImportance_ +
           input.urgency * wUrgency_ +
           input.waitTime * wWaitTime_;
}

NormalizedWeightedPriorityStrategy::NormalizedWeightedPriorityStrategy(
    double wImportance,
    double wUrgency,
    double wWaitTime,
    double maxImportance,
    double maxUrgency,
    double maxWaitTime)
    : wImportance_(wImportance),
      wUrgency_(wUrgency),
      wWaitTime_(wWaitTime),
      maxImportance_(maxImportance > 0.0 ? maxImportance : 1.0),
      maxUrgency_(maxUrgency > 0.0 ? maxUrgency : 1.0),
      maxWaitTime_(maxWaitTime > 0.0 ? maxWaitTime : 1.0)
{
    double weightSum = wImportance_ + wUrgency_ + wWaitTime_;
    if (weightSum <= 0.0)
    {
        throw std::invalid_argument("Sum of strategy weights must be positive");
    }
    normImportance_ = wImportance_ / weightSum;
    normUrgency_ = wUrgency_ / weightSum;
    normWaitTime_ = wWaitTime_ / weightSum;
}

std::string NormalizedWeightedPriorityStrategy::name() const
{
    return "NormalizedWeightedPriorityStrategy";
}

double NormalizedWeightedPriorityStrategy::calculatePriority(const CallPriorityInput& input) const
{
    double nImp = std::clamp(input.importance / maxImportance_, 0.0, 1.0);
    double nUrg = std::clamp(input.urgency / maxUrgency_, 0.0, 1.0);
    double nWait = std::clamp(input.waitTime / maxWaitTime_, 0.0, 1.0);
    return (nImp * normImportance_ + nUrg * normUrgency_ + nWait * normWaitTime_) * 100.0;
}

CallPriorityCalculator::CallPriorityCalculator(std::unique_ptr<IPriorityStrategy> strategy)
    : strategy_(std::move(strategy))
{
    if (!strategy_)
    {
        throw std::invalid_argument("Priority strategy cannot be null");
    }
}

void CallPriorityCalculator::setStrategy(std::unique_ptr<IPriorityStrategy> strategy)
{
    if (!strategy)
    {
        throw std::invalid_argument("Priority strategy cannot be null");
    }
    strategy_ = std::move(strategy);
}

std::string CallPriorityCalculator::strategyName() const
{
    return strategy_->name();
}

double CallPriorityCalculator::compute(const CallPriorityInput& input) const
{
    return strategy_->calculatePriority(input);
}