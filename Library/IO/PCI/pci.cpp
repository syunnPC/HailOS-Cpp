#include "basetype.hpp"
#include "io.hpp"
#include "pci.hpp"
#include "status.hpp"

namespace HailOS::IO::PCI
{
    constexpr auto CONFIG_ADDRES = 0xCF8;
    constexpr auto CONFIG_DATA = 0xCFC;
    
    u32 readConfig32(u8 bus, u8 device, u8 function, u8 offset)
    {
        u32 address = (1u << 31) | (static_cast<u32>(bus) << 16) | (static_cast<u32>(device) << 11) | (static_cast<u32>(function) << 8) | (offset & 0xFC);
        outl(CONFIG_ADDRES, address);
        return inl(CONFIG_DATA);
    }

    void writeConfig32(u8 bus, u8 device, u8 function, u8 offset, u32 value)
    {
        u32 address = (1u << 31) | (static_cast<u32>(bus) << 16) | (static_cast<u32>(device) << 11) | (static_cast<u32>(function) << 8) | (offset & 0xFC);
        outl(CONFIG_ADDRES, address);
        outl(CONFIG_DATA, value);
    }

    bool isExistingPCIDevice(u8 bus, u8 device, u8 function)
    {
        u32 id = readConfig32(bus, device, function, 0);
        IO::PCI::VendorID vendor = static_cast<IO::PCI::VendorID>(id & 0xFFFF);
        if (vendor == IO::PCI::VendorID::INVALID)
        {
            return false;
        }

        return true;
    }

    bool isExistingPCIDevice(PCILocation loc)
    {
        return isExistingPCIDevice(loc.Bus, loc.Device, loc.Function);
    }

    bool getBAR0(PCILocation loc, BARInfo& info)
    {
        if(!isExistingPCIDevice(loc))
        {
            setLastStatus(Status::STATUS_DEVICE_NOT_FOUND);
            return false;
        }

        u8 offset = 0x10;
        u32 original = readConfig32(loc.Bus, loc.Device, loc.Device, offset);

        writeConfig32(loc.Bus, loc.Device, loc.Function, offset, 0xFFFFFFFF);
        u32 mask = readConfig32(loc.Bus, loc.Device, loc.Function, offset);

        writeConfig32(loc.Bus, loc.Device, loc.Function, offset, original);

        if(original == 0 || original == 0xFFFFFFFF)
        {
            
            return false;
        }

        if(original & 0x1)
        {
            info.isIO = true;
            info.Base = static_cast<addr_t>(original & ~0x3U);
            info.Size = static_cast<size_t>(~(mask & ~0x3U) + 1);
        }
        else
        {
            info.isIO = false;
            u32 type = (original >> 1) & 0x3;
            if(type == 0x2)
            {
                u32 original_hi = readConfig32(loc.Bus, loc.Device, loc.Function, offset + 4);
                u32 mask_hi;

                writeConfig32(loc.Bus, loc.Device, loc.Function, offset + 4, 0xFFFFFFFF);
                mask_hi = readConfig32(loc.Bus, loc.Device, loc.Function, offset + 4);
                writeConfig32(loc.Bus, loc.Device, loc.Function, offset+ 4, original_hi);

                u64 full_original = ((static_cast<u64>(original_hi) << 32) | (original & ~0xFULL));
                u64 full_mask = ((static_cast<u64>(mask_hi) << 32) | (mask & ~0xFULL));
                
                info.Base = full_original;
                info.Size = ~(full_mask) + 1;
            }
            else
            {
                info.Base = static_cast<u64>(original & ~0xFULL);
                info.Size = static_cast<u64>(~(mask & ~0xFULL) + 1);
            }
        }

        return true;
    }

    void enableBusMasterMMIO(PCILocation loc)
    {
        u32 value = readConfig32(loc.Bus, loc.Device, loc.Function, 0x04);
        u16 command = static_cast<u16>(value & 0xFFFF);

        command |= (1 << 1); //MMIO有効
        command |= (1 << 2); //Bus Master有効

        u32 nv = (value & 0xFFFF0000) | command;
        writeConfig32(loc.Bus, loc.Device, loc.Function, 0x04, nv);
    }
}