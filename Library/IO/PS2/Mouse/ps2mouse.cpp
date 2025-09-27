#include "basetype.hpp"
#include "ps2mouse.hpp"
#include "ps2.hpp"
#include "io.hpp"
#include "pic.hpp"
#include "vgatype.hpp"
#include "vga.hpp"
#include "cursor.hpp"
#include "kernellib.hpp"

namespace HailOS::Driver::PS2::Mouse
{
    static constexpr auto PS2_WAIT_LOOP = 1000000;

    bool gMouseMoved = false;

    MouseState gMouseState;
    static u8 sMouseCycle;
    static u8 sMouseBytes[4];

    static Graphic::Rectangle sScreenResolution;

    static bool sInitialized = false;
    
    extern "C" void mouseHandler(void)
    {
        if(!sInitialized)
        {
            IO::PIC::sendEOI(IRQ_MOUSE);
            return;
        }

        u8 status = IO::inb(PS2_STATUS_PORT);

        if(!(status & 0x20))
        {
            IO::PIC::sendEOI(IRQ_MOUSE);
            return;
        }

        u8 data = IO::inb(PS2_DATA_PORT);

        if(sMouseCycle == 0 && !(data & 0x08))
        {
            IO::PIC::sendEOI(IRQ_MOUSE);
            return;
        }

        sMouseBytes[sMouseCycle++] = data;

        if(sMouseCycle == 3)
        {
            u8 b = sMouseBytes[0];

            //bool sx = b & 0x10;
            //bool sy = b & 0x20;

            int dx = static_cast<i8>(sMouseBytes[1]);
            int dy = static_cast<i8>(sMouseBytes[2]);

            gMouseState.X += dx;
            gMouseState.Y -= dy;

            if(gMouseState.X < 0)
            {
                gMouseState.X = 0;
            }

            if(gMouseState.Y < 0)
            {
                gMouseState.Y = 0;
            }

            if(gMouseState.X >= sScreenResolution.Width)
            {
                gMouseState.X = sScreenResolution.Width - 1;
            }

            if(gMouseState.Y >= sScreenResolution.Height)
            {
                gMouseState.Y = sScreenResolution.Height - 1;
            }

            gMouseState.LeftButton = ((b & 0x01) != 0);
            gMouseState.RightButton = ((b & 0x02) != 0);
            gMouseState.MiddleButton = ((b & 0x04) != 0);

            gMouseMoved = true;

            sMouseCycle = 0;
        }

        IO::PIC::sendEOI(IRQ_MOUSE);
    }

    static bool waitInputClear(void)
    {
        for(int i=0; i<PS2_WAIT_LOOP; i++)
        {
            if((IO::inb(PS2_STATUS_PORT) & 0x02) == 0)
            {
                return true;
            }
        }

        return false;
    }

    static bool waitOutputFull(void)
    {
        for(int i=0; i<PS2_WAIT_LOOP; i++)
        {
            if(IO::inb(PS2_STATUS_PORT) & 0x01)
            {
                return true;
            }
        }

        return false;
    }

    static bool writeCtrlCmd(u8 cmd)
    {
        if(!waitInputClear())
        {
            return false;
        }

        IO::outb(PS2_STATUS_PORT, cmd);
        return true;
    }

    static bool writeCtrlData(u8 data)
    {
        if(!waitInputClear())
        {
            return false;
        }

        IO::outb(PS2_DATA_PORT, data);
        return true;
    }

    static bool writeMouse(u8 data)
    {
        if(!waitInputClear())
        {
            return false;
        }

        IO::outb(PS2_STATUS_PORT, 0xD4);

        if(!waitInputClear())
        {
            return false;
        }

        IO::outb(PS2_DATA_PORT, data);
        return true;
    }

    static bool waitAck(int timeout)
    {
        for(int i=0; i<timeout; i++)
        {
            if(waitOutputFull())
            {
                u8 v =IO::inb(PS2_DATA_PORT);
                if(v == 0xFA)
                {
                    return true;
                }
            }
        }

        return false;
    }

    bool initMouse(void)
    {
        if(sInitialized)
        {
            return true;
        }

        Kernel::Utility::disableInterrupts();
        IO::PIC::mask(IRQ_MOUSE);

        if(!writeCtrlCmd(0xA8))
        {
            Kernel::Utility::enableInterrupts();
            IO::PIC::unmask(IRQ_MOUSE);
            return false;
        }

        if(!writeCtrlCmd(0x20))
        {
            Kernel::Utility::enableInterrupts();
            IO::PIC::unmask(IRQ_MOUSE);
            return false;
        }

        int wait = PS2_WAIT_LOOP;
        while(wait--)
        {
            if(IO::inb(PS2_STATUS_PORT) & 0x01)
            {
                break;
            }
        }

        if(wait <= 0)
        {
            Kernel::Utility::enableInterrupts();
            IO::PIC::unmask(IRQ_MOUSE);
            return false;
        }

        u8 status = IO::inb(PS2_DATA_PORT);

        status |= 0x03;

        if(!writeCtrlCmd(0x60))
        {
            Kernel::Utility::enableInterrupts();
            IO::PIC::unmask(IRQ_MOUSE);
            return false;
        }

        if(!writeCtrlData(status))
        {
            Kernel::Utility::enableInterrupts();
            IO::PIC::unmask(IRQ_MOUSE);
            return false;
        }

        for(int i=0; i<16; i++)
        {
            if(IO::inb(PS2_STATUS_PORT) & 0x01)
            {
                (void)IO::inb(PS2_DATA_PORT);
            }
        }

        if(!writeMouse(0xF6))
        {
            Kernel::Utility::enableInterrupts();
            IO::PIC::unmask(IRQ_MOUSE);
            return false;
        }

        if(!waitAck(PS2_WAIT_LOOP))
        {
            Kernel::Utility::enableInterrupts();
            IO::PIC::unmask(IRQ_MOUSE);
            return false;
        }

        if(!writeMouse(0xF4))
        {
            Kernel::Utility::enableInterrupts();
            IO::PIC::unmask(IRQ_MOUSE);
            return false;
        }

        if(!waitAck(PS2_WAIT_LOOP))
        {
            Kernel::Utility::enableInterrupts();
            IO::PIC::unmask(IRQ_MOUSE);
            return false;
        }

        gMouseState.X = 0;
        gMouseState.Y = 0;

        sInitialized = true;
        sScreenResolution = Graphic::getScreenResolution();
        IO::PIC::unmask(IRQ_MOUSE);
        Kernel::Utility::enableInterrupts();
        return true;
    }
}