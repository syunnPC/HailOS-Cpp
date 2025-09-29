.section .boot
.global _start
.global kernel_stack_top
.global kernel_stack_bottom

_start:
    cli
    movabs $kernel_stack_top, %rsp
    sub $8, %rsp
    cld
    jmp main
hang_loop:
    cli
    hlt
    jmp hang_loop

.section .bss
.align 16
kernel_stack_bottom:
    .skip 4096*4
kernel_stack_top: