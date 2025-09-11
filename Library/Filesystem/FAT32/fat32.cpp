#include "fat32.hpp"
#include "basetype.hpp"
#include "hal_disk.hpp"
#include "cstring.hpp"
#include "memutil.hpp"
#include "status.hpp"

namespace HailOS::Driver::Filesystem::FAT32
{
    static FAT32VBR sVBR;
    static u64 sFAT32BaseLBA;
    static u32 sSectorsPerCluster;
    static u32 sFATStartLBA;
    static u32 sClusterStartLBA;
    static u32 sRootCluster;
    static u8 sFATSector[SECTOR_SIZE];

    static constexpr u8 ESP_GUID[16] =
    {
        0x28, 0x73, 0x2A, 0xC1, 0x1F, 0xF8, 0xD2, 0x11,
        0xBA, 0x4B, 0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B
    };

    static constexpr u8 MS_BASIC_DATA_GUID[16] =
    {
        0xA2, 0xA0, 0xD0, 0xEB, 0xE5, 0xB9, 0x33, 0x44,
        0x87, 0xC0, 0x68, 0xB6, 0xB7, 0x26, 0x99, 0xC7
    };

    static bool isGPTProtectiveMBR(u8* sector)
    {
        if(sector == nullptr)
        {
            return false;
        }

        MBRPartitionEntry* entry = reinterpret_cast<MBRPartitionEntry*>(sector + 446);
        return (entry->Type == 0xEE);
    }

    static bool readCluster(u32 clusterNum, u8* buffer)
    {
        if(buffer == nullptr)
        {
            return false;
        }

        u32 first = sClusterStartLBA + (clusterNum - 2) * sSectorsPerCluster;
        for(u32 i=0; i<sSectorsPerCluster; i++)
        {
            if(!HAL::Disk::readSector(first+i, buffer + (i * SECTOR_SIZE)))
            {
                return false;
            }
        }

        return true;
    }

    static u32 getNextCluster(u32 cluster)
    {
        if(cluster < 2)
        {
            return 0;
        }

        u32 fat_offset = cluster * 4;
        u32 fat_sector_num = fat_offset / SECTOR_SIZE;
        u32 offset_in_sector = fat_offset % SECTOR_SIZE;

        u32 fat_read_lba = sFATStartLBA + fat_sector_num;

        if(!HAL::Disk::readSector(fat_read_lba, sFATSector))
        {
            return 0;
        }

        u32 raw_next_cluster_val;
        if(offset_in_sector <= SECTOR_SIZE - sizeof(u32))
        {
            u8* p_fat_entry = sFATSector + offset_in_sector;
            raw_next_cluster_val = static_cast<u32>(p_fat_entry[0]) | (static_cast<u32>(p_fat_entry[1]) << 8) | (static_cast<u32>(p_fat_entry[2]) << 16) | (static_cast<u32>(p_fat_entry[3]) << 24);
        }
        else
        {
            return 0;
        }

        u32 final_next_cluster = raw_next_cluster_val & 0x0FFFFFFF;
        return final_next_cluster;
    }

    static bool compareFileName(const u8* fatName, const char* target)
    {
        char formatted[12] = {0};
        size_t len = StdLib::C::strlen(target);
        size_t dot = len;
        for(size_t i=0; i<len; i++)
        {
            if(target[i] == '.')
            {
                dot = i;
                break;
            }
        }

        MemoryManager::fill(formatted, ' ', 11);
        MemoryManager::memcopy(formatted, target, dot > 8 ? 8 : dot);
        if(dot < len - 1)
        {
            MemoryManager::memcopy(formatted + 8, &target[dot + 1], (len - dot -1 > 3) ? 3 : len - dot - 1);
        }
        for(int i=0; i<11; i++)
        {
            if((fatName[i] & 0xDF) != (formatted[i] & 0xDF))
            {
                return false;
            }
        }

        return true;
    }

    __attribute__((optimize("O0")) )Status readFile(const char* fileName, u8* outBuffer, size_t maxSize, size_t* outSize)
    {
        if(maxSize == 0 || outBuffer == nullptr || fileName == nullptr)
        {
            return Status::STATUS_INVALID_PARAMETER;
        }

        sSectorsPerCluster = sVBR.SectorsPerCluster;
        sFATStartLBA = sFAT32BaseLBA + sVBR.ReservedSectorCount;
        sClusterStartLBA = sFAT32BaseLBA + sVBR.ReservedSectorCount + (sVBR.NumFATs * sVBR.FATSize32);
        sRootCluster = sVBR.RootClustor;

        u32 current_cluster = sRootCluster;
        u32 cluster_buf_size = sSectorsPerCluster * SECTOR_SIZE;
        u8* cluster_buf = reinterpret_cast<u8*>(MemoryManager::allocInitializedMemory(cluster_buf_size, 0));

        if(cluster_buf == nullptr)
        {
            return Status::STATUS_MEMORY_ALLOCATION_FAILED;
        }

        while(current_cluster < 0x0FFFFFF8)
        {
            readCluster(current_cluster, cluster_buf);

            for(u32 i=0; i<cluster_buf_size; i+=32)
            {
                u8* entry = &cluster_buf[i];

                if(entry[i] == 0x00)
                {
                    MemoryManager::free(cluster_buf, cluster_buf_size);
                    return Status::STATUS_FAT32_FILESYSTEM;
                }
                if(entry[0] == 0xE5)
                {
                    continue;
                }
                if((entry[11] & 0x3F) == 0x0F)
                {
                    continue;
                }
                if(entry[11] & 0x08)
                {
                    continue;
                }

                if(compareFileName(entry, fileName))
                {
                    u32 file_cluster = static_cast<u32>(entry[26]) | (static_cast<u32>(entry[27]) << 8) | (static_cast<u32>(entry[20]) << 16) | (static_cast<u32>(entry[21]) << 24);
                    u32 filesize = *reinterpret_cast<u32*>(&entry[28]);
                    if(outSize != nullptr)
                    {
                        *outSize = filesize;
                    }
                    u32 bytes_read = 0;
                    while(file_cluster < 0x0FFFFFF8 && bytes_read < maxSize)
                    {
                        readCluster(file_cluster, cluster_buf);
                        u32 remain = filesize - bytes_read;
                        u32 copyable = cluster_buf_size;
                        u32 copy_size = (remain < copyable) ? remain : copyable;

                        if(bytes_read + copy_size > maxSize)
                        {
                            copy_size = maxSize - bytes_read;
                        }

                        if(copy_size == 0 && filesize > bytes_read)
                        {
                            break;
                        }

                        MemoryManager::memcopy(reinterpret_cast<void*>(outBuffer + bytes_read), cluster_buf, copy_size);
                        bytes_read += copy_size;

                        if(bytes_read >= filesize)
                        {
                            break;
                        }

                        file_cluster = getNextCluster(file_cluster);
                        if(file_cluster == 0)
                        {
                            MemoryManager::free(cluster_buf, cluster_buf_size);
                            return Status::STATUS_FAT32_FILESYSTEM;
                        }
                    }

                    MemoryManager::free(cluster_buf, cluster_buf_size);
                    return Status::STATUS_SUCCESS;
                }
            }

            current_cluster = getNextCluster(current_cluster);
            if(current_cluster == 0)
            {
                MemoryManager::free(cluster_buf, cluster_buf_size);
                return Status::STATUS_FAT32_FILESYSTEM;
            }
        }

        MemoryManager::free(cluster_buf, cluster_buf_size);
        return Status::STATUS_FILE_NOT_FOUND;
    }

    __attribute__((optimize("O0"))) Status getFileSize(const char *fileName, size_t &outSize)
    {
        if(fileName == nullptr)
        {
            return Status::STATUS_INVALID_PARAMETER;
        }

        sSectorsPerCluster = sVBR.SectorsPerCluster;
        sFATStartLBA = sFAT32BaseLBA + sVBR.ReservedSectorCount;
        sClusterStartLBA = sFAT32BaseLBA + sVBR.ReservedSectorCount + (sVBR.NumFATs * sVBR.FATSize32);
        sRootCluster = sVBR.RootClustor;

        u32 current_cluster = sRootCluster;
        u32 cluster_buf_size = sSectorsPerCluster * SECTOR_SIZE;
        u8 *cluster_buf = reinterpret_cast<u8 *>(MemoryManager::allocInitializedMemory(cluster_buf_size, 0));

        if (cluster_buf == nullptr)
        {
            return Status::STATUS_MEMORY_ALLOCATION_FAILED;
        }

        while (current_cluster < 0x0FFFFFF8)
        {
            readCluster(current_cluster, cluster_buf);

            for (u32 i = 0; i < cluster_buf_size; i += 32)
            {
                u8 *entry = &cluster_buf[i];

                if (entry[i] == 0x00)
                {
                    MemoryManager::free(cluster_buf, cluster_buf_size);
                    return Status::STATUS_FAT32_FILESYSTEM;
                }
                if (entry[0] == 0xE5)
                {
                    continue;
                }
                if ((entry[11] & 0x3F) == 0x0F)
                {
                    continue;
                }
                if (entry[11] & 0x08)
                {
                    continue;
                }

                if (compareFileName(entry, fileName))
                {
                    u32 file_cluster = static_cast<u32>(entry[26]) | (static_cast<u32>(entry[27]) << 8) | (static_cast<u32>(entry[20]) << 16) | (static_cast<u32>(entry[21]) << 24);
                    u32 filesize = *reinterpret_cast<u32 *>(&entry[28]);
                    outSize = filesize;
                    MemoryManager::free(cluster_buf, cluster_buf_size);
                    return Status::STATUS_SUCCESS;
                }
            }

            current_cluster = getNextCluster(current_cluster);
            if(getNextCluster(current_cluster) == 0)
            {
                MemoryManager::free(cluster_buf, cluster_buf_size);
                return Status::STATUS_FAT32_FILESYSTEM;
            }
        }

        MemoryManager::free(cluster_buf, cluster_buf_size);
        return Status::STATUS_FILE_NOT_FOUND;
    }

    static bool findFAT32Partition(u64& outLBA)
    {
        GPTHeader hdr;
        if(!HAL::Disk::readSector(1, reinterpret_cast<u8*>(&hdr)))
        {
            return false;
        }

        if(hdr.Signature != GPT_SIGNATURE)
        {
            return false;
        }

        u64 entry_lba = hdr.PartitionEntryLBA;
        u32 entry_count = hdr.NumPartitionEntries;
        u32 entry_size = hdr.SizeOfPartitionEntry;
        GPTEntry entry;
        u8 buf[SECTOR_SIZE];
        FAT32VBR vbr_temp;

        for(u32 i=0; i<entry_count; i++)
        {
            if(entry_size == 0)
            {
                return false;
            }

            u32 target_lba = entry_lba + (i * entry_size) / SECTOR_SIZE;
            u32 offset_in_sector = (i * entry_size) % SECTOR_SIZE;

            if(!HAL::Disk::readSector(target_lba, buf))
            {
                return false;
            }

            MemoryManager::memcopy(&entry, buf + offset_in_sector, entry_size);

            if(entry.FirstLBA > 0)
            {
                bool guid_match = false;
                if(MemoryManager::memeq(entry.PartitionTypeGUID, ESP_GUID, 16))
                {
                    guid_match = true;
                }
                else if(MemoryManager::memeq(entry.PartitionTypeGUID, MS_BASIC_DATA_GUID, 16))
                {
                    guid_match = true;
                }

                if(guid_match)
                {
                    if(!HAL::Disk::readSector(entry.FirstLBA, reinterpret_cast<u8*>(&vbr_temp)))
                    {
                        return false;
                    }
                    if(MemoryManager::memeq(vbr_temp.FSType, "FAT32   ", 8))
                    {
                        outLBA = entry.FirstLBA;
                        return true;
                    }
                }
            }
        }

        return false;
    }

    bool initFAT32(void)
    {
        if(!findFAT32Partition(sFAT32BaseLBA))
        {
            return false;
        }

        if(!HAL::Disk::readSector(sFAT32BaseLBA, reinterpret_cast<u8*>(&sVBR)))
        {
            return false;
        }

        if(sVBR.SectorsPerCluster != 512 || sVBR.SectorsPerCluster == 0 || (sVBR.SectorsPerCluster & (sVBR.SectorsPerCluster - 1)) != 0 || sVBR.NumFATs == 0 ||
        sVBR.ReservedSectorCount == 0 || sVBR.FATSize32 == 0 || sVBR.RootClustor < 2 || sVBR.BootSectorSignature != 0xAA55 || StdLib::C::strncmp(sVBR.FSType, "FAT32", 5) != 0)
        {
            return false;
        }

        sSectorsPerCluster = sVBR.SectorsPerCluster;
        sFATStartLBA = sVBR.ReservedSectorCount;
        sClusterStartLBA = sFAT32BaseLBA + sVBR.ReservedSectorCount + (sVBR.NumFATs * sVBR.FATSize32);
        sRootCluster = sVBR.RootClustor;
        return true;
    }
}