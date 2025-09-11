#pragma once

#include "basetype.hpp"
#include "vgatype.hpp"

#define MAKE_RGB(r, g, b) ((HailOS::Graphic::RGB){r, g, b})

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
    addr_t getBufferAddress(void);

    constexpr RGB COLOR_WHITE MAKE_RGB(255, 255, 255);
    constexpr RGB COLOR_SILVER MAKE_RGB(192, 192, 192);
    constexpr RGB COLOR_GRAY MAKE_RGB(128, 128, 128);
    constexpr RGB COLOR_BLACK MAKE_RGB(0, 0, 0);
    constexpr RGB COLOR_RED MAKE_RGB(255, 0, 0);
    constexpr RGB COLOR_MAROON MAKE_RGB(128, 0, 0);
    constexpr RGB COLOR_YELLOW MAKE_RGB(255, 255, 0);
    constexpr RGB COLOR_OLIVE MAKE_RGB(128, 128, 0);
    constexpr RGB COLOR_LIME MAKE_RGB(0, 255, 0);
    constexpr RGB COLOR_GREEN MAKE_RGB(0, 128, 0);
    constexpr RGB COLOR_AQUA MAKE_RGB(0, 255, 255);
    constexpr RGB COLOR_TEAL MAKE_RGB(0, 128, 128);
    constexpr RGB COLOR_BLUE MAKE_RGB(0, 0, 255);
    constexpr RGB COLOR_NAVY MAKE_RGB(0, 0, 128);
    constexpr RGB COLOR_FUCHSIA MAKE_RGB(255, 0, 255);
    constexpr RGB COLOR_PURPLE MAKE_RGB(128, 0, 128);
}