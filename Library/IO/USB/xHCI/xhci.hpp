#pragma once

#include "basetype.hpp"
#include "pci.hpp"

namespace HailOS::IO::USB::xHCI
{
    constexpr auto PCI_CLASS_SERIAL_BUS = 0x0C;
    constexpr auto PCI_SUBCLASS_USB = 0x03;
    constexpr auto PCI_PROGIF_XHCI = 0x30;

    //存在しなければ{0, 0, 0}を返す
    PCI::PCILocation getControllerLocation(void);

    struct MMIO
    {
        volatile u8* Cap;
        volatile u8* Op;
        volatile u32* Db;
        volatile u8* Rt;
    };

    struct CapRegs
    {
        u8 CAPLENGTH;
        u8 :8;
        u16 HCIVERSION;
        u32 HCSParams1;
        u32 HCSParams2;
        u32 HCSParams3;
        u32 HCCParams1;
        u32 DBOFF;
        u32 RTSOFF;
        u32 HCCParams2;
    } PACKED;

    struct OpRegs
    {
        u32 USBCMD;
        u32 USBSTS;
        u32 PAGESIZE;
        u32 Reserved1[2];
        u32 DNCTRL;
        u64 CRCR;
        u32 Reserved2[4];
        u64 DCBAAP;
        u32 CONFIG;
    } PACKED;

    struct PortRegs
    {
        u32 PORTSC;
        u32 PORTPMSC;
        u32 PORTLI;
        u32 PORTHLPMC;
    } PACKED;

    enum class TRBType : u8
    {
        Normal = 1,
        SetupStatge = 2,
        DataStage = 3,
        StatusStage = 4,
        Link = 6,
        EnableSlot = 9,
        AddressDevice = 11,
        ConfigurationEndPoint = 12,
        NoOpCmd = 23,
    };

    struct TRB
    {
        u32 P0;
        u32 P1;
        u32 P2;
        u32 P3;
    } PACKED;

    struct Ring
    {
        TRB* Base;
        u32 Size;
        u32 Enqueue;
        u8 Cycle;
    } PACKED;

    struct ERSTEntry
    {
        u64 RingBase;
        u32 RingSize;
        u32 Reserved;
    } PACKED;

    struct InterrupterRegs
    {
        u32 IMAN;
        u32 IMOD;
        u32 ERSTSZ;
        u32 Reserved;
        u64 ERSTBA;
        u64 ERDP;
    } PACKED;

    struct alignas(64) SlotContext
    {
        u32 RouteSpeed;
        u32 CtxEntries;
        u32 R2;
        u32 R3;
        u32 R4;
        u32 R5;
        u32 R6;
        u32 R7;
        u32 R8;
        u32 R9;
        u32 R10;
        u32 R11;
        u32 R12;
        u32 R13;
        u32 R14;
        u32 R15;
    };

    struct alignas(64) EpContext
    {
        u32 EPState;
        u32 TRDeqPtrLo;
        u32 TRDeqPtrHi;
        u32 TTT_MPS;
        u32 Reserved[12];
    };

    struct alignas(64) DeviceContext
    {
        SlotContext Slot;
        EpContext Ep[31];
    };

    struct alignas(64) InputControlCtx
    {
        u32 DropFlags;
        u32 AddFlags;
        u32 Reserved[6];
    };

    struct alignas(64) InputContext
    {
        InputControlCtx Ctrl;
        SlotContext Slot;
        EpContext Ep[31];
    };

    enum class EventType : u8
    {
        Transfer = 32,
        CommandCompletion = 33,
        PortStatusChange = 34,
    };

    bool initxHCI();
    bool xhciControllerSelfTest(void);
    void dumpPortStatus();
    bool portResetWait(u32 portIndex);
    void usbEventLoop();
}