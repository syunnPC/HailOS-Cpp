#pragma once

#include "basetype.hpp"

namespace HailOS::Utility::Time
{
    struct HardwareClockInfo
    {
        u64 InitialUNIXTime;
        u64 InitialTSC;
        u64 TSCFreq;
    } PACKED;
}