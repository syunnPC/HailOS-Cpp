#pragma once

#include "basetype.hpp"

namespace HailOS::Kernel::Boot
{
    constexpr u16 GDT_NULL_SEL = 0x00;
    constexpr u16 GDT_KERNEL_CS = 0x08;
    constexpr u16 GDT_KERNEL_DS = 0x10;
    constexpr u16 GDT_USER_CS = 0x18;
    constexpr u16 GDT_USER_DS = 0x20;
    constexpr u16 GDT_TSS_SEL = 0x28;

    constexpr auto IDT_ENTRIES = 256;

    struct GDTR
    {
        u16 Limit;
        u64 Base;
    } PACKED;

    struct GDTEntry
    {
        u16 LimitLow;
        u16 BaseLow;
        u8 BaseMid;
        u8 Access;
        u8 Granularity;
        u8 BaseHigh;
    } PACKED;

    struct IDTR
    {
        u16 Limit;
        u64 Base;
    } PACKED;

    struct IDTEntry
    {
        u16 OffsetLow;
        u16 Selector;
        u8 Ist;
        u8 TypeAttribute;
        u16 OffsetMid;
        u32 OffsetHigh;
        u32 Reserved;
    } PACKED;

    struct TSS
    {
        u32 Reserved0;
        u64 Rsp0;
        u64 Rsp1;
        u64 Rsp2;
        u64 Reserved1;
        u64 Ist1;
        u64 Ist2;
        u64 Ist3;
        u64 Ist4;
        u64 Ist5;
        u64 Ist6;
        u64 Ist7;
        u64 Reserved2;
        u16 Reserved3;
        u16 IoMapBase;
    } PACKED;

    struct TSSDescriptor
    {
        u16 LimitLow;
        u16 BaseLow;
        u8 BaseMid;
        u8 Access;
        u8 Granularity;
        u8 BaseHigh;
        u32 BaseUpper;
        u32 Reserved;
    } PACKED;

    extern u8 gGDT[7 * sizeof(GDTEntry)];
    extern GDTR gGDTPtr;
    extern TSS gTSS;
    extern IDTEntry gIDT[IDT_ENTRIES];
    extern IDTR gIDTPtr;

    extern "C"
    {
        extern char _kernel_start[];
        extern char _kernel_end[];
    }

    extern "C" u8 kernel_stack_top[];

    void initGDTandTSS(void* kenrelStackTop);
    void initIDT(void);
}