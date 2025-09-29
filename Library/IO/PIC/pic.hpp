#pragma once

#include "basetype.hpp"

namespace HailOS::IO::PIC
{
    void remap(u32 offsetMaster, u32 offsetSlave);
    void unmask(u8 irq);
    void sendEOI(u8 irq);
    void mask(u8 irq);
    void maskAll(void);
}