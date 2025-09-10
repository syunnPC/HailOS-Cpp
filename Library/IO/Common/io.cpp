#include <basetype.hpp>

namespace HailOS::IO
{
    void outb(u16 port, u8 value)
    {
        asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
    }

    void outw(u16 port, u16 value)
    {
        asm volatile("outw %0, %1" : : "a"(value), "Nd"(port));
    }

    void outl(u16 port, u32 value)
    {
        asm volatile("outl %0, %1" : : "a"(value), "Nd"(port));
    }

    u8 inb(u16 port)
    {
        u8 result;
        asm volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
        return result;
    }

    u16 inw(u16 port)
    {
        u16 result;
        asm volatile("inw %1, %0" : "=a"(result) : "Nd"(port));
        return result;
    }

    u32 inl(u16 port)
    {
        u32 result;
        asm volatile("inl %1, %0" : "=a"(result) : "Nd"(port));
        return result;
    }
}