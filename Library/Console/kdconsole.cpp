#include "basic_font.hpp"
#include "vgatype.hpp"
#include "cstring.hpp"
#include "kd.hpp"
#include "common.hpp"

#define MAKE_RGB(r, g, b) ((HailOS::Graphic::RGB){r, g, b})
#define CALC_PIXEL_OFFSET(x, y) ((y * sPPSL + x) * 4)

namespace HailOS::Kernel::DebugConsole
{
    static Graphic::Rectangle sScreenSize;
    static Graphic::Point sCursorPos;
    static u64 sFrameBufferAddress;
    static u64 sPPSL;
    static Graphic::PixelFormat sPixelFormat;
    static bool sInitialized = false;

    Graphic::FrameBufferColor convertColor(Graphic::RGB color)
    {
        if (!sInitialized)
        {
            PANIC(Status::STATUS_NOT_INITIALIZED, 0, 0, 0, 0);
        }

        Graphic::FrameBufferColor result = {0, 0, 0, 255};

        switch (sPixelFormat)
        {
        case Graphic::PixelFormat::RGB:
            result.Color1 = color.Red;
            result.Color2 = color.Green;
            result.Color3 = color.Blue;
            break;
        case Graphic::PixelFormat::BGR:
            result.Color1 = color.Blue;
            result.Color2 = color.Green;
            result.Color3 = color.Red;
            break;
        default:
            // 論理的に到達しえないコード (RGB/BGR以外なら初期化が失敗するはず)
            PANIC(Status::STATUS_UNREACHABLE, 0, 0, 0, 0);
        }

        return result;
    }

    Graphic::RGB convertColor(Graphic::FrameBufferColor color)
    {
        if (!sInitialized)
        {
            PANIC(Status::STATUS_NOT_INITIALIZED, 0, 0, 0, 0);
        }

        switch (sPixelFormat)
        {
        case Graphic::PixelFormat::RGB:
            return (Graphic::RGB){.Red = color.Color1, .Green = color.Color2, .Blue = color.Color3};
        case Graphic::PixelFormat::BGR:
            return (Graphic::RGB){.Red = color.Color3, .Green = color.Color2, .Blue = color.Color1};
        default:
            // 論理的に到達不能
            PANIC(Status::STATUS_UNREACHABLE, 0, 0, 0, 0);
        }
    }

    void drawPixelRaw(Graphic::Point location, Graphic::RGB color)
    {
        if(location.X >= sScreenSize.Width || location.Y >= sScreenSize.Height || !sInitialized)
        {
            return;
        }

        Graphic::FrameBufferColor dest_color = convertColor(color);
        addr_t offset = CALC_PIXEL_OFFSET(location.X, location.Y);
        Graphic::FrameBufferColor *p = reinterpret_cast<Graphic::FrameBufferColor*>(reinterpret_cast<addr_t>(sFrameBufferAddress) + offset);
        *p = dest_color;
    }

    void initDebugConsole(Graphic::GraphicInfo* info)
    {
        sFrameBufferAddress = info->FrameBufferBase;
        sPPSL = info->PixelsPerScanLine;
        sPixelFormat = info->DisplayPixelFormat;
        sScreenSize = RECT(info->HorizontalResolution, info->VerticalResolution);
        sInitialized = true;
        sCursorPos = COORD(0, 0);

        for (u32 y = 0; y < sScreenSize.Height; y++)
        {
            for (u32 x = 0; x < sScreenSize.Width; x++)
            {
                drawPixelRaw(COORD(x, y), MAKE_RGB(0, 0, 0));
            }
        }
    }

    static inline bool isVisibleChar(char ch)
    {
        if(ch > 0x19 && ch < 0x7f)
        {
            return true;
        }
        return false;
    }

    void printCharRawDbg(char ch)
    {
        if(isVisibleChar(ch))
        {
            const u8* font = HailOS::Console::gConsoleFont[static_cast<int>(ch)];
            if(ch == ' ')
            {
                sCursorPos.X += HailOS::Console::FONT_WIDTH;
                return;
            }
            for(int i=0; i<HailOS::Console::FONT_HEIGHT; i++)
            {
                const u8 line = font[i];
                for(int k=0; k<HailOS::Console::FONT_WIDTH; k++)
                {
                    if(line & (1 << (7-k)))
                    {
                        drawPixelRaw(COORD(sCursorPos.X + k, sCursorPos.Y + i), MAKE_RGB(255, 255, 255));
                    }
                }
            }
            sCursorPos.X += HailOS::Console::FONT_WIDTH;
        }
        else
        {
            switch(ch)
            {
                case '\n':
                    sCursorPos.Y += HailOS::Console::FONT_HEIGHT;
                    sCursorPos.X = 0;
                    return;
                case '\r':
                    sCursorPos.X = 0;
                    return;
                case '\b':
                    return;
                case '\t':
                    if(!(sCursorPos.X + 4*HailOS::Console::FONT_WIDTH >= sScreenSize.Width))
                    {
                        sCursorPos.X += 4*HailOS::Console::FONT_WIDTH;
                    }
                    return;
                default:
                    return;
            }
        }
    }

    void printStringRawDbg(const char* str)
    {
        for(size_t i=0; i<StdLib::C::strlen(str); i++)
        {
            printCharRawDbg(str[i]);
        }

        for(size_t i=0; i<1000000; i++);
    }
}