#include "basetype.hpp"
#include "status.hpp"
#include "common.hpp"
#include "init.hpp"
#include "vga.hpp"
#include "console.hpp"
#include "kernellib.hpp"
#include "cstring.hpp"
#include "pic.hpp"
#include "kdconsole.hpp"
#include "kernel_def.hpp"

namespace HailOS::Kernel
{
    //i8042キーボードコントローラーからリセットを試し、ダメならトリプルフォールトを起こす
    void forceReboot(void)
    {
        for(size_t i=0; i<100000000; i++);
        Utility::reset();

        Boot::IDTR idtr = {0, 0};
        asm volatile(
            "lidt %[idtr]\n\t"
            "int3\r\n"
            "hlt\r\n"
            "jmp .\n\t"
            : :[idtr] "m"(idtr) : "memory"
        );
    }

    static bool sInPanic = false;
    
    namespace Boot
    {
        extern BootInfo* bootInfoPtr;
    }

    void panic(Status status, u64 param1, u64 param2, u64 param3, u64 param4, const char *file, int line)
    {
        if(!Graphic::isGraphicAvailable() || sInPanic)
        {
            forceReboot();
        }

        IO::PIC::maskAll();
        Utility::disableInterrupts();
        Utility::setForceDisableInterrupt(true);

        sInPanic = true;

        /*
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
        */

        Kernel::DebugConsole::initDebugConsole(Boot::bootInfoPtr->FrameBufferInfo);
        DebugConsole::printStringRawDbg("System Error! Status: ");
        DebugConsole::printStringRawDbg(StdLib::C::utohexstr(static_cast<u64>(status)));
        DebugConsole::printStringRawDbg(" (");
        DebugConsole::printStringRawDbg(statusToString(status));
        DebugConsole::printStringRawDbg(")\n");
        DebugConsole::printStringRawDbg("Param 1:");
        DebugConsole::printStringRawDbg(StdLib::C::utohexstr(param1));
        DebugConsole::printStringRawDbg("\n");
        DebugConsole::printStringRawDbg("Param 2:");
        DebugConsole::printStringRawDbg(StdLib::C::utohexstr(param2));
        DebugConsole::printStringRawDbg("\n");
        DebugConsole::printStringRawDbg("Param 3:");
        DebugConsole::printStringRawDbg(StdLib::C::utohexstr(param3));
        DebugConsole::printStringRawDbg("\n");
        DebugConsole::printStringRawDbg("Param 4:");
        DebugConsole::printStringRawDbg(StdLib::C::utohexstr(param4));
        DebugConsole::printStringRawDbg("\nFile: ");
        DebugConsole::printStringRawDbg(file);
        DebugConsole::printStringRawDbg("\nline: ");
        DebugConsole::printStringRawDbg(StdLib::C::utos(line));

        for(size_t i=0; i<1000000000000; i++);

        Utility::reset();
    }
}