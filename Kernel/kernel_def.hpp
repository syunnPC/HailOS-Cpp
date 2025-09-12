#pragma once

#include "basetype.hpp"
#include "vgatype.hpp"
#include "memmgr.hpp"
#include "timedef.hpp"
#include "acpi.hpp"

namespace HailOS::Kernel
{
    struct BootInfo
    {
        char* Args;
        MemoryManager::MemoryInfo* MemInfo;
        HailOS::Utility::Timer::HardwareClockInfo* ClockInfo;
        Graphic::GraphicInfo* FrameBufferInfo;
        PowerManager::ACPI::RSDPtr* RSDP;
    } PACKED;
}