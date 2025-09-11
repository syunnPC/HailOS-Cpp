#include "fileio.hpp"
#include "basetype.hpp"
#include "fat32.hpp"
#include "status.hpp"
#include "memutil.hpp"

namespace HailOS::API::FileIO
{
    Status openFile(const char* fileName, FileObject& result)
    {
        Status status = Driver::Filesystem::FAT32::getFileSize(fileName, result.Size);
        if(status != Status::STATUS_SUCCESS)
        {
            return status;
        }

        result.Buffer = reinterpret_cast<u8*>(MemoryManager::alloc(result.Size));
        if(result.Buffer == nullptr)
        {
            return Status::STATUS_MEMORY_ALLOCATION_FAILED;
        }

        status = Driver::Filesystem::FAT32::readFile(fileName, result.Buffer, result.Size, nullptr);
        if(status != Status::STATUS_SUCCESS)
        {
            MemoryManager::free(result.Buffer, result.Size);
            return status;
        }

        return Status::STATUS_SUCCESS;
    }

    Status closeFile(FileObject& object)
    {
        MemoryManager::free(object.Buffer, object.Size);
        object.Size = 0;
        object.Buffer = nullptr;
        return Status::STATUS_SUCCESS;
    }

    bool isExistingFile(const char* fileName)
    {
        u8 buf;
        Status status = Driver::Filesystem::FAT32::readFile(fileName, &buf, 1, nullptr);
        if(status != Status::STATUS_SUCCESS)
        {
            return false;
        }
        return true;
    }
}