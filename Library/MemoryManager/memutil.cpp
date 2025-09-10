#include "basetype.hpp"
#include "memutil.hpp"

namespace HailOS::MemoryManager
{
    void* allocInitializedMemory(size_t size, u8 value)
    {
        void* ptr = alloc(size);
        if(ptr == nullptr)
        {
            return nullptr;
        }

        fill(ptr, size, value);
        return ptr;
    }

    void fill(void* ptr, size_t size, u8 value)
    {
        if(ptr == nullptr)
        {
            return;
        }

        u8* p = reinterpret_cast<u8*>(ptr);

        for(size_t i=0; i<size; i++)
        {
            p[i] = value;
        }
    }

    bool memeq(const void* mem1, const void* mem2, size_t size)
    {
        if(mem1 == nullptr || mem2 == nullptr)
        {
            return mem1 == mem2;
        }

        if(size == 0)
        {
            return true;
        }

        const u8* p1 = reinterpret_cast<const u8*>(mem1);
        const u8* p2 = reinterpret_cast<const u8*>(mem2);

        for(size_t i=0; i<size; i++)
        {
            if(p1[i] != p2[i])
            {
                return false;
            }
        }

        return true;
    }

    void memcopy(void* dest, const void* src, size_t size)
    {
        if(dest == nullptr || src == nullptr || size == 0)
        {
            return;
        }

        u8* d = reinterpret_cast<u8*>(dest);
        const u8* s = reinterpret_cast<const u8*>(src);

        for(size_t i=0; i<size; i++)
        {
            d[i] = s[i];
        }
    }
}