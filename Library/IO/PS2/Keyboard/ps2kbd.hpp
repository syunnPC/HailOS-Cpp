#pragma once

#include "basetype.hpp"

namespace HailOS::Driver::PS2::Keyboard
{
    constexpr auto KEY_BUFFER_SIZE = 256;
    constexpr auto IRQ_KEYBOARD = 1;
    extern u8 gKeyBuffer[KEY_BUFFER_SIZE];
    extern size_t gOffsetRead, gOffsetWrite;
    extern "C" void keyboardHandler(void);
}