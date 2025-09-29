#pragma once

#include "vga.hpp"

namespace HailOS::Kernel::DebugConsole
{
    void printCharRawDbg(char ch);
    void printStringRawDbg(const char* str);
    void initDebugConsole(Graphic::GraphicInfo* info, Graphic::RGB charColor = Graphic::COLOR_WHITE, Graphic::RGB bgColor = Graphic::COLOR_BLACK, Graphic::RGB* charBgColor = nullptr);
    void setBackgroundColor(Graphic::RGB color);
    void setCharColor(Graphic::RGB color);
    void setCharBgColor(Graphic::RGB* color);
}