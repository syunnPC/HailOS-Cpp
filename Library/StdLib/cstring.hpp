#pragma once

#include "basetype.hpp"

namespace HailOS::StdLib::C
{
    size_t strlen(const char* str);
    int strncmp(const char* str1, const char* str2, size_t count);
    int strcmp(const char* str1, const char* str2);
    void* memset(void* dest, u8 value, size_t size);
    char* utos(u64 n);
    char* itos(i64 n);
}
