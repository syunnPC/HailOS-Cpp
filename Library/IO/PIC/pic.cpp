#include "basetype.hpp"
#include "io.hpp"

namespace HailOS::IO::PIC
{
    static constexpr auto PIC_EOI = 0x20;
    static constexpr auto PIC_MASTER_CMD = 0x20;
    static constexpr auto PIC_MASTER_DATA = 0x21;
    static constexpr auto PIC_SLAVE_CMD = 0xA0;
    static constexpr auto PIC_SLAVE_DATA = 0xA1;
    static constexpr auto ICW1_INIT = 0x10;
    static constexpr auto ICW1_ICW4 = 0x01;
    static constexpr auto ICW4_8086 = 0x01;
    static constexpr auto PIC_MASTER_ISR_OFFSET = 0x20;
    static constexpr auto PIC_SLAVE_ISR_OFFSET = 0x28;

    static inline void wait(void)
    {
        asm volatile("outb %%al, $0x80" : : "a"(0));
    }

    void remap(u32 offsetMaster, u32 offsetSlave)
    {
        u8 a1, a2;
        a1 = inb(PIC_MASTER_DATA);
        a2 = inb(PIC_SLAVE_DATA);
        outb(PIC_MASTER_CMD, ICW1_INIT | ICW1_ICW4);
        wait();
        outb(PIC_SLAVE_CMD, ICW1_INIT | ICW1_ICW4);
        wait();

        outb(PIC_MASTER_DATA, offsetMaster);
        wait();
        outb(PIC_SLAVE_DATA, offsetSlave);
        wait();

        outb(PIC_MASTER_DATA, 0x04);
        wait();
        outb(PIC_SLAVE_DATA, 0x02);
        wait();

        outb(PIC_MASTER_DATA, ICW4_8086);
        wait();
        outb(PIC_SLAVE_DATA, ICW4_8086);
        wait();

        outb(PIC_MASTER_DATA, a1);
        outb(PIC_SLAVE_DATA, a2);
    }

    void unmask(u8 irq)
    {
        u16 port;
        u8 value;

        if(irq < 8)
        {
            port = PIC_MASTER_DATA;
        }
        else
        {
            port = PIC_SLAVE_DATA;
            irq -= 8;
        }

        value = inb(port) & ~(1 << irq);
        outb(port, value);
    }

    void mask(u8 irq)
    {
        u16 port;
        u8 value;
        if(irq < 8)
        {
            port = PIC_MASTER_DATA;
        }
        else
        {
            port = PIC_SLAVE_DATA;
            irq -= 8;
        }

        value = inb(port) | (1 << irq);
        outb(port, value);
    }

    void maskAll(void)
    {
        outb(PIC_MASTER_DATA, 0xFF);
        wait();
        outb(PIC_SLAVE_DATA, 0xFF);
        wait();
    }

    void sendEOI(u8 irq)
    {
        if(irq >= 8)
        {
            outb(PIC_SLAVE_CMD, PIC_EOI);
        }
        outb(PIC_MASTER_CMD, PIC_EOI);
    }
}
