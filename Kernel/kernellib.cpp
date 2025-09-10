namespace HailOS::Kernel::Utility
{
    void enableInterrupts(void)
    {
        asm volatile("sti");
    }

    void disableInterrupts(void)
    {
        asm volatile("cli");
    }

    [[noreturn]] void haltProcessor(void)
    {
        while (true)
        {
            asm volatile("cli");
            asm volatile("hlt");
        }
    }
}