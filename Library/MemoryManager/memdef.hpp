#pragma once

#include "basetype.hpp"

namespace HailOS::MemoryManager
{
    constexpr auto PAGE_SIZE = 4096;
    constexpr auto MAX_FREE_REGIONS = 64;

    struct FreeRegion
    {
        addr_t Base;
        size_t Length;
    } PACKED;

    struct MemoryInfo
    {
        FreeRegion FreeMemory[MAX_FREE_REGIONS];
        u64 FreeRegionCount;
    } PACKED;
}
