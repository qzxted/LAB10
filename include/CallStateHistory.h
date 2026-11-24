#pragma once

#include <string>

struct CallStateHistory
{
    int id = 0;
    int callId = 0;
    std::string fromState;
    std::string toState;
    std::string changedAt;
    std::string note;
};