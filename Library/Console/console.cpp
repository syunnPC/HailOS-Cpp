#include "basetype.hpp"
#include "vga.hpp"
#include "vgatype.hpp"
#include "basic_font.hpp"
#include "hal_kbd.hpp"
#include "keycode.hpp"
#include "cstring.hpp"
#include "memutil.hpp"
#include "console.hpp"
#include "cursor.hpp"

namespace HailOS::Console
{
    using namespace HailOS::Graphic;

    static Point sCursorPos;
    static Rectangle sScreenSize;
    static bool sInitialized = false;

    bool initConsole(void)
    {
        if(!isGraphicAvailable())
        {
            return false;
        }

        if(!sInitialized)
        {
            sScreenSize = getScreenResolution();
            sInitialized = true;
        }
        return true;
    }

    static inline bool isNewLineRequired(void)
    {
        if(!isGraphicAvailable() || !sInitialized)
        {
            return false;
        }

        if(sCursorPos.X > sScreenSize.Width - FONT_WIDTH)
        {
            return true;
        }

        return false;
    }

    static inline bool isScrollRequired(void)
    {
        if(!isGraphicAvailable() || !sInitialized)
        {
            return false;
        }

        if(sCursorPos.Y > sScreenSize.Height - FONT_HEIGHT)
        {
            return true;
        }

        return false;
    }

    static inline bool isVisibleChar(char ch)
    {
        if(ch > 0x19 && ch < 0x7f)
        {
            return true;
        }
        return false;
    }

    static void deleteCharOnBuffer(void)
    {
        if (!isGraphicAvailable() || !sInitialized)
        {
            return;
        }

        if (sCursorPos.X > FONT_WIDTH)
        {
            sCursorPos.X -= FONT_WIDTH;
        }
        else
        {
            if (sCursorPos.Y >= FONT_HEIGHT)
            {
                sCursorPos.X = ((sScreenSize.Width / FONT_WIDTH) * FONT_WIDTH) - FONT_WIDTH;
                sCursorPos.Y -= FONT_HEIGHT;
            }
            else
            {
                return;
            }
        }

        for (u32 y = 0; y < FONT_HEIGHT; y++)
        {
            for (u32 x = 0; x < FONT_WIDTH; x++)
            {
                setEmptyPixelOnBuffer(COORD(sCursorPos.X + x, sCursorPos.Y + y));
            }
        }

        //UI::Cursor::updateCursor(UI::Cursor::getCursorPosition());
    }

    static void printCharToBuffer(char ch, RGB color)
    {
        if(!isGraphicAvailable() || !sInitialized)
        {
            return;
        }

        if(isVisibleChar(ch))
        {
            const u8* font = gConsoleFont[static_cast<int>(ch)];
            if(isNewLineRequired())
            {
                sCursorPos.X = 0;
                sCursorPos.Y += FONT_HEIGHT;
            }
            if(isScrollRequired())
            {
                sCursorPos.X = 0;
                while(isScrollRequired())
                {
                    sCursorPos.Y -= FONT_HEIGHT;
                    scroll(1);
                }
            }
            if(ch == ' ')
            {
                sCursorPos.X += FONT_WIDTH;
                return;
            }
            for(int i=0; i<FONT_HEIGHT; i++)
            {
                const u8 line = font[i];
                for(int k=0; k<FONT_WIDTH; k++)
                {
                    if(line & (1 << (7-k)))
                    {
                        drawPixelToBuffer(COORD(sCursorPos.X + k, sCursorPos.Y + i), color);
                    }
                    else
                    {
                        setEmptyPixelOnBuffer(COORD(sCursorPos.X + k, sCursorPos.Y + i));
                    }
                }
            }
            sCursorPos.X += FONT_WIDTH;
            //UI::Cursor::updateCursor(UI::Cursor::getCursorPosition());
        }
        else
        {
            switch(ch)
            {
                case '\n':
                    sCursorPos.Y += FONT_HEIGHT;
                    sCursorPos.X = 0;
                    while(isScrollRequired())
                    {
                        scroll(1);
                        sCursorPos.Y -= FONT_HEIGHT;
                    }
                    return;
                case '\r':
                    sCursorPos.X = 0;
                    return;
                case '\b':
                    deleteCharOnBuffer();
                    return;
                case '\t':
                    if(!(sCursorPos.X + 4*FONT_WIDTH >= getScreenResolution().Width))
                    {
                        sCursorPos.X += 4*FONT_WIDTH;
                    }
                    return;
                default:
                    return;
            }
        }
    }

    void printChar(char ch, RGB color)
    {
        if (!isGraphicAvailable() || !sInitialized)
        {
            return;
        }

        printCharToBuffer(ch, color);
        drawBufferContentsToFrameBuffer();
        //UI::Cursor::updateCursor(UI::Cursor::getCursorPosition());
    }

    void printString(const char* str, RGB color)
    {
        if (!isGraphicAvailable() || !sInitialized)
        {
            return;
        }

        for(size_t i=0; i<StdLib::C::strlen(str); i++)
        {
            printCharToBuffer(str[i], color);
        }
        drawBufferContentsToFrameBuffer();
        //UI::Cursor::updateCursor(UI::Cursor::getCursorPosition());
    }

    void scroll(u32 line)
    {
        if(!isGraphicAvailable() || !sInitialized || line == 0)
        {
            return;
        }

        if(line >= sScreenSize.Height/FONT_HEIGHT)
        {
            clearBuffer();
            fillScreenWithBackgroundColor();
            sCursorPos = COORD(0, 0);
            return;
        }

        shiftBufferContents(FONT_HEIGHT*line, Direction::VERTICAL_UP);
        fillScreenWithBackgroundColor();
        drawBufferContentsToFrameBuffer();
        //UI::Cursor::updateCursor(UI::Cursor::getCursorPosition());
    }

    Point setCursorPos(Point location)
    {
        Point prev = sCursorPos;
        sCursorPos = location;
        return prev;
    }

    Point getCursorPos()
    {
        return sCursorPos;
    }

    void puts(const char* str)
    {
        printString(str, DEFAULT_CHAR_COLOR);
    }

    void deleteChar(void)
    {
        if(!isGraphicAvailable() || !sInitialized)
        {
            return;
        }

        deleteCharOnBuffer();
        fillScreenWithBackgroundColor();
        drawBufferContentsToFrameBuffer();
        //UI::Cursor::updateCursor(UI::Cursor::getCursorPosition());
    }

    char readKeywithEcho(RGB color)
    {
        if(!isGraphicAvailable() || !sInitialized)
        {
            return false;
        }

        u8 sc;
        while(true)
        {
            if(HAL::Keyboard::readKeyFromBuffer(sc))
            {
                sc = HAL::Keyboard::scancodeToAscii(sc);
                if(sc != 0)
                {
                    printChar(sc, color);
                    return sc;
                }
            }
        }
    }

    char readInputWithEcho(char* buffer, size_t bufferSize, RGB color, bool newLine, bool exitBufferFull)
    {
        if(!isGraphicAvailable() || !sInitialized)
        {
            return 0;
        }

        if(buffer == nullptr || bufferSize == 0)
        {
            return 0;
        }

        size_t read_size = 0;
        u8 sc;
        char ch;
        MemoryManager::fill(buffer, bufferSize, 0);
        while(true)
        {
            if(HAL::Keyboard::readKeyFromBuffer(sc))
            {
                if(sc == Driver::PS2::Keyboard::ENTER_KEY)
                {
                    if(newLine)
                    {
                        printString("\n", {0, 0, 0});
                    }
                    buffer[read_size] = '\0';
                    return read_size;
                }
                else if(sc == 0x0E)
                {
                    if(read_size > 0)
                    {
                        read_size--;
                        buffer[read_size] = '\0';
                        deleteChar();
                    }
                }
                else
                {
                    ch = HAL::Keyboard::scancodeToAscii(sc);
                    if(ch != 0)
                    {
                        if(!(read_size < bufferSize - 1))
                        {
                            buffer[read_size++] = ch;
                            printChar(ch, color);
                        }
                        if(exitBufferFull && (read_size == bufferSize - 1))
                        {
                            buffer[read_size] = '\0';
                            return read_size;
                        }
                    }
                }
            }
        }
    }
}