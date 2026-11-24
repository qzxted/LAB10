#pragma once

#include "IPriorityStrategy.h"

class MaxPriorityStrategy final : public IPriorityStrategy
{
public:
    std::string name() const override;
    double calculatePriority(const CallPriorityInput& input) const override;
};

class WeightedSumPriorityStrategy final : public IPriorityStrategy
{
public:
    WeightedSumPriorityStrategy(double wImportance, double wUrgency, double wWaitTime);
    std::string name() const override;
    double calculatePriority(const CallPriorityInput& input) const override;

private:
    double wImportance_;
    double wUrgency_;
    double wWaitTime_;
};

class NormalizedWeightedPriorityStrategy final : public IPriorityStrategy
{
public:
    NormalizedWeightedPriorityStrategy(
        double wImportance,
        double wUrgency,
        double wWaitTime,
        double maxImportance = 10.0,
        double maxUrgency = 10.0,
        double maxWaitTime = 300.0);

    std::string name() const override;
    double calculatePriority(const CallPriorityInput& input) const override;

private:
    double wImportance_;
    double wUrgency_;
    double wWaitTime_;
    double maxImportance_;
    double maxUrgency_;
    double maxWaitTime_;
    double normImportance_ = 0.0;
    double normUrgency_ = 0.0;
    double normWaitTime_ = 0.0;
};