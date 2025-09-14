#pragma once

#include "memdef.hpp"

namespace HailOS::MemoryManager
{
    bool initMemoryManager(MemoryInfo* info);
    void showStat();
}
