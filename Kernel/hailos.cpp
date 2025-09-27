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
#include "cursor.hpp"
#include "time.hpp"
#include "bitmap.hpp"
#include "cstring.hpp"
#include "memutil.hpp"
#include "hal_kbd.hpp"
#include "pci.hpp"
#include "xhci.hpp"
#include "cr3ctrl.hpp"
#include "paging.hpp"
#include "kdconsole.hpp"
#include "kd.hpp"

namespace HailOS::Kernel::Boot
{
    BootInfo* bootInfoPtr = nullptr;
}

extern "C" void main(HailOS::Kernel::BootInfo *info)
{
    using namespace HailOS;

    HailOS::Kernel::Boot::bootInfoPtr = info;

    IO::PIC::maskAll();
    
    Kernel::Utility::setForceDisableInterrupt(true);
    Kernel::Utility::disableInterrupts();

    Kernel::DebugConsole::initDebugConsole(info->FrameBufferInfo);
    Kernel::DebugConsole::printStringRawDbg("Successfully initialized debug console.\n");

    MemoryManager::ReserveRange(*info->MemInfo, reinterpret_cast<u64>(Kernel::Boot::_kernel_start), reinterpret_cast<u64>(Kernel::Boot::_kernel_end) - reinterpret_cast<u64>(Kernel::Boot::_kernel_start));
    MemoryManager::initPageTableAllocator(*info->MemInfo);

    Kernel::Boot::initGDTandTSS(HailOS::Kernel::Boot::kernel_stack_top);
    Kernel::Boot::initIDT();
    u64 cr3 = 0;

    //必ずkeepNullUnmapped=falseで呼ぶ、NULL周辺2MiBエリアにFree領域がある場合がある.
    if(Kernel::buildIdentitiyMap2M(info->MemInfo, cr3, info->FrameBufferInfo->FrameBufferBase, false))
    {
        Kernel::checkVAMapped(cr3);
        Kernel::loadCR3(cr3);
        Kernel::flushTLBAll();
    }
    if(!MemoryManager::initMemoryManager(info->MemInfo))
    {
        PANIC(Status::STATUS_NOT_INITIALIZED, 0, 0, 0, 0);
    }
    if (!Graphic::initGraphics(info->FrameBufferInfo, Console::DEFAULT_CONSOLE_BACKGROUND_COLOR))
    {
        PANIC(Status::STATUS_NOT_INITIALIZED, 0, 0, 0, 0);
    }
    if (!Console::initConsole())
    {
        PANIC(Status::STATUS_NOT_INITIALIZED, 1, static_cast<u64>(getLastStatus()), 0, 0);
    }
    if (!Utility::Time::initTime(info->ClockInfo))
    {
        PANIC(Status::STATUS_NOT_INITIALIZED, 2, static_cast<u64>(getLastStatus()), 0, 0);
    }
    if (!HAL::Disk::initDisk())
    {
        PANIC(Status::STATUS_NOT_INITIALIZED, 3, static_cast<u64>(getLastStatus()), 0, 0);
    }
    if (!Driver::Filesystem::FAT32::initFAT32())
    {
        PANIC(Status::STATUS_NOT_INITIALIZED, 4, static_cast<u64>(getLastStatus()), 0, 0);
    }
    if (!PowerManager::ACPI::initACPI(info->RSDP))
    {
        Console::puts("Failed to initialize ACPI.\n");
    }

    if (!Driver::PS2::Mouse::initMouse())
    {
        Console::puts("Failed to initialize PS/2 Mouse.\n");
    }
    else
    {
        if (!UI::Cursor::initCursor())
        {
            Console::puts("Failed to initialize cursor.\nlastStatus: ");
            Console::puts(statusToString(getLastStatus()));
            Console::puts("\n");
        }
    }

    if(cr3 == 0)
    {
        Console::puts("Failed to build page table.\n");
    }

    Kernel::Utility::setForceDisableInterrupt(false);

    IO::PIC::remap(0x20, 0x28);
    IO::PIC::unmask(2); //PIC スレーブ
    IO::PIC::unmask(Driver::PS2::Mouse::IRQ_MOUSE);
    IO::PIC::unmask(Driver::PS2::Keyboard::IRQ_KEYBOARD);

    Kernel::Utility::enableInterrupts();

    Graphic::Rectangle rect;

    Console::puts("HailOS-C++ version 0.4 Beta\nSee https://github.com/syunnPC/HailOS-Cpp\nFramebuffer address: ");
    Console::puts(StdLib::C::utohexstr(Graphic::getBufferAddress()));
    Console::puts(", resolution ");
    Console::puts(StdLib::C::utos(Graphic::getScreenResolution().Width));
    Console::puts(" x ");
    Console::puts(StdLib::C::utos(Graphic::getScreenResolution().Height));
    Console::puts(", PPSL: ");
    Console::puts(StdLib::C::utos(Console::getPixelsPerScanLine()));
    Console::puts("\n");
    Console::puts("Memory size = ");
    Console::puts(StdLib::C::utos(MemoryManager::queryAvailableMemorySize()));
    Console::puts(" bytes, largest memory block = ");
    Console::puts(StdLib::C::utos(MemoryManager::queryLargestMemoryRegion()));
    Console::puts(" bytes\n");
    Graphic::BitmapImage::drawBitmap("picture1.bmp", Console::getCursorPos(), &rect);
    Console::setCursorPos({0, Console::getCursorPos().Y + rect.Height});
    IO::USB::xHCI::initxHCI();
    IO::USB::xHCI::xhciControllerSelfTest();
    IO::USB::xHCI::dumpPortStatus();
    Console::puts("lastStatus : ");
    Console::puts(statusToString(getLastStatus()));
    Console::puts("\n");

    IO::USB::xHCI::TRB ev{};

    while(true)
    {
        if(Driver::PS2::Mouse::gMouseMoved)
        {
            UI::Cursor::updateCursor(COORD(Driver::PS2::Mouse::gMouseState.X, Driver::PS2::Mouse::gMouseState.Y));
            Driver::PS2::Mouse::gMouseMoved = false;
        }
        if(IO::USB::xHCI::pollEvent(&ev))
        {
            //ログ
        }
    }
}