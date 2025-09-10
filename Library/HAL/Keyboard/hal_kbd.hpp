#pragma once

#include "basetype.hpp"

namespace HailOS::HAL::Keyboard
{
    bool readKeyFromBuffer(u8& out);
    char scancodeToAscii(u8 scancode);
    char readKey(void);
    size_t readInput(char* buffer, size_t bufferSize);
}