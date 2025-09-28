#pragma once

#include "memdef.hpp"

namespace HailOS::MemoryManager
{
    bool initMemoryManager(MemoryInfo* info);
    void showStat();
    void ReserveRange(MemoryInfo& info, u64 base, u64 length);
    void initPageTableAllocator(MemoryInfo& info);
    void* ptAlloc4K();
    void* convertVirtToPhys(void* virt);
}
