#pragma once

namespace HailOS::Kernel::DebugConsole
{
    void printCharRawDbg(char ch);
    void printStringRawDbg(const char* str);
    void initDebugConsole(Graphic::GraphicInfo* info);
}