#pragma once

#include "basetype.hpp"

namespace HailOS::Driver::ATA
{
    bool readSectorLBA28(u32 lba, u8* buffer);
    bool checkMasterDevice(void);
}