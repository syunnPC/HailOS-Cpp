#include "memdef.hpp"
#include "basetype.hpp"
#include "kernellib.hpp"
#include "console.hpp"
#include "cstring.hpp"

namespace HailOS::MemoryManager
{
    static constexpr auto ALLOC_ALIGN = 8;

    static MemoryInfo* sMemoryInfo = nullptr;

    bool initMemoryManager(MemoryInfo* info)
    {
        if(info == nullptr)
        {
            return false;
        }
        
        if(sMemoryInfo == nullptr)
        {
            sMemoryInfo = info;
        }

        return true;
    }

    void* alloc(size_t size)
    {
        if(size == 0)
        {
            return nullptr;
        }

        if(sMemoryInfo == nullptr)
        {
            return nullptr;
        }

        Kernel::Utility::disableInterrupts();

        size = (size + ALLOC_ALIGN - 1) & ~(ALLOC_ALIGN - 1);

        for(size_t i=0; i<sMemoryInfo->FreeRegionCount; i++)
        {
            addr_t base = sMemoryInfo->FreeMemory[i].Base;
            u64 len = sMemoryInfo->FreeMemory[i].Length;

            addr_t aligned_base = (base + ALLOC_ALIGN - 1) &~(ALLOC_ALIGN - 1);
            u64 padding = aligned_base - base;

            if(len < size + padding)
            {
                continue;
            }

            void* allocated = reinterpret_cast<void*>(aligned_base);

            sMemoryInfo->FreeMemory[i].Base = aligned_base + size;
            sMemoryInfo->FreeMemory[i].Length = len - (size + padding);

            Kernel::Utility::enableInterrupts();

            return allocated;
        }

        Kernel::Utility::enableInterrupts();

        return nullptr;
    }

    void free(void* ptr, size_t size)
    {
        if(ptr == nullptr || size == 0 || sMemoryInfo == nullptr)
        {
            return;
        }

        Kernel::Utility::disableInterrupts();

        if(sMemoryInfo->FreeRegionCount < MAX_FREE_REGIONS)
        {
            sMemoryInfo->FreeMemory[sMemoryInfo->FreeRegionCount++] = (FreeRegion){.Base = reinterpret_cast<addr_t>(ptr), .Length = size};
        }

        Kernel::Utility::enableInterrupts();
    }

    size_t queryAvailableMemorySize(void)
    {
        if(sMemoryInfo == nullptr)
        {
            return 0;
        }

        size_t result = 0;
        for(size_t i=0; i<sMemoryInfo->FreeRegionCount; i++)
        {
            result += sMemoryInfo->FreeMemory[i].Length;
        }

        return result;
    }

    size_t queryLargestMemoryRegion(void)
    {
        if(sMemoryInfo == nullptr)
        {
            return 0;
        }

        size_t result = 0;
        for(size_t i=0; i<sMemoryInfo->FreeRegionCount; i++)
        {
            if(sMemoryInfo->FreeMemory[i].Length > result)
            {
                result = sMemoryInfo->FreeMemory[i].Length;
            }
        }

        return result;
    }

    void showStat()
    {
        Console::puts("Region count = ");
        Console::puts(StdLib::C::utos(sMemoryInfo->FreeRegionCount));
        Console::puts("\n");
        for (size_t i = 0; i < sMemoryInfo->FreeRegionCount; i++)
        {
            Console::puts("FreeRegion ");
            Console::puts(StdLib::C::utos(i));
            Console::puts(" : Base addr = ");
            Console::puts(StdLib::C::utohexstr(sMemoryInfo->FreeMemory[i].Base));
            Console::puts(" , Length = ");
            Console::puts(StdLib::C::utohexstr(sMemoryInfo->FreeMemory[i].Length));
            Console::puts("\n");
        }
    }
}