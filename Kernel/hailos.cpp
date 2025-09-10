#include "basetype.hpp"
#include "common.hpp"
#include "status.hpp"

#include "kernel_def.hpp"
#include "kernellib.hpp"

#include "init.hpp"

#include "memmgr.hpp"
#include "vga.hpp"
#include "console.hpp"
#include "timemgr.hpp"
#include "ps2mouse.hpp"
#include "ps2kbd.hpp"
#include "hal_disk.hpp"
#include "fat32.hpp"
#include "pic.hpp"


extern "C" void main(HailOS::Kernel::BootInfo* info)
{
    using namespace HailOS;

    Kernel::Boot::initGDTandTSS(HailOS::Kernel::Boot::kernel_stack_top);
    IO::PIC::remap(0x20, 0x28);
    Kernel::Boot::initIDT();
    IO::PIC::unmask(2);
    IO::PIC::unmask(Driver::PS2::Mouse::IRQ_MOUSE);
    IO::PIC::unmask(Driver::PS2::Keyboard::IRQ_KEYBOARD);

    if(!MemoryManager::initMemoryManager(info->MemInfo))
    {
        Kernel::forceReboot();
    }
    if(!Graphic::initGraphics(info->FrameBufferInfo, Console::DEFAULT_CONSOLE_BACKGROUND_COLOR))
    {
        Kernel::forceReboot();
    }
    if(!Console::initConsole())
    {
        PANIC(Status::STATUS_NOT_INITIALIZED, 1, 0, 0, 0);
    }
    if(!Utility::Timer::initTime(info->ClockInfo))
    {
        PANIC(Status::STATUS_NOT_INITIALIZED, 2, 0, 0, 0);
    }
    if(!HAL::Disk::initDisk())
    {
        PANIC(Status::STATUS_NOT_INITIALIZED, 3, 0, 0, 0);
    }
    if(!Driver::PS2::Mouse::initMouse())
    {
        PANIC(Status::STATUS_NOT_INITIALIZED, 4, 0, 0, 0);
    }

    Kernel::Utility::enableInterrupts();
}