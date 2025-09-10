#pragma once

namespace HailOS::Driver::PS2::Keyboard
{
    constexpr auto LEFT_SHIFT_DOWN = 0x2A;
    constexpr auto RIGHT_SHIFT_DOWN = 0x36;
    constexpr auto LEFT_SHIFT_UP = 0xAA;
    constexpr auto RIGHT_SHIFT_UP = 0xB6;
    constexpr auto ENTER_KEY = 0x1C;

    extern "C" const char gScancodeAsciiTable[0x7F];
    extern "C" const char gScancodeAsciiTableShift[0x7F];
}