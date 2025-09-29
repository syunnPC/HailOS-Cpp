#pragma once

#include "basetype.hpp"

namespace HailOS::Kernel
{
    extern "C"
    {
        void loadCR3(u64 pml4Phys);
        u64 readCR3();
        void flushTLBAll();
    }
}