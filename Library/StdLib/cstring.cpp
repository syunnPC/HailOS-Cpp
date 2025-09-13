#include "basetype.hpp"
#include "memutil.hpp"

namespace HailOS::StdLib::C
{
    size_t strlen(const char *str)
    {
        if (str == nullptr)
        {
            return 0;
        }

        size_t i = 0;
        while (true)
        {
            if (str[i++] == '\0')
            {
                return i - 1;
            }
        }
    }

    int strncmp(const char* str1, const char* str2, size_t count)
    {
        if(count == 0)
        {
            return 0;
        }

        if(str1 == nullptr && str2 == nullptr)
        {
            return 0;
        }
        if(str1 == nullptr)
        {
            return -1;
        }
        if(str2 == nullptr)
        {
            return 1;
        }

        while(count > 0 && *str1 != '\0' && *str1 == *str2)
        {
            str1++;
            str2++;
            count--;
            if(*(str1 - 1) == '\0')
            {
                break;
            }
        }

        if(count == 0)
        {
            return 0;
        }

        return count;
    }

    int strcmp(const char* str1, const char* str2)
    {
        if(strlen(str1) != strlen(str2))
        {
            return -1;
        }

        for(size_t i=0; i<strlen(str1); i++)
        {
            if(str1[i] != str2[i])
            {
                return i;
            }
        }

        return 0;
    }

    void* memset(void* dest, u8 value, size_t size)
    {
        if(dest == nullptr)
        {
            return nullptr;
        }
        else if(size == 0)
        {
            return dest;
        }

        u8* p = reinterpret_cast<u8*>(dest);
        for(size_t i=0; i<size; i++)
        {
            p[i] = value;
        }

        return dest;
    }

    char* itos(i64 n)
    {
        char buf[21];
        int i=0;
        bool is_negative = false;
        char* result;
        u64 u_n;

        if(n == 0)
        {
            result = reinterpret_cast<char*>(MemoryManager::alloc(2));
            if(result == nullptr)
            {
                return nullptr;
            }

            result[0] = '0';
            result[1] = '\0';
            return result;
        }

        if(n < 0)
        {
            is_negative = true;
            if (n == (-9223372036854775807LL - 1LL))
            {
                const char *llong_min_abs_str = "9223372036854775808";
                size_t len = strlen(llong_min_abs_str);
                MemoryManager::memcopy(buf, llong_min_abs_str, len);
                i = len;
                u_n = 0;
            }
            else
            {
                u_n = static_cast<u64>(-n);
            }
        }
        else
        {
            u_n = static_cast<u64>(n);
        }

        if (u_n > 0)
        {
            while (u_n > 0)
            {
                buf[i++] = (u_n % 10) + '0';
                u_n /= 10;
            }
        }

        if(is_negative)
        {
            buf[i++] = '-';
        }

        result = reinterpret_cast<char*>(MemoryManager::alloc(i + 1));
        if(result == nullptr)
        {
            return nullptr;
        }

        for(int j = 0; j<i; j++)
        {
            result[j] = buf[i - 1 -j];
        }

        result[i] = '\0';

        return result;
    }

    char* utos(u64 n)
    {
        char buf[21];
        int i=0;
        char* result;

        if (n == 0)
        {
            result = reinterpret_cast<char *>(MemoryManager::alloc(2));
            if (result == nullptr)
            {
                return nullptr;
            }

            result[0] = '0';
            result[1] = '\0';
            return result;
        }

        while(n > 0)
        {
            buf[i++] = (n % 10) + '0';
            n /= 10;
        }

        result = reinterpret_cast<char*>(MemoryManager::alloc(i + 1));
        if(result == nullptr)
        {
            return nullptr;
        }

        for(int j=0; j<i; j++)
        {
            result[j] = buf[i-1-j];
        }
        result[i] = '\0';
        return result;
    }

    const char* utohexstr(u64 n)
    {
        static char buf[19];
        static const char* hex = "0123456789ABCDEF";

        buf[0] = '0';
        buf[1] = 'x';

        for(int i=0; i<16; i++)
        {
            int shift = (15 - i) * 4;
            u8 nib = (n >> shift) & 0xF;
            buf[2+i] = hex[nib];
        }

        buf[18] = '\0';
        return buf;
    }
}