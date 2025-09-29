#include "io.hpp"

namespace HailOS::Kernel::Utility
{
    static bool sForceDisableInterruptState = false;

    void enableInterrupts(void)
    {
        if(!sForceDisableInterruptState)
        {
            asm volatile("sti");
        }
    }

    void disableInterrupts(void)
    {
        asm volatile("cli");
    }

    void setForceDisableInterrupt(bool state)
    {
        sForceDisableInterruptState = state;
    }

    [[noreturn]] void haltProcessor(void)
    {
        while (true)
        {
            asm volatile("cli");
            asm volatile("hlt");
        }
    }

    [[noreturn]] void reset(void)
    {
        while(IO::inb(0x64) & 0x2);
        IO::outb(0x64, 0xFE);
        haltProcessor();
    }
}