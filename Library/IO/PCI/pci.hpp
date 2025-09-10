#pragma once

#include "basetype.hpp"

namespace HailOS::IO::PCI
{
    enum class VendorID : u16
    {
        INVALID = 0xFFFF,
        AMD = 0x1022,
        INTEL = 0x8086,
        NVIDIA = 0x10DE,
        REALTEK = 0x10EC,
        MICROSOFT = 0x1414,
        MARVELL = 0x1DCA,
        MSI = 0x1462,
        MICRON = 0x1344,
        SEAGATE = 0x1BB1,
        ASUS = 0x1043,
        DELL = 0x1028,
        VMWARE = 0x15AD,
        ORACLE = 0x108E,
        INNOTEK = 0x80EE,
    };

    u32 readConfig32(u8 bus, u8 device, u8 function, u8 offset);
}