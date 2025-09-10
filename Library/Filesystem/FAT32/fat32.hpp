#pragma once

#include "basetype.hpp"
#include "status.hpp"

namespace HailOS::Driver::Filesystem::FAT32
{
    constexpr auto SECTOR_SIZE = 512;
    constexpr auto MAX_PATH = 128;
    constexpr auto GPT_SIGNATURE = 0x5452415020494645ULL;

    struct MBRPartitionEntry
    {
        u8 Status;
        u8 CHSFirst[3];
        u8 Type;
        u8 CHSLast[3];
        u32 LBAStart;
        u32 TotalSectors;
    } PACKED;

    struct GPTEntry
    {
        u8 PartitionTypeGUID[16];
        u8 UniquePartitionGUID[16];
        u64 FirstLBA;
        u64 LastLBA;
        u64 Attributes;
        char16_t* PartitionName[36];
    } PACKED;

    struct GPTHeader
    {
        u64 Signature;
        u32 Revision;
        u32 HeaderSize;
        u32 HeaderCRC2;
        u32 Reserved;
        u64 CurrentLBA;
        u64 BackupLBA;
        u64 FirstAvailableLBA;
        u64 LastAvailableLBA;
        u8 DiskGUID[16];
        u64 PartitionEntryLBA;
        u32 NumPartitionEntries;
        u32 SizeOfPartitionEntry;
        u32 PartitionEntriesCRC32;
        u8 Padding[420];
    } PACKED;

    struct FAT32VBR
    {
        u8 JmpInstr[3];
        char OEMName[8];
        u16 BytesPerSector;
        u8 SectorsPerCluster;
        u16 ReservedSectorCount;
        u8 NumFATs;
        u16 RootEntryCount;
        u16 TotalSectors16;
        u8 MediaType;
        u16 FATSize16;
        u16 SectorsPerTrack;
        u16 NumHeads;
        u32 HiddenSectors;
        u32 TotalSectors32;
        u32 FATSize32;
        u16 ExtFlags;
        u16 FSVersion;
        u32 RootClustor;
        u16 FSInfo;
        u16 BackupBootSector;
        u8 Reserved[12];
        u8 DriveNumber;
        u8 Reserved1;
        u8 BootSignature;
        u32 VolumeID;
        char VolumeLabel[11];
        char FSType[8];
        u8 BootCode[420];
        u16 BootSectorSignature;
    } PACKED;

    Status readFile(const char* fileName, u8* outBuffer, size_t maxSize, size_t& outSize);
    Status getFileSize(const char* fileName, size_t& outSize);
    bool initFAT32();
}