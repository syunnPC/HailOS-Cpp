#include "acpi.hpp"
#include "basetype.hpp"
#include "io.hpp"
#include "cstring.hpp"
#include "common.hpp"
#include "time.hpp"

namespace HailOS::PowerManager::ACPI
{
    static bool sInitialized = false;
    RSDPtr* sRSDP = nullptr;

    constexpr auto ACPI_SIG_RSDP = "RSD PTR ";
    constexpr auto ACPI_SIG_XSDT = "XSDT";
    constexpr auto ACPI_SIG_RSDT = "RSDT";
    constexpr auto ACPI_SIG_FACP = "FACP";

    constexpr u16 SLP_TYPE_S5 = (0x5 << 10);
    constexpr u16 SLP_EN = (1 << 13);
    constexpr u16 SLP_CMD = SLP_TYPE_S5 | SLP_EN;

    template<typename T>
    static void writePhysMem(void* addr, T value)
    {
        if(addr == 0)
        {
            return;
        }

        volatile T* ptr = reinterpret_cast<volatile T*>(addr);
        *ptr = value;
    }

    static inline bool checkSigEq(const char* sig1, const char* sig2, size_t length)
    {
        for(size_t i=0; i<length; i++)
        {
            if(sig1[i] != sig2[i])
            {
                return false;
            }
        }

        return true;
    }

    static bool validateRSDPChecksum(RSDPtr* rsdp)
    {
        if(rsdp == nullptr)
        {
            return false;
        }

        if(rsdp->Revision < 2)
        {
            u8* p = reinterpret_cast<u8*>(rsdp);
            u8 sum = 0;
            for(size_t i=0; i<20; i++)
            {
                sum = static_cast<u8>(sum + p[i]);
            }

            return sum == 0;
        }
        else
        {
            u8* p = reinterpret_cast<u8*>(rsdp);
            u32 len = rsdp->Length;

            if(len < 20)
            {
                return false;
            }
            
            u8 sum = 0;
            for(u32 i=0; i<len; i++)
            {
                sum = static_cast<u8>(sum + p[i]);
            }

            return sum == 0;
        }
    }
    
    static SDTHeader* findTable(RSDPtr* rsdp, const char* sig)
    {
        if(rsdp == nullptr || sig == nullptr)
        {
            return nullptr;
        }

        if(rsdp->Revision >= 2)
        {
            SDTHeader* xsdt = reinterpret_cast<SDTHeader*>(rsdp->XSDTAddress);
            if(xsdt == nullptr)
            {
                return nullptr;
            }

            u32 entries = (xsdt->Length - sizeof(SDTHeader)) / sizeof(u64);
            u64 *entry = reinterpret_cast<u64*>(reinterpret_cast<addr_t>(xsdt) + sizeof(SDTHeader));
            for(u32 i=0; i<entries; i++)
            {
                SDTHeader* h = reinterpret_cast<SDTHeader*>(entry[i]);
                if(h != nullptr && checkSigEq(h->Signature, sig, StdLib::C::strlen(sig)))
                {
                    return h;
                }
            }
        }
        else
        {
            SDTHeader* rsdt = reinterpret_cast<SDTHeader*>(rsdp->RSDTAddress);
            if(rsdt == nullptr)
            {
                return nullptr;
            }

            u32 entries = (rsdt->Length - sizeof(SDTHeader)) / sizeof(u32);
            u32* entry = reinterpret_cast<u32*>(reinterpret_cast<addr_t>(rsdt + sizeof(SDTHeader)));
            for(u32 i=0; i<entries; i++)
            {
                SDTHeader* h = reinterpret_cast<SDTHeader*>(entry[i]);
                if(h != nullptr && checkSigEq(h->Signature, sig, StdLib::C::strlen(sig)))
                {
                    return h;
                }
            }
        }

        return nullptr;
    }

    static void writeGAS(GenericAddress* gas, u64 value)
    {
        if(gas == nullptr)
        {
            return;
        }

        if(gas->Address == 0)
        {
            return;
        }

        if(gas->AddressSpaceID == 0)
        {
            u16 port = static_cast<u16>(gas->Address);
            switch(gas->AccessSize)
            {
                case 1:
                    IO::outb(port, static_cast<u8>(value));
                    break;
                case 2:
                default:
                    IO::outw(port, static_cast<u16>(value));
                    break;
                case 3:
                case 4:
                    IO::outl(port, static_cast<u32>(value));
                    break;
            }
        }
        else if(gas->AddressSpaceID == 1)
        {
            void* p = reinterpret_cast<void*>(gas->Address);
            switch(gas->AccessSize)
            {
                case 1:
                    writePhysMem(p, static_cast<u8>(value));
                    break;
                case 2:
                default:
                    writePhysMem(p, static_cast<u16>(value));
                    break;
                case 3:
                case 4:
                    writePhysMem(p, static_cast<u32>(value));
                    break;
                case 5:
                    writePhysMem(p, static_cast<u64>(value));
                    break;
            }
        }
        else
        {
            return;
        }
    }

    __attribute__((optimize("O0"))) void shutdown(void)
    {
        if(sInitialized == false || sRSDP == nullptr)
        {
            PANIC(Status::STATUS_NOT_INITIALIZED, 0, 0, 0, 0);
        }

        SDTHeader* fadt_hdr = findTable(sRSDP, ACPI_SIG_FACP);
        if(fadt_hdr == nullptr)
        {
            PANIC(Status::STATUS_ACPI_ERROR, 0, 0, 0, 0);
        }

        FADT* fadt = reinterpret_cast<FADT*>(fadt_hdr);

        if(fadt->ACPIEnable!= 0 && fadt->SMICommandPort != 0)
        {
            IO::outb(fadt->SMICommandPort, fadt->ACPIEnable);
        }

        bool mmio = false;
        u64 pm1a = 0;
        size_t width;
        bool b_mmio = false;
        bool b_enable = false;
        u64 pm1b = 0;
        size_t b_width;

        if(fadt->X_PM1aCtrlBlock.Address != 0)
        {
            mmio = (fadt->X_PM1aCtrlBlock.AddressSpaceID == 0);
            pm1a = fadt->X_PM1aCtrlBlock.Address;
            width = (fadt->X_PM1aCtrlBlock.AccessSize != 0) ? (1 << (fadt->X_PM1aCtrlBlock.AccessSize - 1)) : fadt->PM1CtrlLength;
        }
        else if(fadt->PM1aCtrlBlock != 0)
        {
            mmio = false;
            pm1a = fadt->PM1aCtrlBlock;
            width = fadt->PM1CtrlLength;
        }
        else
        {
            PANIC(Status::STATUS_ACPI_ERROR, 0, 0, 0, 0);
        }

        if(fadt->X_PM1bCtrlBlock.Address != 0)
        {
            b_mmio = (fadt->X_PM1bCtrlBlock.AddressSpaceID == 0);
            pm1b = fadt->X_PM1bCtrlBlock.Address;
            b_width = (fadt->X_PM1bCtrlBlock.AccessSize != 0) ? (1 << (fadt->X_PM1bCtrlBlock.AccessSize - 1)) : fadt->PM1CtrlLength;
            b_enable = true;
        }
        else if (fadt->PM1bCtrlBlock != 0)
        {
            b_mmio = false;
            pm1b = fadt->PM1bCtrlBlock;
            b_width = fadt->PM1CtrlLength;
            b_enable = true;
        }

        if(!mmio)
        {
            IO::outw(static_cast<u16>(pm1a), SLP_CMD);
        }
        else
        {
            writePhysMem(reinterpret_cast<void*>(pm1a), SLP_CMD);
        }

        if(b_enable)
        {
            if(!b_mmio)
            {
                IO::outw(static_cast<u16>(pm1b), SLP_CMD);
            }
            else
            {
                writePhysMem(reinterpret_cast<void*>(pm1b), SLP_CMD);
            }
        }

        Utility::Time::Sleep(10000);

        PANIC(Status::STATUS_ACPI_ERROR, static_cast<u64>(mmio), pm1a, width, static_cast<u64>(b_enable));
    }

    bool initACPI(RSDPtr* ptr)
    {
        if(sInitialized && sRSDP != nullptr)
        {
            return true;
        }

        if(!validateRSDPChecksum(ptr))
        {
            return false;
        }

        sRSDP = ptr;
        sInitialized = true;
        return true;
    }
}