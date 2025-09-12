#pragma once

#include "basetype.hpp"

namespace HailOS::PowerManager::ACPI
{
    struct SDTHeader
    {
        char Signature[4];
        u32 Length;
        u8 Revision;
        u8 Checksum;
        char OEMID[6];
        char OEMTableID[8];
        u32 OEMRevision;
        u32 CreatorID;
        u32 CreatorRev;
    } PACKED;

    struct RSDPtr
    {
        char Signature[8];
        u8 Checksum;
        char OEMID[6];
        u8 Revision;
        u32 RSDTAddress;
        u32 Length;
        u64 XSDTAddress;
        u8 ExtendedChecksum;
        u8 Reserved[3];
    } PACKED;

    struct GenericAddress
    {
        u8 AddressSpaceID;
        u8 RegisterBitWidth;
        u8 RegisterBitOffset;
        u8 AccessSize;
        u64 Address;
    } PACKED;

    struct FADT
    {
        SDTHeader Header;
        u32 FirmwareCtrl;
        u32 DSDT;
        u8 Reserved1;
        u8 PreferredPowerProfile;
        u16 SCIInterrupt;
        u32 SMICommandPort;
        u8 ACPIEnable;
        u8 ACPIDisable;
        u8 S4BIOSReq;
        u8 PStateCtrl;
        u32 PM1aEventBlock;
        u32 PM1bEventBlock;
        u32 PM1aCtrlBlock;
        u32 PM1bCtrlBlock;
        u32 PM2CtrlBlock;
        u32 PMTimerBlock;
        u32 GPE0Block;
        u32 GPE1Block;
        u8 PM1EventLength;
        u8 PM1CtrlLength;
        u8 PM2CtrlLength;
        u8 PMTimerLength;
        u8 GPE0Length;
        u8 GPE1Length;
        u8 GPE1Base;
        u8 CStateCtrl;
        u16 WorstC2Latency;
        u16 WorstC3Latency;
        u16 FlushSize;
        u16 FlushStride;
        u8 DutyOffset;
        u8 DutyWidth;
        u8 DayAlarm;
        u8 MonthAlarm;
        u8 Century;
        u16 BootArchitectureFlags;
        u8 Reserved2;
        u32 Flags;
        GenericAddress ResetReg;
        u8 ReserValue;
        u8 Reserved3[3];
        u64 X_FirmwareCtrl;
        u64 X_DSDT;
        GenericAddress X_PM1aEventBlock;
        GenericAddress X_PM1bEventBlock;
        GenericAddress X_PM1aCtrlBlock;
        GenericAddress X_PM1bCtrlBlock;
        GenericAddress X_PM2CtrlBlock;
        GenericAddress X_PMTimerBlock;
        GenericAddress X_GPE0Block;
        GenericAddress X_GPE1Block;
    } PACKED;

    bool initACPI(RSDPtr* ptr);
    void shutdown(void);
}