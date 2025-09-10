#pragma once

#include "basetype.hpp"
#include "timedef.hpp"

namespace HailOS::Utility::Timer
{
    u64 queryCurrentUNIXTime(void);
    u64 queryPerformanceCounter(void);
    i64 convertPerformanceCounterDeltaToMs(i64 delta);
    u64 querySystemUptime(void);
    void Sleep(u64 ms);
    void SleepNano(u64 ns);
    u64 queryPerformanceCounterFreq(void);
}