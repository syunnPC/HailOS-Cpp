#include "basetype.hpp"
#include "status.hpp"
#include "common.hpp"
#include "init.hpp"
#include "vga.hpp"
#include "console.hpp"
#include "kernellib.hpp"
#include "cstring.hpp"

namespace HailOS::Kernel
{
    void forceReboot(void)
    {
        Boot::IDTR idtr = {0, 0};
        asm volatile(
            "lidt %[idtr]\n\t"
            "int3\r\n"
            "hlt\r\n"
            "jmp .\n\t"
            : :[idtr] "m"(idtr) : "memory"
        );

        Utility::haltProcessor();
    }

    void panic(Status status, u64 param1, u64 param2, u64 param3, u64 param4, const char *file, int line)
    {
        if(!Graphic::isGraphicAvailable())
        {
            forceReboot();
        }

        Graphic::setBackgroundColor(Graphic::COLOR_BLUE);
        Graphic::fillScreenWithBackgroundColor();
        Console::setCursorPos(COORD(0, 0));
        Console::clearBuffer();

        Console::puts("System Error! Status: ");
        Console::puts(StdLib::C::utohexstr(static_cast<u64>(status)));
        Console::puts(" (");
        Console::puts(statusToString(status));
        Console::puts(")\n");
        Console::puts("Param 1:");
        Console::puts(StdLib::C::utohexstr(param1));
        Console::puts("\n");
        Console::puts("Param 2:");
        Console::puts(StdLib::C::utohexstr(param2));
        Console::puts("\n");
        Console::puts("Param 3:");
        Console::puts(StdLib::C::utohexstr(param3));
        Console::puts("\n");
        Console::puts("Param 4:");
        Console::puts(StdLib::C::utohexstr(param4));
        Console::puts("\n");
        Console::puts("in file ");
        Console::puts(file);
        Console::puts(", line ");
        Console::puts(StdLib::C::utos(line));

        Utility::haltProcessor();
    }
}