#include "memdef.hpp"
#include "basetype.hpp"
#include "kernellib.hpp"
#include "console.hpp"
#include "cstring.hpp"
#include "init.hpp"
#include "memutil.hpp"
#include "kd.hpp"

namespace HailOS::MemoryManager
{
    static constexpr auto ALLOC_ALIGN = 8;

    static MemoryInfo* sMemoryInfo = nullptr;

    static inline u64 end(const MemoryRegion& r)
    {
        return r.Base + r.Length;
    }

    static inline bool overlap(u64 b0, u64 e0, u64 b1, u64 e1)
    {
        return (b0<e1) && (b1 < e0);
    }

    static inline bool pushFree(MemoryInfo& info, MemoryRegion region)
    {
        if(region.Length == 0)
        {
            return true;
        }

        if(info.FreeRegionCount >= MAX_REGIONS)
        {
            return false;
        }

        info.FreeMemory[info.FreeRegionCount++] = region;
        return true;
    }

    static inline bool pushReserved(MemoryInfo& info, MemoryRegion region)
    {
        if(region.Length == 0)
        {
            return true;
        }

        if(info.FreeRegionCount >= MAX_REGIONS)
        {
            return false;
        }

        info.ReservedMemory[info.ReservedRegionCount++] = region;
        return true;
    }

    void ReserveRange(MemoryInfo& info, u64 base, u64 length)
    {
        u64 rb = base;
        u64 re = base + length;

        for(u64 i=0; i<info.FreeRegionCount;)
        {
            MemoryRegion r = info.FreeMemory[i];
            u64 fb = r.Base;
            u64 fe = end(r);
            if(!overlap(fb, fe, rb, re))
            {
                i++;
                continue;
            }

            MemoryRegion left = {fb, (rb>fb)?(rb-fb) : 0};
            MemoryRegion right = {(re<fe)?re:fe, (re<fe)?(fe-re) : 0};

            info.FreeMemory[i] = info.FreeMemory[info.FreeRegionCount - 1];
            info.FreeRegionCount--;

            bool result = true;
            if(left.Length != 0)
            {
                result &= pushFree(info, left);
            }
            if(right.Length != 0)
            {
                result &= pushFree(info, right);
            }
            
            if(!result)
            {
                break;
            }
        }

        pushReserved(info, {rb, length});
    }

    void* convertVirtToPhys(void* virt)
    {
        //1:1マップなので Virt == Phys
        return virt;
    }

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

    static constexpr u64 PT_POOL_SIZE = 8 * 1024 * 1024;
    static u64 sPTBase, sPTPoolEnd, sPTPtr;

    static inline u64 alignUp(u64 x, u64 a)
    {
        return (x + a - 1) & ~(a - 1);
    }

    void initPageTableAllocator(MemoryInfo& info)
    {
        u64 base = alignUp(reinterpret_cast<u64>(Kernel::Boot::_kernel_end), 0x1000);
        ReserveRange(info, base, PT_POOL_SIZE);
        sPTBase = base;
        sPTPoolEnd = base + PT_POOL_SIZE;
        sPTPtr = base;
    }

    void* ptAlloc4K()
    {
        sPTPtr = alignUp(sPTPtr, 0x1000);
        if(sPTPtr + 4096 > sPTPoolEnd)
        {
            return nullptr;
        }

        void* p = reinterpret_cast<void*>(sPTPtr);
        sPTPtr += 4096;
        fill(p, 4096, 0);
        return p;
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
            if(aligned_base == 0) //nullをアロケートすると、無効だと勘違いされるので
            {
                aligned_base = ALLOC_ALIGN;
            }
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

    void* allocAligned(size_t size, size_t align)
    {
        if(size == 0 || align == 0 || (align & (align - 1)) != 0)
        {
            return nullptr;
        }

        if(sMemoryInfo == nullptr)
        {
            return nullptr;
        }

        Kernel::Utility::disableInterrupts();

        for(size_t i=0; i<sMemoryInfo->FreeRegionCount; i++)
        {
            addr_t base = sMemoryInfo->FreeMemory[i].Base;
            u64 len = sMemoryInfo->FreeMemory[i].Length;

            addr_t aligned_base = (base + align - 1) & ~(align - 1);
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

        if(sMemoryInfo->FreeRegionCount < MAX_REGIONS)
        {
            sMemoryInfo->FreeMemory[sMemoryInfo->FreeRegionCount++] = (MemoryRegion){.Base = reinterpret_cast<addr_t>(ptr), .Length = size};
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