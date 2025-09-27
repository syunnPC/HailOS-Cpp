#include "vgatype.hpp"
#include "basetype.hpp"
#include "memutil.hpp"
#include "common.hpp"
#include "status.hpp"
#include "cursor.hpp"
#include "kernellib.hpp"
#include "kd.hpp"

#define CALC_PIXEL_OFFSET(x, y) ((y * HailOS::Graphic::sGraphicInfo->PixelsPerScanLine + x) * HailOS::Graphic::PIXEL_SIZE)

namespace HailOS::Graphic
{
    static GraphicInfo* sGraphicInfo = nullptr;
    static u8* sBuffer;
    static RGB sBackgroundColor;
    static bool sInitialized = false;

    static constexpr auto NO_TRANSFER_BYTE = 0x00;
    static constexpr FrameBufferColor NO_TRANSFER_COLOR = {NO_TRANSFER_BYTE, NO_TRANSFER_BYTE, NO_TRANSFER_BYTE, NO_TRANSFER_BYTE};

    bool isGraphicAvailable(void)
    {
        return sInitialized;
    }

    FrameBufferColor convertColor(RGB color)
    {
        if (!sInitialized)
        {
            PANIC(Status::STATUS_NOT_INITIALIZED, 0, 0, 0, 0);
        }

        FrameBufferColor result = {0, 0, 0, 255};

        switch (sGraphicInfo->DisplayPixelFormat)
        {
        case PixelFormat::RGB:
            result.Color1 = color.Red;
            result.Color2 = color.Green;
            result.Color3 = color.Blue;
            break;
        case PixelFormat::BGR:
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

    RGB convertColor(FrameBufferColor color)
    {
        if (!sInitialized)
        {
            PANIC(Status::STATUS_NOT_INITIALIZED, 0, 0, 0, 0);
        }

        switch (sGraphicInfo->DisplayPixelFormat)
        {
        case PixelFormat::RGB:
            return (RGB){.Red = color.Color1, .Green = color.Color2, .Blue = color.Color3};
        case PixelFormat::BGR:
            return (RGB){.Red = color.Color3, .Green = color.Color2, .Blue = color.Color1};
        default:
            // 論理的に到達不能
            PANIC(Status::STATUS_UNREACHABLE, 0, 0, 0, 0);
        }
    }

    bool drawPixel(Point location, RGB color)
    {
        if (!sInitialized)
        {
            return false;
        }

        if (location.X >= sGraphicInfo->HorizontalResolution || location.Y >= sGraphicInfo->VerticalResolution)
        {
            return false;
        }

        FrameBufferColor dest_color = convertColor(color);
        addr_t offset = CALC_PIXEL_OFFSET(location.X, location.Y);
        FrameBufferColor *p = reinterpret_cast<FrameBufferColor *>(sGraphicInfo->FrameBufferBase + offset);
        *p = dest_color;
        return true;
    }

    void fillScreenWithBackgroundColor(void)
    {
        if (!sInitialized)
        {
            return;
        }

        for (u32 y = 0; y < sGraphicInfo->VerticalResolution; y++)
        {
            for (u32 x = 0; x < sGraphicInfo->HorizontalResolution; x++)
            {
                drawPixel(COORD(x, y), sBackgroundColor);
            }
        }
    }

    bool initGraphics(GraphicInfo* info, RGB initialColor)
    {
        if(sGraphicInfo != nullptr)
        {
            return true;
        }

        if(info == nullptr)
        {
            return false;
        }

        if(info->DisplayPixelFormat != PixelFormat::RGB && info->DisplayPixelFormat != PixelFormat::BGR)
        {
            return false;
        }

        sGraphicInfo = info;
        sInitialized = true;

        sBuffer = reinterpret_cast<u8*>(MemoryManager::allocInitializedMemory(sGraphicInfo->FrameBufferSize, NO_TRANSFER_BYTE));
        if(sBuffer == nullptr)
        {
            return false;
        }

        sBackgroundColor = initialColor;
        fillScreenWithBackgroundColor();
        return true;
    }

    bool drawPixelToBuffer(Point location, RGB color)
    {
        if(!sInitialized)
        {
            return false;
        }

        if(location.X >= sGraphicInfo->HorizontalResolution || location.Y >= sGraphicInfo->VerticalResolution)
        {
            return false;
        }

        FrameBufferColor dest_color = convertColor(color);
        addr_t offset = CALC_PIXEL_OFFSET(location.X, location.Y);
        FrameBufferColor *p = reinterpret_cast<FrameBufferColor *>(reinterpret_cast<addr_t>(sBuffer) + offset);
        *p = dest_color;
        return true;
    }

    bool drawBufferContentsToFrameBuffer(void)
    {
        if(!sInitialized)
        {
            return false;
        }

        Kernel::Utility::disableInterrupts();

        fillScreenWithBackgroundColor();

        for(u32 y=0; y<sGraphicInfo->VerticalResolution; y++)
        {
            for(u32 x=0; x<sGraphicInfo->HorizontalResolution; x++)
            {
                addr_t offset = CALC_PIXEL_OFFSET(x, y);
                FrameBufferColor* buf_addr = reinterpret_cast<FrameBufferColor*>(reinterpret_cast<addr_t>(sBuffer) + offset);
                if(buf_addr->Color4 != 0)
                {
                    drawPixel(COORD(x,y), convertColor(*buf_addr));
                }
            }
        }

        //UI::Cursor::updateCursor(UI::Cursor::getCursorPosition());

        Kernel::Utility::enableInterrupts();

        return true;
    }

    void clearBuffer(void)
    {
        if(!sInitialized)
        {
            return;
        }

        MemoryManager::fill(sBuffer, sGraphicInfo->FrameBufferSize, NO_TRANSFER_BYTE);
        //UI::Cursor::updateCursor(UI::Cursor::getCursorPosition());
    }

    void shiftBufferContents(u32 shiftPx, Direction direction)
    {
        if(!sInitialized || shiftPx == 0)
        {
            return;
        }

        if((shiftPx >= sGraphicInfo->VerticalResolution && (direction == Direction::VERITCAL_DOWN || direction == Direction::VERTICAL_UP)) ||
        (shiftPx >= sGraphicInfo->HorizontalResolution && (direction == Direction::HORIZONTAL_LEFT || direction == Direction::HORIZONTAL_RIGHT)))
        {
            clearBuffer();
            return;        
        }

        u8* buf = reinterpret_cast<u8*>(MemoryManager::allocInitializedMemory(sGraphicInfo->FrameBufferSize, NO_TRANSFER_BYTE));
        if(buf == nullptr)
        {
            PANIC(Status::STATUS_MEMORY_ALLOCATION_FAILED, sGraphicInfo->FrameBufferSize, 0, 0, 0);
        }

        u32 begin_x, end_x, begin_y, end_y;
        i64 delta_x, delta_y;

        switch(direction)
        {
            case Direction::VERTICAL_UP:
                begin_x = 0;
                end_x = sGraphicInfo->HorizontalResolution;
                begin_y = shiftPx;
                end_y = sGraphicInfo->VerticalResolution;
                delta_x = 0;
                delta_y = -(static_cast<i64>(shiftPx));
                break;
            case Direction::VERITCAL_DOWN:
                begin_x = 0;
                end_x = sGraphicInfo->HorizontalResolution;
                begin_y = 0;
                end_y = sGraphicInfo->VerticalResolution - shiftPx;
                delta_x = 0;
                delta_y = static_cast<i64>(shiftPx);
                break;
            case Direction::HORIZONTAL_LEFT:
                begin_x = shiftPx;
                end_x = sGraphicInfo->HorizontalResolution;
                begin_y = 0;
                end_y = sGraphicInfo->VerticalResolution;
                delta_x = -(static_cast<i64>(shiftPx));
                delta_y = 0;
                break;
            case Direction::HORIZONTAL_RIGHT:
                begin_x = 0;
                end_x = sGraphicInfo->HorizontalResolution - shiftPx;
                begin_y = 0;
                end_y = sGraphicInfo->VerticalResolution;
                delta_x = static_cast<i64>(shiftPx);
                delta_y = 0;
                break;
            default:
                MemoryManager::free(buf, sGraphicInfo->FrameBufferSize);
                PANIC(Status::STATUS_INVALID_PARAMETER, static_cast<u64>(direction), 0, 0, 0);
        }

        for(u32 y_src = begin_y; y_src < end_y; y_src++)
        {
            for(u32 x_src = begin_x; x_src < end_x; x_src++)
            {
                addr_t offset_src = CALC_PIXEL_OFFSET(x_src, y_src);
                addr_t offset_dest = CALC_PIXEL_OFFSET(static_cast<u64>(static_cast<i64>(x_src) + delta_x), static_cast<u64>(static_cast<i64>(y_src) + delta_y));

                if(offset_src < sGraphicInfo->FrameBufferSize && offset_dest < sGraphicInfo->FrameBufferSize)
                {
                    FrameBufferColor* src_px_addr = reinterpret_cast<FrameBufferColor*>(reinterpret_cast<addr_t>(sBuffer) + offset_src);
                    FrameBufferColor* dest_px_addr = reinterpret_cast<FrameBufferColor*>(reinterpret_cast<addr_t>(buf) + offset_dest);

                    if(src_px_addr->Color4 != 0)
                    {
                        *dest_px_addr = *src_px_addr;
                    }
                }
            }
        }

        clearBuffer();
        MemoryManager::memcopy(sBuffer, buf, sGraphicInfo->FrameBufferSize);
        MemoryManager::free(buf, sGraphicInfo->FrameBufferSize);
        //UI::Cursor::updateCursor(UI::Cursor::getCursorPosition());
    }

    RGB setBackgroundColor(RGB color)
    {
        RGB prev = sBackgroundColor;
        sBackgroundColor = color;
        //UI::Cursor::updateCursor(UI::Cursor::getCursorPosition());
        return prev;
    }

    RGB changeBackgroundColor(RGB color)
    {
        if(!sInitialized)
        {
            return color;
        }

        RGB prev = setBackgroundColor(color);
        fillScreenWithBackgroundColor();
        drawBufferContentsToFrameBuffer();
        //UI::Cursor::updateCursor(UI::Cursor::getCursorPosition());
        return prev;
    }

    Rectangle getScreenResolution(void)
    {
        if(!sInitialized)
        {
            return RECT(0, 0);
        }

        return RECT(sGraphicInfo->HorizontalResolution, sGraphicInfo->VerticalResolution);
    }

    void setEmptyPixelOnBuffer(Point location)
    {
        if(!sInitialized)
        {
            return;
        }

        if(location.X >= sGraphicInfo->HorizontalResolution || location.Y >= sGraphicInfo->VerticalResolution)
        {
            return;
        }

        addr_t offset = CALC_PIXEL_OFFSET(location.X, location.Y);
        FrameBufferColor* addr = reinterpret_cast<FrameBufferColor*>(reinterpret_cast<addr_t>(sBuffer) + offset);
        *addr = NO_TRANSFER_COLOR;
        //UI::Cursor::updateCursor(UI::Cursor::getCursorPosition());
    }

    void shiftBufferContentsAndDraw(u32 shiftPx, Direction direction)
    {
        if(!sInitialized)
        {
            return;
        }

        shiftBufferContents(shiftPx, direction);
        fillScreenWithBackgroundColor();
        drawBufferContentsToFrameBuffer();
        //UI::Cursor::updateCursor(UI::Cursor::getCursorPosition());
    }

    u32 getPixelsPerScanLine(void)
    {
        if(!sInitialized)
        {
            return 0;
        }

        return sGraphicInfo->PixelsPerScanLine;
    }

    addr_t getFrameBufferAddress(void)
    {
        if(!sInitialized)
        {
            return 0;
        }

        return sGraphicInfo->FrameBufferBase;
    }

    addr_t getBufferAddress(void)
    {
        if(!sInitialized)
        {
            return 0;
        }

        return reinterpret_cast<addr_t>(sBuffer);
    }
}