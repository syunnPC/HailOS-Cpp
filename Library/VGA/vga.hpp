#pragma once

#include "basetype.hpp"
#include "vgatype.hpp"

#define RGB(r, g, b) ((HailOS::Graphic::RGB){r, g, b})

namespace HailOS::Graphic
{
    bool isGraphicAvailable(void);
    bool initGraphics(GraphicInfo* info, RGB initialColor);
    FrameBufferColor convertColor(RGB color);
    RGB convertColor(FrameBufferColor color);
    bool drawPixel(Point location, RGB color);
    bool drawPixelToBuffer(Point location, RGB color);
    bool drawBufferContentsToFrameBuffer(void);
    void shiftBufferContents(u32 shiftPx, Direction direction);
    void clearBuffer(void);
    RGB setBackgroundColor(RGB color);
    RGB changeBackgroundColor(RGB color);
    void fillScreenWithBackgroundColor(void);
    Rectangle getScreenResolution(void);
    void setEmptyPixelOnBuffer(Point location);
    void shiftBufferContentsAndDraw(u32 shiftPx, Direction direction);
    u32 getPixelsPerScanLine(void);
    addr_t getFrameBufferAddress(void);

    constexpr RGB COLOR_WHITE RGB(255, 255, 255);
    constexpr RGB COLOR_SILVER RGB(192, 192, 192);
    constexpr RGB COLOR_GRAY RGB(128, 128, 128);
    constexpr RGB COLOR_BLACK RGB(0, 0, 0);
    constexpr RGB COLOR_RED RGB(255, 0, 0);
    constexpr RGB COLOR_MAROON RGB(128, 0, 0);
    constexpr RGB COLOR_YELLOW RGB(255, 255, 0);
    constexpr RGB COLOR_OLIVE RGB(128, 128, 0);
    constexpr RGB COLOR_LIME RGB(0, 255, 0);
    constexpr RGB COLOR_GREEN RGB(0, 128, 0);
    constexpr RGB COLOR_AQUA RGB(0, 255, 255);
    constexpr RGB COLOR_TEAL RGB(0, 128, 128);
    constexpr RGB COLOR_BLUE RGB(0, 0, 255);
    constexpr RGB COLOR_NAVY RGB(0, 0, 128);
    constexpr RGB COLOR_FUCHSIA RGB(255, 0, 255);
    constexpr RGB COLOR_PURPLE RGB(128, 0, 128);
}