#include "basetype.hpp"
#include "io.hpp"

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
}