#pragma once

#include "memdef.hpp"
#include "basetype.hpp"

namespace HailOS::MemoryManager
{
    void* alloc(size_t size);
    void *allocAligned(size_t size, size_t align);
    void free(void *ptr, size_t size);
    void* allocInitializedMemory(size_t size, u8 value);
    void fill(void* ptr, size_t size, u8 value);
    bool memeq(const void* mem1, const void* mem2, size_t size);
    void memcopy(void* dest, const void* src, size_t size);
    size_t queryAvailableMemorySize(void);
    size_t queryLargestMemoryRegion(void);
}