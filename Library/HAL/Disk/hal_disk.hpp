#pragma once

#include "basetype.hpp"
#include "ahci.hpp"

namespace HailOS::HAL::Disk
{
    enum class DiskType
    {
        None = 0,
        IDE = 1,
        AHCI = 2,
    };

    struct DiskInfo
    {
        DiskType Type;
        volatile Driver::AHCI::HBAMemory* Abar;
        u32 PortIndex; 
    };

    bool readSector(u64 lba, u8* buf);
    bool initDisk(void);
}