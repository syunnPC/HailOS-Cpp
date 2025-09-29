#include "basetype.hpp"

namespace HailOS::Kernel
{
    extern "C"
    {
        void loadCR3(u64 pml4Phys)
        {
            asm volatile("mov %0, %%cr3" : : "r"(pml4Phys) : "memory");
        }

        u64 readCR3()
        {
            u64 v;
            asm volatile("mov %%cr3, %0" : "=r"(v) :: "memory");
            return v;
        }

        void flushTLBAll()
        {
            asm volatile(
                "mov %%cr3, %%rax\n\t"
                "mov %%rax, %%cr3\n\t"
                : : :"rax", "memory"
            );
        }
    }
}