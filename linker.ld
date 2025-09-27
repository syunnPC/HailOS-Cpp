OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)

KERNEL_BASE_ADDRESS = 0x100000;

SECTIONS
{
    . = KERNEL_BASE_ADDRESS;

    _kernel_start = .;

    .text : ALIGN(4096)
    {
        KEEP(*(.boot))
        *(.text .text.*)
        *(.gnu.linkonce.t.*)
    }

    .rodata : ALIGN(4096)
    {
        *(.rodata .rodata.*)
        *(.gnu.linkonce.r.*)
    }

    .data : ALIGN(4096)
    {
        *(.data .data.*)
        *(.gnu.linkonce.d.*)
    }

    .bss : ALIGN(4096)
    {
        _kernel_bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        _kernel_bss_end = .;
    }

    _kernel_end = ALIGN(4096);

    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame)
        *(.note.gnu.build-id)
    }
}