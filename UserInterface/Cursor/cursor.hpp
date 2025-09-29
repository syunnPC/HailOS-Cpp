#pragma once

#include "vgatype.hpp"
#include "basetype.hpp"
#include "vga.hpp"

namespace HailOS::UI::Cursor
{
    constexpr Graphic::RGB CURSOR_DEFAULT_COLOR = Graphic::COLOR_BLACK;

    extern Graphic::FrameBufferColor** gCursorImage;
    extern Graphic::FrameBufferColor** gBufferContentsUnderCursor;
    extern Graphic::Rectangle gCursorSize;
    extern const u8 gCursor[];

    bool initCursor();
    void updateCursor(Graphic::Point location);
    void updateBufferUnderCursor(void);
    bool loadCursor(const Graphic::RGB** source, Graphic::Rectangle size);
    Graphic::Point getCursorPosition();
    void drawCursor();
    bool isInitialized();
    Graphic::Rectangle getCursorSize();
}