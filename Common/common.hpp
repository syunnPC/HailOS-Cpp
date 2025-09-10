#pragma once

#include "basetype.hpp"
#include "status.hpp"

namespace HailOS::Kernel
{
    void panic(Status status, u64 param1, u64 param2, u64 param3, u64 param4, const char *file, int line);
    void forceReboot(void);
}

#define PANIC(status, p1, p2, p3, p4) HailOS::Kernel::panic(status, p1, p2, p3, p4, __FILE__, __LINE__)