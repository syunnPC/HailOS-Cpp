#pragma once

#include "basetype.hpp"
#include "vgatype.hpp"

namespace HailOS::Console
{
    using namespace HailOS::Graphic;

    constexpr RGB DEFAULT_CONSOLE_BACKGROUND_COLOR = COLOR_GRAY;
    constexpr RGB DEFAULT_CHAR_COLOR = COLOR_WHITE;

    void printChar(char ch, RGB color);
    void printString(const char* str, RGB color);
    void deleteChar(void);
    void scroll(u32 line);
    Point setCursorPos(Point location);
    Point getCursorPos(void);
    char readKeyWithEcho(RGB color);
    size_t ReadInputWithEcho(char* buf, size_t bufSize, RGB color, bool newLine, bool exitBufferFull);
    void puts(const char* str);
    bool initConsole(void);
}