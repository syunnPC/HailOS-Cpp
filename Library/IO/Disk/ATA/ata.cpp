#include "basetype.hpp"
#include "io.hpp"
#include "common.hpp"
#include "status.hpp"
#include "time.hpp"

namespace HailOS::Driver::ATA
{
    static constexpr auto ATA_DATA_PORT = 0x1F0;
    static constexpr auto ATA_ERROR_PORT = 0x1F1;
    static constexpr auto ATA_SECTOR_COUNT = 0x1F2;
    static constexpr auto ATA_LBA_LOW = 0x1F3;
    static constexpr auto ATA_LBA_MID = 0x1F4;
    static constexpr auto ATA_LBA_HIGH = 0x1F5;
    static constexpr auto ATA_DRIVE_SELECT = 0x1F6;
    static constexpr auto ATA_COMMAND_PORT = 0x1F7;
    static constexpr auto ATA_STATUS_PORT = 0x1F7;
    static constexpr auto ATA_CTRL_BASE = 0x3F6;
    static constexpr auto ATA_CMD_READ_SECTORS = 0x20;
    static constexpr auto ATA_CMD_IDENTIFY = 0xEC;
    static constexpr auto ATA_STATUS_BSY = 0x80;
    static constexpr auto ATA_STATUS_DRQ = 0x08;
    static constexpr auto ATA_STATUS_ERR = 0x01;
    static constexpr auto ATA_REG_STATUS = 0x07;
    static constexpr auto ATA_REG_DRIVE_HEAD = 0x06;

    static constexpr auto SLEEP_LENGTH = 500;

    bool checkMasterDevice(void)
    {
        IO::outb(ATA_DATA_PORT + ATA_REG_DRIVE_HEAD, 0xA0);
        Utility::Time::SleepNano(SLEEP_LENGTH);
        IO::outb(ATA_DATA_PORT + ATA_REG_STATUS, 0x00);
        Utility::Time::SleepNano(SLEEP_LENGTH);
        IO::outb(ATA_DATA_PORT + ATA_REG_STATUS, ATA_CMD_IDENTIFY);
        Utility::Time::SleepNano(SLEEP_LENGTH);

        u8 status = IO::inb(ATA_DATA_PORT + ATA_REG_STATUS);
        if(status == 0)
        {
            return false;
        }

        u64 f = Utility::Time::queryPerformanceCounterFreq();
        double cpns = f / 1000000000;
        u64 waitclk = static_cast<u64>(cpns * SLEEP_LENGTH);
        u64 start = Utility::Time::queryPerformanceCounter();
        while(IO::inb(ATA_DATA_PORT + ATA_REG_STATUS) & ATA_STATUS_BSY)
        {
            if(Utility::Time::queryPerformanceCounter() - start > waitclk)
            {
                return false;
            }
        }

        if(Utility::Time::convertPerformanceCounterDeltaToMs(Utility::Time::queryPerformanceCounter() - start) >= 1000)
        {
            return false;
        }

        if(status & ATA_STATUS_ERR)
        {
            return false;
        }

        if(status & ATA_STATUS_DRQ)
        {
            return true;
        }

        return false;
    }

    bool readSectorLBA28(u32 lba, u8* buffer)
    {
        using namespace HailOS::IO;

        if(!checkMasterDevice())
        {
            return false;
        }

        if(buffer == nullptr)
        {
            return false;
        }

        while(inb(ATA_STATUS_PORT) & ATA_STATUS_BSY);

        outb(ATA_SECTOR_COUNT, 1);
        outb(ATA_LBA_LOW, static_cast<u8>(lba & 0xFF));
        outb(ATA_LBA_MID, static_cast<u8>((lba >> 8) & 0xFF));
        outb(ATA_LBA_HIGH, static_cast<u8>((lba >> 16) & 0xFF));
        outb(ATA_DRIVE_SELECT, static_cast<u8>((lba >> 24) & 0x0F));
        outb(ATA_COMMAND_PORT, ATA_CMD_READ_SECTORS);

        while(true)
        {
            u8 status = inb(ATA_STATUS_PORT);
            if(status & ATA_STATUS_ERR)
            {
                return false;
            }
            if(!(status & ATA_STATUS_BSY) && (status & ATA_STATUS_DRQ))
            {
                break;
            }
        }

        for(int i=0; i<256; i++)
        {
            u16 word = inw(ATA_DATA_PORT);
            buffer[i*2] = static_cast<u8>(word & 0xFF);
            buffer[i*2+1] = static_cast<u8>((word >> 8) & 0xFF);
        }

        return true;
    }
}