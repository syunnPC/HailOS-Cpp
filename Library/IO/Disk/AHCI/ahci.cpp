#include "ahci.hpp"
#include "basetype.hpp"
#include "pci.hpp"
#include "timer.hpp"
#include "memutil.hpp"
#include "common.hpp"

#define HBA_PORT(base, i) (reinterpret_cast<volatile HailOS::Driver::AHCI::HBAPort*>(reinterpret_cast<volatile u8*>(base) + 0x100 + i*0x80))

namespace HailOS::Driver::AHCI
{
    static constexpr auto SATA_SIG_ATAPI = 0xEB140101;
    static constexpr auto SATA_SIG_SEMB = 0xC33C0101;
    static constexpr auto SATA_SIG_PM = 0x96690101;
    static constexpr auto SATA_SIG_SATA = 0x00000101;

    static constexpr auto AHCI_GHC_HR = (1 << 0);
    static constexpr auto AHCI_GHC_IE = (1 << 1);
    static constexpr auto AHCI_GHC_AE = (1 << 31);

    static constexpr auto HBA_PxCMD_ST = (1 << 0);
    static constexpr auto HBA_PxCMD_FRE = (1 << 4);
    static constexpr auto HBA_PxCMD_FR = (1 << 14);
    static constexpr auto HBA_PxCMD_CR = (1 << 15);

    static constexpr auto FIS_TYPE_REG_H2D = 0x27;
    static constexpr auto FIS_TYPE_REG_D2H = 0x34;

    static constexpr auto ATA_TFD_BSY = (1 << 7);
    static constexpr auto ATA_TFD_DRQ = (1 << 3);

    static constexpr auto TFES_BIT = (1u << 30);

    static constexpr auto ATA_CMD_IDENTIFY_DEVICE = 0xEC;
    static constexpr auto ATA_CMD_READ_DMA_EXT = 0x25;
    static constexpr auto ATA_CMD_WRITE_DMA_EXT = 0x35;
    static constexpr auto ATA_CMD_READ_DMA = 0xC8;
    static constexpr auto ATA_CMD_WRITE_DMA = 0xCA;

    static constexpr auto HBA_PxIS_TFES = 0x40000000;

    static constexpr auto PCI_CLASS_MASS_STORAGE = 0x01;
    static constexpr auto PCI_SUBCLASS_SATA = 0x06;
    static constexpr auto PCI_PROGIF_AHCI = 0x01;

    alignas(1024) static u8* sCmdListMemory[1024];
    alignas(256) static FISRecv sFisRecv;
    alignas(128) static HBACommandTable sCmdTable;
    static u8 sIdentifyBuf[512];

    static inline AHCIDeviceType checkType(volatile HBAPort& port)
    {
        u32 ssts = port.Ssts;
        u8 ipm = (ssts >> 8) & 0x0F;
        u8 det = ssts & 0x0F;

        if(det != 3)
        {
            return AHCIDeviceType::NULL_DEV;
        }

        if(ipm != 1)
        {
            return AHCIDeviceType::NULL_DEV;
        }

        switch (port.Sig)
        {
            case SATA_SIG_ATAPI:
                return AHCIDeviceType::SATAPI;
            case SATA_SIG_PM:
                return AHCIDeviceType::PM;
            case SATA_SIG_SEMB:
                return AHCIDeviceType::SEMB;
            case SATA_SIG_SATA:
                return AHCIDeviceType::SATA;
            default:
                return AHCIDeviceType::NULL_DEV;
        }
    }

    static bool isAHCIController(u8 bus, u8 device, u8 function)
    {
        u32 classcode = IO::PCI::readConfig32(bus, device, function, 0x08);
        u8 progif = (classcode >> 8) & 0xFF;
        u8 subclass = (classcode >> 16) & 0xFF;
        u8 pci_class = (classcode >> 24) & 0xFF;

        if(pci_class == PCI_CLASS_MASS_STORAGE && subclass == PCI_SUBCLASS_SATA && progif == PCI_PROGIF_AHCI)
        {
            return true;
        }

        return false;
    }

    static addr_t getAHCIMMIOAddress(u8 bus, u8 device, u8 function)
    {
        if(!isAHCIController(bus, device, function))
        {
            return reinterpret_cast<addr_t>(nullptr);
        }

        u32 abar_low = IO::PCI::readConfig32(bus, device, function, 0x24);
        return static_cast<addr_t>(abar_low & ~0xF);
    }

    bool findAHCIController(AHCIControllerInfo& out)
    {
        for(u16 bus=0; bus<256; bus++)
        {
            for(u8 dev=0; dev<32; dev++)
            {
                for(u8 func=0; func<8; func++)
                {
                    u32 id = IO::PCI::readConfig32(bus,dev,func,0x00);
                    IO::PCI::VendorID vendor = static_cast<IO::PCI::VendorID>(id & 0xFFFF);
                    if(vendor == IO::PCI::VendorID::INVALID)
                    {
                        continue;
                    }

                    if(!isAHCIController(bus, dev, func))
                    {
                        continue;
                    }

                    out.Bus = bus;
                    out.Device = dev;
                    out.Function = func;
                    out.Abar = getAHCIMMIOAddress(bus, dev, func);
                    return true;
                }
            }
        }

        return false;
    }

    static void resetEnable(volatile HBAMemory& hba)
    {
        u64 start = Utility::Timer::queryPerformanceCounter();
        hba.Ghc |= AHCI_GHC_HR;
        while(true)
        {
            if((hba.Ghc & AHCI_GHC_HR) == 0)
            {
                break;
            }

            if(Utility::Timer::convertPerformanceCounterDeltaToMs(Utility::Timer::queryPerformanceCounter() - start) > 1000)
            {
                PANIC(Status::STATUS_DISK_IO_ERROR, 0, 0, 0, 0);
            }
        }

        hba.Ghc |= AHCI_GHC_AE;
    }

    static void stopPort(volatile HBAPort& port)
    {
        u64 start = Utility::Timer::queryPerformanceCounter();
        port.Cmd &= ~HBA_PxCMD_ST;
        port.Cmd &= ~HBA_PxCMD_FRE;
        while(true)
        {
            if((port.Cmd & (HBA_PxCMD_CR | HBA_PxCMD_FR)) == 0)
            {
                break;
            }

            if(Utility::Timer::convertPerformanceCounterDeltaToMs(Utility::Timer::queryPerformanceCounter() - start) > 1000)
            {
                PANIC(Status::STATUS_DISK_IO_ERROR, 0, 0, 0, 0);
            }
        }
    }

    static void startPort(volatile HBAPort& port)
    {
        port.Cmd |= HBA_PxCMD_FRE;
        port.Cmd |= HBA_PxCMD_ST;
    }

    u32 initPort(volatile HBAMemory& abar)
    {
        u32 pi = abar.Pi;
        for(int i=0; i<32; i++)
        {
            if(!(pi & (1 << i)))
            {
                continue;
            }

            volatile HBAPort& port = *HBA_PORT(&abar, i);
            AHCIDeviceType type = checkType(port);
            if(type == AHCIDeviceType::SATA)
            {
                stopPort(port);
                port.Clb = 0;
                port.Clbu = 0;
                port.Fb = 0;
                port.Fbu = 0;
                startPort(port);

                return i;
            }
        }

        return static_cast<u32>(-1);
    }

    void rebasePort(volatile HBAMemory& abar, int portIndex)
    {
        volatile HBAPort& port = *HBA_PORT(&abar, portIndex);

        MemoryManager::fill(sCmdListMemory, sizeof(sCmdListMemory), 0);
        MemoryManager::fill(&sFisRecv, sizeof(sFisRecv), 0);
        MemoryManager::fill(&sCmdTable, sizeof(sCmdTable), 0);

        port.Clb = static_cast<u32>(reinterpret_cast<addr_t>(sCmdListMemory));
        port.Clbu = 0;
        port.Fb = static_cast<u32>(reinterpret_cast<addr_t>(&sFisRecv));
        port.Fbu = 0;

        HBACommandHeader& hdr = *reinterpret_cast<HBACommandHeader*>(reinterpret_cast<addr_t>(sCmdListMemory));
        hdr.Cfl = sizeof(FISRegH2D) / 4;
        hdr.W = 0;
        hdr.Prdtl = 1;
        hdr.Ctba = static_cast<u32>(reinterpret_cast<addr_t>(&sCmdTable));
        hdr.Ctbau = 0;
        hdr.Prdbc = 0;
        hdr.Pmp = 0;
        hdr.Rsv0 = 0;
        MemoryManager::fill(&hdr.Rsv1, sizeof(u32)*4, 0);

        HBACommandTable& cmdtbl = sCmdTable;
        MemoryManager::fill(cmdtbl.Cfis, sizeof(cmdtbl.Cfis), 0);
        MemoryManager::fill(cmdtbl.Acmd, sizeof(cmdtbl.Acmd), 0);
        MemoryManager::fill(cmdtbl.Prdt, sizeof(cmdtbl.Prdt), 0);
    }

    __attribute__((optimize("O0"))) bool identifyDevice(volatile HBAMemory& abar, int portIndex)
    {
        volatile HBAPort& port = *HBA_PORT(&abar, portIndex);

        u64 start = Utility::Timer::queryPerformanceCounter();
        while(port.Tfd & (ATA_TFD_BSY | ATA_TFD_DRQ))
        {
            if(Utility::Timer::convertPerformanceCounterDeltaToMs(Utility::Timer::queryPerformanceCounter() - start) > 1000)
            {
                PANIC(Status::STATUS_DISK_IO_ERROR, 0, 0, 0, 0);
            }
        }

        port.Clb = static_cast<u32>(reinterpret_cast<addr_t>(sCmdListMemory));
        port.Clbu = 0;
        port.Fb = static_cast<u32>(reinterpret_cast<addr_t>(&sCmdTable));
        port.Fbu = 0;
        
        HBACommandHeader& hdr = *reinterpret_cast<HBACommandHeader*>(reinterpret_cast<addr_t>(sCmdListMemory));
        hdr.Cfl = sizeof(FISRegH2D)/4;
        hdr.W = 0;
        hdr.Prdtl = 1;
        hdr.Ctba = static_cast<u32>(reinterpret_cast<addr_t>(&sCmdTable));
        hdr.Ctbau = 0;
        hdr.Prdbc = 0;

        HBAPRDTEntry& prdt = sCmdTable.Prdt[0];
        prdt.Dba = static_cast<u32>(reinterpret_cast<addr_t>(sIdentifyBuf));
        prdt.Dbau = 0;
        prdt.DbcI = (512 - 1) | (1u << 31);

        FISRegH2D* cfis = reinterpret_cast<FISRegH2D*>(sCmdTable.Cfis);
        for(int i=0; i<64; i++)
        {
            reinterpret_cast<u8*>(cfis)[i] = 0;
        }

        cfis->FisType = FIS_TYPE_REG_H2D;
        cfis->C = 1;
        cfis->Command = ATA_CMD_IDENTIFY_DEVICE;
        cfis->Device = 1 << 6;

        port.Is = static_cast<u32>(0xFFFFFFFF);
        port.Ci = 1 << 0;

        start = Utility::Timer::queryPerformanceCounter();
        while(port.Ci & (1 << 0))
        {
            if(port.Is & TFES_BIT)
            {
                PANIC(Status::STATUS_DISK_IO_ERROR, 0, 0, 0, 0);
            }

            if(Utility::Timer::convertPerformanceCounterDeltaToMs(Utility::Timer::queryPerformanceCounter() - start) > 1000)
            {
                PANIC(Status::STATUS_DISK_IO_ERROR, 0, 0, 0, 0);
            }
        }

        if(port.Is & TFES_BIT)
        {
            PANIC(Status::STATUS_DISK_IO_ERROR, 0, 0, 0, 0);
        }

        return true;
    }

    bool initAHCI(void)
    {
        AHCIControllerInfo info;
        if(!findAHCIController(info))
        {
            return false;
        }

        volatile HBAMemory& abar = *reinterpret_cast<volatile HBAMemory*>(info.Abar);
        int port = initPort(abar);
        rebasePort(abar, port);
        return identifyDevice(abar, port);
    }

    __attribute__((optimize("O0")))bool readSector(volatile HBAMemory& abar, int portIndex, u64 lba, u8* buffer)
    {
        if(buffer == nullptr)
        {
            return false;
        }

        volatile HBAPort& port = *HBA_PORT(&abar, portIndex);

        HBACommandHeader* hdrs = reinterpret_cast<HBACommandHeader*>(reinterpret_cast<addr_t>(sCmdListMemory));
        HBACommandHeader& cmdh = hdrs[0];
        
        cmdh.Cfl = sizeof(FISRegH2D)/4;
        cmdh.W = 0;
        cmdh.Prdtl = 1;
        
        MemoryManager::fill(&sCmdTable, sizeof(sCmdTable), 0);

        HBAPRDTEntry& prdt = sCmdTable.Prdt[0];
        prdt.Dba = static_cast<u32>(reinterpret_cast<addr_t>(buffer));
        prdt.Dbau = (reinterpret_cast<u64>(buffer) >> 32);
        prdt.DbcI = (512 - 1) | (1u << 31);

        FISRegH2D& cmdfis = *reinterpret_cast<FISRegH2D*>(&sCmdTable.Cfis);
        MemoryManager::fill(&cmdfis, sizeof(FISRegH2D), 0);

        cmdfis.FisType = FIS_TYPE_REG_H2D;
        cmdfis.C = 1;
        cmdfis.Command = ATA_CMD_READ_DMA_EXT;
        cmdfis.Lba0 = static_cast<u8>(lba & 0xFF);
        cmdfis.Lba1 = static_cast<u8>((lba >> 8) & 0xFF);
        cmdfis.Lba2 = static_cast<u8>((lba >> 16) & 0xFF);
        cmdfis.Device = 1 << 6;
        cmdfis.Lba3 = static_cast<u8>((lba >> 24) & 0xFF);
        cmdfis.Lba4 = static_cast<u8>((lba >> 32) % 0xFF);
        cmdfis.Lba5 = static_cast<u8>((lba >> 40) & 0xFF);

        cmdfis.Countl = 1;
        cmdfis.Counth = 0;

        cmdh.Ctba = static_cast<u32>(reinterpret_cast<addr_t>(&sCmdTable));
        cmdh.Ctbau = 0;
        
        port.Is = 0xFFFFFFFF;
        port.Ci = 1 << 0;

        u64 start = Utility::Timer::queryPerformanceCounter();
        while(true)
        {
            if((port.Ci & (1 << 0)) == 0)
            {
                break;
            }

            if(port.Is & HBA_PxIS_TFES)
            {
                return false;
            }

            if(Utility::Timer::convertPerformanceCounterDeltaToMs(Utility::Timer::queryPerformanceCounter() - start) > 1000)
            {
                return false;
            }
        }

        return true;
    }
}