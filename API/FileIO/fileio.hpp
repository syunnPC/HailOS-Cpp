#pragma once

#include "status.hpp"
#include "basetype.hpp"

namespace HailOS::API::FileIO
{
    constexpr auto FILENAME_MAX = 12;

    enum class FileOpenMode
    {
        ReadOnly,
        ReadWrite,
        WriteOnly,
    };

    struct FileObject
    {
        u8* Buffer;
        size_t Size;
        FileOpenMode Mode;
    };

    Status openFile(const char* fileName, FileObject& object);
    Status closeFile(FileObject& object);
    bool isExistingFile(const char* fileName);
}