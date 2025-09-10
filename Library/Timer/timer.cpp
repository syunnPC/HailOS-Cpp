#include "timedef.hpp"
#include "basetype.hpp"
#include "tsc.hpp"

namespace HailOS::Utility::Timer
{
    static HardwareClockInfo* sClockInfo = nullptr;

    bool initTime(HardwareClockInfo* info)
    {
        if(info == nullptr)
        {
            return false;
        }

        if(sClockInfo == nullptr)
        {
            sClockInfo = info;
        }

        return true;
    }

    u64 queryCurrentUNIXTime(void)
    {
        if(sClockInfo == nullptr)
        {
            return 0;
        }

        u64 current = HailOS::IO::TSC::read();
        u64 delta = current - sClockInfo->InitialTSC;
        u64 sec = delta / sClockInfo->TSCFreq;
        return sClockInfo->InitialUNIXTime+sec;
    }

    u64 queryPerformanceCounter(void)
    {
        return HailOS::IO::TSC::read();
    }

    i64 convertPerformanceCounterDeltaToMs(i64 delta)
    {
        if(sClockInfo == nullptr)
        {
            return 0;
        }

        i64 freq_ms = sClockInfo->TSCFreq / 1000;
        return delta/freq_ms;
    }

    u64 querySystemUpTime(void)
    {
        if(sClockInfo == nullptr)
        {
            return 0;
        }

        return sClockInfo->InitialUNIXTime - queryCurrentUNIXTime();
    }

    void Sleep(u64 ms)
    {
        if(sClockInfo == nullptr)
        {
            return;
        }

        u64 freq = static_cast<u64>(sClockInfo->TSCFreq / 1000);
        u64 start = HailOS::IO::TSC::read();
        while(HailOS::IO::TSC::read() < start + freq * ms);
    }

    void SleepNano(u64 ns)
    {
        if(sClockInfo == nullptr)
        {
            return;
        }

        double cpns = sClockInfo->TSCFreq / 1000000000;
        u64 waitclk = static_cast<u64>(ns * cpns);
        u64 start = HailOS::IO::TSC::read();
        while(HailOS::IO::TSC::read() < start + waitclk);
    }

    u64 queryPerformanceCounterFreq(void)
    {
        if(sClockInfo == nullptr)
        {
            return 0;
        }
        
        return sClockInfo->TSCFreq;
    }
}