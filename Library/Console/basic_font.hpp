#pragma once

namespace HailOS::Console
{
    constexpr auto FONT_HEIGHT = 16;
    constexpr auto FONT_WIDTH = 8;

    extern "C" const unsigned char gConsoleFont[127][16];
}