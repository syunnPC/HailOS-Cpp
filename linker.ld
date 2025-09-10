/*
 * HailOS Kernel Linker Script (x86-64)
 *
 * Loads the kernel at 1MB physical address.
 */

OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start) /* Assembly entry point */

/* Define the physical base address for the kernel */
KERNEL_BASE_ADDRESS = 0x100000; /* 1MB */

SECTIONS
{
    /* Begin placing sections at KERNEL_BASE_ADDRESS */
    . = KERNEL_BASE_ADDRESS;

    /*
     * .text section: Executable code
     * ALIGN(4096) aligns this section to a 4KB page boundary.
     */
    .text : ALIGN(4096)
    {
        *(.boot)            /* Our assembly _start code */
        *(.text .text.*)    /* All .text sections from input object files */
        *(.gnu.linkonce.t.*)
    }

    /*
     * .rodata section: Read-only data
     */
    .rodata : ALIGN(4096)
    {
        *(.rodata .rodata.*) /* All .rodata sections */
        *(.gnu.linkonce.r.*)
    }

    /*
     * .data section: Initialized read-write data
     */
    .data : ALIGN(4096)
    {
        *(.data .data.*)    /* All .data sections */
        *(.gnu.linkonce.d.*)
    }

    /*
     * .bss section: Uninitialized read-write data
     * FrostLoader will zero this section based on ELF program headers (p_filesz vs p_memsz).
     */
    .bss : ALIGN(4096)
    {
        _kernel_bss_start = .; /* Symbol to mark the start of .bss */
        *(.bss .bss.*)      /* All .bss sections */
        *(COMMON)           /* Common symbols (uninitialized globals) */
        _kernel_bss_end = .;   /* Symbol to mark the end of .bss */
    }

    /*
     * Define a symbol for the end of the kernel image in memory.
     * Useful for knowing the total memory footprint.
     */
    _kernel_end = ALIGN(4096); /* Align to the next page boundary */


    /*
     * Discard sections we don't need in the final kernel image.
     */
    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame)
        *(.note.gnu.build-id)
    }
}