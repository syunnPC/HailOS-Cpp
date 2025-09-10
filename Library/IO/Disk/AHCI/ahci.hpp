#pragma once

#include "basetype.hpp"

namespace HailOS::Driver::AHCI
{
    enum class AHCIDeviceType
    {
        NULL_DEV = 0,
        SATA, 
        SEMB,
        PM,
        SATAPI
    };

    struct HBAMemory
    {
        u32 Cap;
        u32 Ghc;
        u32 Is;
        u32 Pi;
        u32 Vs;
        u32 CccCtl;
        u32 CccPts;
        u32 EmLock;
        u32 EmCtl;
        u32 Cap2;
        u32 Bohc;
        u8 Rsv[0xA0 - 0x2C];
        u8 Vendor[0x100 - 0xA0];
    } PACKED;

    struct HBAPort
    {
        u32 Clb;
        u32 Clbu;
        u32 Fb;
        u32 Fbu;
        u32 Is;
        u32 Ie;
        u32 Cmd;
        u32 Rsv0;
        u32 Tfd;
        u32 Sig;
        u32 Ssts;
        u32 Sctl;
        u32 Serr;
        u32 Sact;
        u32 Ci;
        u32 Sntf;
        u32 Fbs;
        u32 Rsv1[11];
        u32 Vendor[4];
    } PACKED;

    struct HBAPRDTEntry
    {
        u32 Dba;
        u32 Dbau;
        u32 Rsv0;
        u32 DbcI;
    } PACKED;

    struct alignas(128) HBACommandTable
    {
        u8 Cfis[64];
        u8 Acmd[16];
        u8 Rsv[48];
        HBAPRDTEntry Prdt[1];
    } PACKED;

    struct HBACommandHeader
    {
        u8 Cfl:5;
        u8 A:1;
        u8 W:1;
        u8 P:1;
        u8 R:1;
        u8 B:1;
        u8 C:1;
        u8 Rsv0:1;
        u8 Pmp:4;
        u16 Prdtl;
        volatile u32 Prdbc;
        u32 Ctba;
        u32 Ctbau;
        u32 Rsv1[4];
    } PACKED;

    struct alignas(256) FISRecv
    {
        u8 Dsfis[0x1C];
        u8 Rsv0[0x04];
        u8 Psfis[0x14];
        u8 Rsv1[0x0C];
        u8 Rfis[0x14];
        u8 Rsv2[0x04];
        u8 Sdbfis[0x08];
        u8 Ufis[0x40];
        u8 Rsv3[0x60];
    } PACKED;

    struct FISRegH2D
    {
        u8 FisType;
        u8 Pmport:4;
        u8 Rsv0:3;
        u8 C:1;
        u8 Command;
        u8 Feature1;
        u8 Lba0;
        u8 Lba1;
        u8 Lba2;
        u8 Device;
        u8 Lba3;
        u8 Lba4;
        u8 Lba5;
        u8 Featureh;
        u8 Countl;
        u8 Counth;
        u8 Icc;
        u8 Control;
        u8 Rsv1[4];
    } PACKED;

    struct AHCIControllerInfo
    {
        u8 Bus;
        u8 Device;
        u8 Function;
        addr_t Abar;
    };

    bool initAHCI(void);
    bool readSector(volatile HBAMemory& abar, int portIndex, u64 lba, u8* buffer);
    bool findAHCIController(AHCIControllerInfo &out);
    void rebasePort(volatile HBAMemory &abar, int portIndex);
    u32 initPort(volatile HBAMemory &abar);
    bool identifyDevice(volatile HBAMemory &abar, int portIndex);
}