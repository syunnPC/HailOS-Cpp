#pragma once

#include "basetype.hpp"

namespace HailOS::MemoryManager
{
    constexpr auto PAGE_SIZE = 4096;
    constexpr auto MAX_REGIONS = 64;

    struct MemoryRegion
    {
        addr_t Base;
        size_t Length;
    } PACKED;

    struct MemoryInfo
    {
        MemoryRegion FreeMemory[MAX_REGIONS];
        u64 FreeRegionCount;
        MemoryRegion ReservedMemory[MAX_REGIONS];
        u64 ReservedRegionCount;
    } PACKED;
}
