#include "hal_disk.hpp"
#include "basetype.hpp"
#include "ata.hpp"
#include "ahci.hpp"
#include "status.hpp"

namespace HailOS::HAL::Disk
{
    static DiskInfo sDiskInfo;

    bool initDisk(void)
    {
        Driver::AHCI::AHCIControllerInfo info;
        sDiskInfo.Abar = nullptr;
        sDiskInfo.PortIndex = 0;

        if(Driver::AHCI::findAHCIController(info))
        {
            sDiskInfo.Type = DiskType::AHCI;
            sDiskInfo.Abar = reinterpret_cast<volatile Driver::AHCI::HBAMemory*>(info.Abar);
            u32 port = Driver::AHCI::initPort(*sDiskInfo.Abar);
            sDiskInfo.PortIndex = port;
            Driver::AHCI::rebasePort(*sDiskInfo.Abar, port);
            Driver::AHCI::identifyDevice(*sDiskInfo.Abar, port);
            return true;
        }
        else if(Driver::ATA::checkMasterDevice())
        {
            sDiskInfo.Type = DiskType::IDE;
            return true;
        }
        else
        {
            return false;
        }
    }

    bool readSector(u64 lba, u8* buf)
    {
        if(buf == nullptr)
        {
            return false;
        }

        switch(sDiskInfo.Type)
        {
            case DiskType::IDE:
                return Driver::ATA::readSectorLBA28(static_cast<u32>(lba), buf);
            case DiskType::AHCI:
                return Driver::AHCI::readSector(*sDiskInfo.Abar, sDiskInfo.PortIndex, lba, buf);
            default:
                return false;
        }
    }
}