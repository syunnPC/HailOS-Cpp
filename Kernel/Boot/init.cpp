#include "init.hpp"
#include "basetype.hpp"
#include "cstring.hpp"
#include "interrupt.hpp"
#include "ps2kbd.hpp"
#include "ps2mouse.hpp"

#define IRQ_IDT(x) (32+x)

namespace HailOS::Kernel::Boot
{
    //必要なGDTエントリ:null+r0code+r0data+r3code+r3data+tss(2) = 7
    alignas(4096) u8 gGDT[7 * sizeof(GDTEntry)];
    alignas(4096) GDTR gGDTPtr;
    alignas(4096) TSS gTSS;
    alignas(4096) IDTEntry gIDT[IDT_ENTRIES];
    alignas(4096) IDTR gIDTPtr;

    constexpr auto IDT_TYPE_INTERRUPT_GATE = 0x8E;
    constexpr auto IDT_TYPE_TRAP_GATE = 0x8F;

    static inline void setGDTEntry(GDTEntry* entry, u32 base, u32 limit, u8 access, u8 gran)
    {
        entry->LimitLow = static_cast<u16>(limit & 0xFFFF);
        entry->BaseLow = static_cast<u16>(base & 0xFFFF);
        entry->BaseMid = static_cast<u8>((base >> 16) & 0xFF);
        entry->Access = access;
        entry->Granularity = static_cast<u8>(((limit >> 16) & 0x0F) | (gran & 0xF0));
        entry->BaseHigh = static_cast<u8>((base >> 24) & 0xFF);
    }

    static inline void setTSSDescriptor(TSSDescriptor* desc, u64 base, u32 limit, u8 access, u8 gran)
    {
        desc->LimitLow = static_cast<u16>(limit & 0xFFFF);
        desc->BaseLow = static_cast<u16>(base & 0xFFFF);
        desc->BaseMid = static_cast<u8>((base >> 16) & 0xFF);
        desc->Access = access;
        desc->Granularity = static_cast<u8>(((limit >> 16) & 0x0F) | (gran & 0xF0));
        desc->BaseHigh = static_cast<u8>((base >> 24) & 0xFF);
        desc->BaseUpper = static_cast<u32>((base >> 32) & 0xFFFFFFFF);
        desc->Reserved = 0;
    }

    extern "C" void loadGDT(const GDTR* gdtr)
    {
        asm volatile("lgdt (%0)" : : "r"(gdtr) : "memory");
    }

    extern "C" void reloadSegmentsAndLoadTSS(u16 kernelDS, u16 KernelCS, u16 userDSRPL3, u16 userCSRPL3, u16 tssSel)
    {
        asm volatile(
            "mov %0, %%ax\n\t"
            "mov %%ax, %%ds\n\t"
            "mov %%ax, %%es\n\t"
            "mov %%ax, %%ss\n\t"
            "mov %%ax, %%fs\n\t"
            "mov %%ax, %%gs\n\t"
            : : "r"(kernelDS) : "rax"
        );

        asm volatile(
            "pushq %0\n\t"
            "lea 1f(%%rip), %%rax\n\t"
            "pushq %%rax\n\t"
            "lretq\n\t"
            "1:\n\t"
            : : "r"(static_cast<u64>(KernelCS)) : "rax"
        );

        asm volatile("ltr %0" : : "r"(tssSel) : "memory");
    }

    void initGDTandTSS(void* kernelStackTop)
    {
        StdLib::C::memset(gGDT, 0, sizeof(gGDT));
        StdLib::C::memset(&gTSS, 0, sizeof(gTSS));

        GDTEntry* gdt = reinterpret_cast<GDTEntry*>(gGDT);

        setGDTEntry(&gdt[1], 0, 0, 0x9A, 0xA0);
        setGDTEntry(&gdt[2], 0, 0, 0x92, 0x00);
        setGDTEntry(&gdt[3], 0, 0, 0xFA, 0xA0);
        setGDTEntry(&gdt[4], 0, 0, 0xF2, 0x00);

        TSSDescriptor* tss = reinterpret_cast<TSSDescriptor*>(&gGDT[5 * sizeof(GDTEntry)]);
        u64 tss_base = reinterpret_cast<u64>(&gTSS);
        u32 tss_lim = static_cast<u32>(sizeof(TSS) - 1);

        setTSSDescriptor(tss, tss_base, tss_lim, 0x89, 0x00);

        gTSS.Rsp0 = reinterpret_cast<u64>(kernelStackTop);
        gTSS.IoMapBase = sizeof(TSS);
        gGDTPtr.Limit = static_cast<u16>(sizeof(gGDT) - 1);
        gGDTPtr.Base = reinterpret_cast<u64>(gGDT);

        loadGDT(&gGDTPtr);

        reloadSegmentsAndLoadTSS(GDT_KERNEL_DS, GDT_KERNEL_CS, (GDT_USER_DS | 0x3), (GDT_USER_CS | 0x3), GDT_TSS_SEL);
    }

    void setIDTEntry(int vec, void* handler,u8 typeAttribute)
    {
        u64 addr = reinterpret_cast<u64>(handler);
        gIDT[vec].OffsetLow = static_cast<u16>(addr & 0xFFFF);
        gIDT[vec].Selector = 0x08;
        gIDT[vec].Ist = 0;
        gIDT[vec].TypeAttribute = typeAttribute;
        gIDT[vec].OffsetMid = static_cast<u16>((addr >> 16) & 0xFFFF);
        gIDT[vec].OffsetHigh = static_cast<u32>((addr >> 32));
        gIDT[vec].Reserved = 0;
    }

    void initIDT(void)
    {
        using namespace Kernel::Boot;

        setIDTEntry(0, reinterpret_cast<void*>(handler0), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(1, reinterpret_cast<void*>(handler1), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(2, reinterpret_cast<void*>(handler2), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(3, reinterpret_cast<void*>(handler3), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(4, reinterpret_cast<void*>(handler4), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(5, reinterpret_cast<void*>(handler5), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(6, reinterpret_cast<void*>(handler6), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(7, reinterpret_cast<void*>(handler7), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(8, reinterpret_cast<void*>(handler8), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(9, reinterpret_cast<void*>(handler9), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(10, reinterpret_cast<void*>(handler10), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(11, reinterpret_cast<void*>(handler11), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(12, reinterpret_cast<void*>(handler12), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(13, reinterpret_cast<void*>(handler13), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(14, reinterpret_cast<void*>(handler14), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(15, reinterpret_cast<void*>(handler15), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(16, reinterpret_cast<void*>(handler16), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(17, reinterpret_cast<void*>(handler17), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(18, reinterpret_cast<void*>(handler18), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(19, reinterpret_cast<void*>(handler19), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(20, reinterpret_cast<void*>(handler20), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(21, reinterpret_cast<void*>(handler21), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(22, reinterpret_cast<void*>(handler22), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(23, reinterpret_cast<void*>(handler23), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(24, reinterpret_cast<void*>(handler24), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(25, reinterpret_cast<void*>(handler25), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(26, reinterpret_cast<void*>(handler26), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(27, reinterpret_cast<void*>(handler27), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(28, reinterpret_cast<void*>(handler28), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(29, reinterpret_cast<void*>(handler29), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(30, reinterpret_cast<void*>(handler30), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(31, reinterpret_cast<void*>(handler31), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(32, reinterpret_cast<void*>(handler32), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(33, reinterpret_cast<void*>(handler33), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(34, reinterpret_cast<void*>(handler34), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(35, reinterpret_cast<void*>(handler35), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(36, reinterpret_cast<void*>(handler36), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(37, reinterpret_cast<void*>(handler37), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(38, reinterpret_cast<void*>(handler38), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(39, reinterpret_cast<void*>(handler39), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(40, reinterpret_cast<void*>(handler40), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(41, reinterpret_cast<void*>(handler41), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(42, reinterpret_cast<void*>(handler42), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(43, reinterpret_cast<void*>(handler43), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(44, reinterpret_cast<void*>(handler44), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(45, reinterpret_cast<void*>(handler45), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(46, reinterpret_cast<void*>(handler46), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(47, reinterpret_cast<void*>(handler47), IDT_TYPE_INTERRUPT_GATE);
        /*
        setIDTEntry(48, reinterpret_cast<void*>(handler48), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(49, reinterpret_cast<void*>(handler49), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(50, reinterpret_cast<void*>(handler50), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(51, reinterpret_cast<void*>(handler51), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(52, reinterpret_cast<void*>(handler52), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(53, reinterpret_cast<void*>(handler53), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(54, reinterpret_cast<void*>(handler54), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(55, reinterpret_cast<void*>(handler55), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(56, reinterpret_cast<void*>(handler56), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(57, reinterpret_cast<void*>(handler57), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(58, reinterpret_cast<void*>(handler58), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(59, reinterpret_cast<void*>(handler59), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(60, reinterpret_cast<void*>(handler60), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(61, reinterpret_cast<void*>(handler61), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(62, reinterpret_cast<void*>(handler62), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(63, reinterpret_cast<void*>(handler63), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(64, reinterpret_cast<void*>(handler64), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(65, reinterpret_cast<void*>(handler65), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(66, reinterpret_cast<void*>(handler66), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(67, reinterpret_cast<void*>(handler67), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(68, reinterpret_cast<void*>(handler68), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(69, reinterpret_cast<void*>(handler69), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(70, reinterpret_cast<void*>(handler70), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(71, reinterpret_cast<void*>(handler71), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(72, reinterpret_cast<void*>(handler72), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(73, reinterpret_cast<void*>(handler73), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(74, reinterpret_cast<void*>(handler74), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(75, reinterpret_cast<void*>(handler75), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(76, reinterpret_cast<void*>(handler76), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(77, reinterpret_cast<void*>(handler77), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(78, reinterpret_cast<void*>(handler78), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(79, reinterpret_cast<void*>(handler79), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(80, reinterpret_cast<void*>(handler80), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(81, reinterpret_cast<void*>(handler81), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(82, reinterpret_cast<void*>(handler82), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(83, reinterpret_cast<void*>(handler83), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(84, reinterpret_cast<void*>(handler84), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(85, reinterpret_cast<void*>(handler85), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(86, reinterpret_cast<void*>(handler86), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(87, reinterpret_cast<void*>(handler87), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(88, reinterpret_cast<void*>(handler88), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(89, reinterpret_cast<void*>(handler89), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(90, reinterpret_cast<void*>(handler90), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(91, reinterpret_cast<void*>(handler91), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(92, reinterpret_cast<void*>(handler92), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(93, reinterpret_cast<void*>(handler93), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(94, reinterpret_cast<void*>(handler94), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(95, reinterpret_cast<void*>(handler95), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(96, reinterpret_cast<void*>(handler96), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(97, reinterpret_cast<void*>(handler97), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(98, reinterpret_cast<void*>(handler98), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(99, reinterpret_cast<void*>(handler99), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(100, reinterpret_cast<void *>(handler100), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(101, reinterpret_cast<void *>(handler101), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(102, reinterpret_cast<void *>(handler102), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(103, reinterpret_cast<void *>(handler103), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(104, reinterpret_cast<void *>(handler104), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(105, reinterpret_cast<void *>(handler105), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(106, reinterpret_cast<void *>(handler106), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(107, reinterpret_cast<void *>(handler107), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(108, reinterpret_cast<void *>(handler108), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(109, reinterpret_cast<void *>(handler109), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(110, reinterpret_cast<void *>(handler110), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(111, reinterpret_cast<void *>(handler111), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(112, reinterpret_cast<void *>(handler112), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(113, reinterpret_cast<void *>(handler113), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(114, reinterpret_cast<void *>(handler114), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(115, reinterpret_cast<void *>(handler115), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(116, reinterpret_cast<void *>(handler116), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(117, reinterpret_cast<void *>(handler117), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(118, reinterpret_cast<void *>(handler118), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(119, reinterpret_cast<void *>(handler119), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(120, reinterpret_cast<void *>(handler120), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(121, reinterpret_cast<void *>(handler121), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(122, reinterpret_cast<void *>(handler122), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(123, reinterpret_cast<void *>(handler123), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(124, reinterpret_cast<void *>(handler124), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(125, reinterpret_cast<void *>(handler125), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(126, reinterpret_cast<void *>(handler126), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(127, reinterpret_cast<void *>(handler127), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(128, reinterpret_cast<void *>(handler128), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(129, reinterpret_cast<void *>(handler129), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(130, reinterpret_cast<void *>(handler130), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(131, reinterpret_cast<void *>(handler131), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(132, reinterpret_cast<void *>(handler132), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(133, reinterpret_cast<void *>(handler133), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(134, reinterpret_cast<void *>(handler134), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(135, reinterpret_cast<void *>(handler135), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(136, reinterpret_cast<void *>(handler136), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(137, reinterpret_cast<void *>(handler137), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(138, reinterpret_cast<void *>(handler138), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(139, reinterpret_cast<void *>(handler139), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(140, reinterpret_cast<void *>(handler140), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(141, reinterpret_cast<void *>(handler141), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(142, reinterpret_cast<void *>(handler142), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(143, reinterpret_cast<void *>(handler143), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(144, reinterpret_cast<void *>(handler144), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(145, reinterpret_cast<void *>(handler145), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(146, reinterpret_cast<void *>(handler146), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(147, reinterpret_cast<void *>(handler147), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(148, reinterpret_cast<void *>(handler148), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(149, reinterpret_cast<void *>(handler149), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(150, reinterpret_cast<void *>(handler150), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(151, reinterpret_cast<void *>(handler151), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(152, reinterpret_cast<void *>(handler152), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(153, reinterpret_cast<void *>(handler153), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(154, reinterpret_cast<void *>(handler154), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(155, reinterpret_cast<void *>(handler155), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(156, reinterpret_cast<void *>(handler156), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(157, reinterpret_cast<void *>(handler157), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(158, reinterpret_cast<void *>(handler158), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(159, reinterpret_cast<void *>(handler159), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(160, reinterpret_cast<void *>(handler160), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(161, reinterpret_cast<void *>(handler161), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(162, reinterpret_cast<void *>(handler162), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(163, reinterpret_cast<void *>(handler163), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(164, reinterpret_cast<void *>(handler164), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(165, reinterpret_cast<void *>(handler165), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(166, reinterpret_cast<void *>(handler166), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(167, reinterpret_cast<void *>(handler167), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(168, reinterpret_cast<void *>(handler168), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(169, reinterpret_cast<void *>(handler169), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(170, reinterpret_cast<void *>(handler170), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(171, reinterpret_cast<void *>(handler171), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(172, reinterpret_cast<void *>(handler172), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(173, reinterpret_cast<void *>(handler173), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(174, reinterpret_cast<void *>(handler174), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(175, reinterpret_cast<void *>(handler175), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(176, reinterpret_cast<void *>(handler176), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(177, reinterpret_cast<void *>(handler177), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(178, reinterpret_cast<void *>(handler178), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(179, reinterpret_cast<void *>(handler179), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(180, reinterpret_cast<void *>(handler180), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(181, reinterpret_cast<void *>(handler181), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(182, reinterpret_cast<void *>(handler182), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(183, reinterpret_cast<void *>(handler183), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(184, reinterpret_cast<void *>(handler184), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(185, reinterpret_cast<void *>(handler185), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(186, reinterpret_cast<void *>(handler186), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(187, reinterpret_cast<void *>(handler187), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(188, reinterpret_cast<void *>(handler188), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(189, reinterpret_cast<void *>(handler189), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(190, reinterpret_cast<void *>(handler190), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(191, reinterpret_cast<void *>(handler191), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(192, reinterpret_cast<void *>(handler192), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(193, reinterpret_cast<void *>(handler193), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(194, reinterpret_cast<void *>(handler194), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(195, reinterpret_cast<void *>(handler195), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(196, reinterpret_cast<void *>(handler196), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(197, reinterpret_cast<void *>(handler197), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(198, reinterpret_cast<void *>(handler198), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(199, reinterpret_cast<void *>(handler199), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(200, reinterpret_cast<void *>(handler200), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(201, reinterpret_cast<void *>(handler201), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(202, reinterpret_cast<void *>(handler202), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(203, reinterpret_cast<void *>(handler203), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(204, reinterpret_cast<void *>(handler204), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(205, reinterpret_cast<void *>(handler205), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(206, reinterpret_cast<void *>(handler206), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(207, reinterpret_cast<void *>(handler207), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(208, reinterpret_cast<void *>(handler208), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(209, reinterpret_cast<void *>(handler209), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(210, reinterpret_cast<void *>(handler210), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(211, reinterpret_cast<void *>(handler211), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(212, reinterpret_cast<void *>(handler212), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(213, reinterpret_cast<void *>(handler213), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(214, reinterpret_cast<void *>(handler214), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(215, reinterpret_cast<void *>(handler215), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(216, reinterpret_cast<void *>(handler216), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(217, reinterpret_cast<void *>(handler217), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(218, reinterpret_cast<void *>(handler218), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(219, reinterpret_cast<void *>(handler219), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(220, reinterpret_cast<void *>(handler220), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(221, reinterpret_cast<void *>(handler221), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(222, reinterpret_cast<void *>(handler222), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(223, reinterpret_cast<void *>(handler223), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(224, reinterpret_cast<void *>(handler224), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(225, reinterpret_cast<void *>(handler225), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(226, reinterpret_cast<void *>(handler226), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(227, reinterpret_cast<void *>(handler227), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(228, reinterpret_cast<void *>(handler228), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(229, reinterpret_cast<void *>(handler229), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(230, reinterpret_cast<void *>(handler230), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(231, reinterpret_cast<void *>(handler231), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(232, reinterpret_cast<void *>(handler232), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(233, reinterpret_cast<void *>(handler233), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(234, reinterpret_cast<void *>(handler234), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(235, reinterpret_cast<void *>(handler235), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(236, reinterpret_cast<void *>(handler236), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(237, reinterpret_cast<void *>(handler237), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(238, reinterpret_cast<void *>(handler238), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(239, reinterpret_cast<void *>(handler239), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(240, reinterpret_cast<void *>(handler240), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(241, reinterpret_cast<void *>(handler241), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(242, reinterpret_cast<void *>(handler242), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(243, reinterpret_cast<void *>(handler243), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(244, reinterpret_cast<void *>(handler244), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(245, reinterpret_cast<void *>(handler245), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(246, reinterpret_cast<void *>(handler246), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(247, reinterpret_cast<void *>(handler247), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(248, reinterpret_cast<void *>(handler248), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(249, reinterpret_cast<void *>(handler249), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(250, reinterpret_cast<void *>(handler250), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(251, reinterpret_cast<void *>(handler251), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(252, reinterpret_cast<void *>(handler252), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(253, reinterpret_cast<void *>(handler253), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(254, reinterpret_cast<void *>(handler254), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(255, reinterpret_cast<void *>(handler255), IDT_TYPE_INTERRUPT_GATE);
        */

        for(int i=48; i<256; i++)
        {
            setIDTEntry(i, reinterpret_cast<void*>(handlerRet), IDT_TYPE_INTERRUPT_GATE);
        }

        setIDTEntry(IRQ_IDT(Driver::PS2::Keyboard::IRQ_KEYBOARD), reinterpret_cast<void*>(isrKeyboard), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(IRQ_IDT(Driver::PS2::Mouse::IRQ_MOUSE), reinterpret_cast<void*>(isrMouse), IDT_TYPE_INTERRUPT_GATE);
        setIDTEntry(IRQ_IDT(7), reinterpret_cast<void*>(isrIRQ7), IDT_TYPE_INTERRUPT_GATE);

        gIDTPtr.Base = reinterpret_cast<u64>(&gIDT);
        gIDTPtr.Limit = sizeof(gIDT) - 1;

        asm volatile("lidt %0" : : "m"(gIDTPtr));
    }
}