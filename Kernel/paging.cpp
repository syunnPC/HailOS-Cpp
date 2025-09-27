#include "basetype.hpp"
#include "memmgr.hpp"
#include "memdef.hpp"
#include "memutil.hpp"
#include "init.hpp"
#include "kernel_def.hpp"
#include "kdconsole.hpp"
#include "cstring.hpp"

namespace HailOS::Kernel
{
    static constexpr u64 PG_P = 1ull << 0;
    static constexpr u64 PG_RW = 1ull << 1;
    static constexpr u64 PG_PS = 1ull << 7;
    static constexpr u64 PG_PWT = 1ull << 3;
    static constexpr u64 PG_PCD = 1ull << 4;
    static constexpr u64 PG_ADDR_MASK = 0x000FFFFFFFFFF000ull;

    static constexpr u64 PAGE_4K = 0x1000;
    static constexpr u64 PAGE_2M = 0x200000;
    static constexpr u64 PAGE_1G = 0x40000000;
    static constexpr u64 ENTIRIES = 512;

    static inline u64 alignUp(u64 x, u64 a)
    {
        return (x + a - 1) & ~(a - 1);
    }

    struct BuiltTables
    {
        void* Pml4;
        u64 Pml4Phys;
    };

    static u64 calcMaxPhys(const MemoryManager::MemoryInfo* info)
    {
        u64 max = 0;
        for(u64 i=0; i<info->FreeRegionCount; i++)
        {
            u64 end = info->FreeMemory[i].Base + info->FreeMemory[i].Length;
            if(end > max)
            {
                max = end;
            }
        }
        for(u64 i=0; i<info->ReservedRegionCount; i++)
        {
            u64 end = info->ReservedMemory[i].Base + info->ReservedMemory[i].Length;
            if(end > max)
            {
                max = end;
            }
        }

        return (max < 0x200000ull) ? 0x200000ull : max;
    }

    static u64 computeMapEnd(const MemoryManager::MemoryInfo* info)
    {
        u64 phys_max = calcMaxPhys(info);
        if(phys_max < 0x100000000ull)
        {
            phys_max = 0x100000000ull;
        }

        return (phys_max + (2 * 1024 * 1024 - 1)) & ~(2* 1024 * 1024 - 1);
    }

    static bool isMMIORegion(u64 phys, const MemoryManager::MemoryInfo* info)
    {   
        for(u64 i=0; i<info->ReservedRegionCount; i++)
        {
            u64 base = info->ReservedMemory[i].Base;
            u64 end = base+info->ReservedMemory[i].Length;
            if(phys >= base && phys < end)
            {
                return true;
            }
        }

        return false;
    }

    bool buildIdentitiyMap2M(const MemoryManager::MemoryInfo* info, u64& outCr3Phys, u64 frameBufferBase, bool keepNullUnmapped = false)
    {
        if(info == nullptr)
        {
            return false;
        }

        u64 max_phys = computeMapEnd(info);
        Kernel::DebugConsole::printStringRawDbg("Map end phys = ");
        Kernel::DebugConsole::printStringRawDbg(StdLib::C::utohexstr(max_phys));
        Kernel::DebugConsole::printStringRawDbg("\n");
        u64 lim = alignUp(max_phys, PAGE_2M);

        u64 gib_cnt = alignUp(max_phys, PAGE_1G) / PAGE_1G;
        if(gib_cnt > ENTIRIES)
        {
            return false;
        }

        void* pml4 = MemoryManager::ptAlloc4K();
        if(pml4 == nullptr)
        {
            return false;
        }

        void* pdpt = MemoryManager::ptAlloc4K();
        if(pdpt == nullptr)
        {
            return false;
        }

        reinterpret_cast<u64*>(pml4)[0] = (reinterpret_cast<u64>(pdpt) & PG_ADDR_MASK) | PG_P | PG_RW;

        for(u64 g=0; g<gib_cnt; g++)
        {
            void* pd = MemoryManager::ptAlloc4K();
            if(pd == nullptr)
            {
                return false;
            }
            MemoryManager::fill(pd, PAGE_4K, 0);
            reinterpret_cast<u64*>(pdpt)[g] = (reinterpret_cast<u64>(pd) & PG_ADDR_MASK) | PG_P | PG_RW;

            u64 g_base = g * PAGE_1G;

            for(u64 i=0; i<ENTIRIES; i++)
            {
                u64 phys = g_base + i * PAGE_2M;
                if(phys >= lim)
                {
                    break;
                }

                if(keepNullUnmapped && phys == 0)
                {
                    continue;
                }

                u64 flags = PG_P | PG_RW | PG_PS;

                if(isMMIORegion(phys, info))
                {
                    //キャッシュ無効化
                    flags |= PG_PCD | PG_PWT;
                }

                u64 entry = (phys & 0xFFFFFFFFFFE00000ull) | flags;
                reinterpret_cast<u64*>(pd)[i] = entry;
            }
        }

        outCr3Phys = reinterpret_cast<u64>(pml4);
        return true;
    }

    static bool probe2M(u64 cr3, u64 va)
    {
        u64* pml4 = reinterpret_cast<u64*>(cr3);
        u64 l4e = pml4[(va >> 39) & 0x1FF];
        if(!(l4e & 1))
        {
            return false;
        }
        u64* pdpt = reinterpret_cast<u64*>(l4e & ~0xFFFULL);
        u64 l3e = pdpt[(va >> 30) & 0x1FF];
        if(!(l3e & 1))
        {
            return false;
        }
        u64* pd = reinterpret_cast<u64*>(l3e & ~0xFFFULL);
        u64 pde = pd[(va >> 21) & 0x1FF];
        if(!(pde & 1) || !(pde & (1 << 7)))
        {
            return false;
        }

        return true;
    }

    void checkVAMapped(u64 cr3)
    {
        u64 rip = reinterpret_cast<u64>(&&here);
        u64 rsp;
        asm volatile ("mov %%rsp, %0" : "=r"(rsp));
        here:
        struct {const char* name; u64 va;} t[] = {{"RIP", rip}, {"RSP", rsp}, {"IDT", reinterpret_cast<u64>(&Boot::gIDT)}, {"GDT", reinterpret_cast<u64>(&Boot::gGDT)}, {"TSS", reinterpret_cast<u64>(&Boot::gTSS)}};
        for(size_t i=0; i<5; i++)
        {
            Kernel::DebugConsole::printStringRawDbg(t[i].name);
            Kernel::DebugConsole::printStringRawDbg(":");
            Kernel::DebugConsole::printStringRawDbg(probe2M(cr3, t[i].va) == true ? "OK\n" : "Not mapped\n");
        }

        for(size_t i=0; i<10000000000; i++);
    }
}