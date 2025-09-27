#pragma once

namespace HailOS::Kernel::Utility
{
    void haltProcessor(void);
    void enableInterrupts(void);
    void disableInterrupts(void);  
    void setForceDisableInterrupt(bool state);
    void reset(void);
}
