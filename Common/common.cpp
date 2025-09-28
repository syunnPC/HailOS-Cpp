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

        Kernel::DebugConsole::initDebugConsole(Boot::bootInfoPtr->FrameBufferInfo, Graphic::COLOR_WHITE, Graphic::COLOR_BLUE);
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

        for(volatile size_t i=0; i<1000000000; i++);

        Utility::reset();
    }
}