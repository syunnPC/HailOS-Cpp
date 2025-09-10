#include "basetype.hpp"
#include "io.hpp"
#include "pic.hpp"
#include "ps2.hpp"
#include "ps2kbd.hpp"

namespace HailOS::Driver::PS2::Keyboard
{
    size_t gOffsetRead, gOffsetWrite;
    u8 gKeyBuffer[KEY_BUFFER_SIZE];

    static constexpr auto KEYCODE_MASK_RELEASE = 0x80;

    extern "C" void keyboardHandler(void)
    {
        u8 sc = IO::inb(PS2_DATA_PORT);
        if(gOffsetWrite == KEY_BUFFER_SIZE)
        {
            gOffsetWrite = 0;
        }

        gKeyBuffer[gOffsetWrite++] = sc;
        
        IO::PIC::sendEOI(IRQ_KEYBOARD);
    }
}