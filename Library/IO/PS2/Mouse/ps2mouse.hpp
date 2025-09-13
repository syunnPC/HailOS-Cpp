#pragma once

#include "basetype.hpp"
#include "vgatype.hpp"

namespace HailOS::Driver::PS2::Mouse
{
    constexpr auto IRQ_MOUSE = 12;

    struct MouseState
    {
        int X;
        int Y;
        bool LeftButton;
        bool RightButton;
        bool MiddleButton;
    };
    
    extern "C" void mouseHandler(void);
    bool initMouse(void);
    extern bool gMouseMoved;
    extern MouseState gMouseState;
}