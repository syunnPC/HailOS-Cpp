#include "ps2kbd.hpp"
#include "keycode.hpp"
#include "basetype.hpp"
#include "memutil.hpp"

namespace HailOS::HAL::Keyboard
{
    static bool sShiftStatus = false;

    using namespace HailOS::Driver::PS2::Keyboard;

    bool readKeyFromBuffer(u8& out)
    {
        if(gOffsetWrite == gOffsetRead)
        {
            return false;
        }

        if(gOffsetRead == KEY_BUFFER_SIZE)
        {
            gOffsetRead = 0;
        }

        out = gKeyBuffer[gOffsetRead++];
        return true;
    }

    char scancodeToAscii(u8 scancode)
    {
        if(scancode & 0x80)
        {
            char c = scancode & 0x7F;
            if(c == LEFT_SHIFT_DOWN || c == RIGHT_SHIFT_DOWN)
            {
                sShiftStatus = true;
            }
            return 0;
        }
        if(scancode == LEFT_SHIFT_DOWN || scancode == RIGHT_SHIFT_DOWN)
        {
            sShiftStatus = true;
            return 0;
        }
        if(sShiftStatus)
        {
            return gScancodeAsciiTableShift[scancode];
        }
        else
        {
            return gScancodeAsciiTable[scancode];
        }
    }

    char readKey(void)
    {
        u8 sc;
        while(true)
        {
            if(readKeyFromBuffer(sc))
            {
                sc = scancodeToAscii(sc);
                if(sc != 0)
                {
                    return sc;
                }
            }
        }
    }

    size_t readInput(char* buffer, size_t bufferSize)
    {
        if(buffer == nullptr || bufferSize == 0)
        {
            return 0;
        }

        size_t read_size = 0;
        u8 sc;
        MemoryManager::fill(buffer, bufferSize, 0);
        while(true)
        {
            if(readKeyFromBuffer(sc))
            {
                if(sc == ENTER_KEY)
                {
                    return read_size;
                }
                if(read_size == bufferSize - 1)
                {
                    return read_size;
                }
                buffer[read_size++] = scancodeToAscii(sc);
            }
        }
    }
}