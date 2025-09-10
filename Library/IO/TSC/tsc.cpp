#include "basetype.hpp"

namespace HailOS::IO::TSC
{
    u64 read(void)
    {
        u32 lo, hi;
        asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
        return (static_cast<u64>(hi) << 32) | lo;
    }
}