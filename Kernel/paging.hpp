#pragma once

#include "memmgr.hpp"

namespace HailOS::Kernel
{
    bool buildIdentitiyMap2M(const MemoryManager::MemoryInfo* info, u64& outCr3Phys, u64 frameBufferBase, bool keepNullUnmapped = false);
    void checkVAMapped(u64 cr3);
}