#include "basetype.hpp"
#include "pci.hpp"
#include "xhci.hpp"
#include "memutil.hpp"
#include "status.hpp"
#include "console.hpp"
#include "cstring.hpp"

namespace HailOS::IO::USB::xHCI
{
    static PCI::PCILocation sControllerLoc;
    static MMIO sMMIO;
    static PCI::BARInfo sControllerBAR0;
    static Ring sCmdRing{};
    static Ring sEventRing{};
    static ERSTEntry* sERST = nullptr;
    static constexpr PCI::PCILocation NOT_FOUND = (PCI::PCILocation){0, 0, 0};
    static TRB* sEventDeq = nullptr;
    static u8 sEventCycle = 1;

    ///{0,0,0}に注意
    PCI::PCILocation getControllerLocation()
    {
        for(u16 bus = 0; bus < 256; bus++)
        {
            for(u8 dev=0; dev<32; dev++)
            {
                for(u8 func = 0; func < 8; func++)
                {
                    if(PCI::isExistingPCIDevice(bus, dev, func))
                    {
                        u32 classcode = IO::PCI::readConfig32(bus, dev, func, 0x08);
                        u8 progif = (classcode >> 8) & 0xFF;
                        u8 subclass = (classcode >> 16) & 0xFF;
                        u8 pci_class = (classcode >> 24) & 0xFF;

                        if (pci_class == PCI_CLASS_SERIAL_BUS && subclass == PCI_SUBCLASS_USB && progif == PCI_PROGIF_XHCI)
                        {
                            return {bus, dev, func};
                        }
                    }
                }
            }
        }

        return {0, 0, 0};
    }

    static inline u32 load32(volatile void* p)
    {
        return *reinterpret_cast<volatile u32*>(p);
    }

    static inline void store32(volatile void* p, u32 v)
    {
        *reinterpret_cast<volatile u32*>(p) = v;
    }

    static inline u64 load64(volatile void* p)
    {
        return *reinterpret_cast<volatile u64*>(p);
    }

    static void store64(volatile void* p, u64 v)
    {
        *reinterpret_cast<volatile u64*>(p) = v;
    }

    static inline CapRegs* cap()
    {
        return const_cast<CapRegs*>(reinterpret_cast<volatile CapRegs*>(sMMIO.Cap));
    }

    static inline OpRegs* op()
    {
        return const_cast<OpRegs*>(reinterpret_cast<volatile OpRegs*>(sMMIO.Op));
    }

    static inline PortRegs* port(u32 i)
    {
        return const_cast<PortRegs*>(reinterpret_cast<volatile PortRegs*>(sMMIO.Op + 0x400 + (i - 1) * 0x10));
    }

    inline void initRing(Ring& r, TRB* mem, u32 n)
    {
        r.Base = mem;
        r.Size = n;
        r.Enqueue = 0;
        r.Cycle = 1;

        TRB& link = r.Base[n-1];
        link.P0 = static_cast<u32>(reinterpret_cast<u64>(r.Base) & 0xFFFFFFFF);
        link.P1 = static_cast<u32>(reinterpret_cast<u64>(r.Base) >> 32);
        link.P2 = 0;
        link.P3 = (static_cast<u32>(TRBType::Link) << 10) | 1 | (r.Cycle? 1: 0);
    }

    inline TRB* pushTRB(Ring& r)
    {
        TRB* t = &r.Base[r.Enqueue ++];
        if(r.Enqueue == r.Size -1)
        {
            r.Enqueue = 0;
            r.Cycle ^= 1;
        }

        return t;
    }

    inline void fillTypeCycle(TRB& t, TRBType type, u8 cycle)
    {
        t.P3 = (static_cast<u32>(type) << 10) | (cycle ? 1 : 0);
    }

    static InterrupterRegs* intr0()
    {
        return const_cast<InterrupterRegs*>(reinterpret_cast<volatile InterrupterRegs*>(sMMIO.Rt + 0x20));
    }

    static void doorbell(u32 slot, u32 target = 0)
    {
        sMMIO.Db[slot] = target;
    }

    bool startController(void)
    {
        store32(&op()->USBCMD, load32(&op()->USBCMD) | (1 << 1));

        for (volatile int i = 0; i < 100000000; i++)
        {
            if((load32(&op()->USBSTS) & (1 << 11)) == 0)
            {
                break;
            }
        }

        u32 max_slots = cap()->HCSParams1 & 0xFF;
        store32(&op()->CONFIG, max_slots ? max_slots : 8);

        u64* dcbaa = reinterpret_cast<u64*>(MemoryManager::allocAligned((max_slots + 1) * sizeof(u64), 4096));
        MemoryManager::fill(dcbaa, (max_slots + 1)* sizeof(u64), 0);
        store64(&op()->DCBAAP, reinterpret_cast<u64>(dcbaa));

        //scratchpadは4096バイト境界のアラインが必要？
        u32 sp = (cap()->HCSParams2 >> 27) & 0x1F;
        if(sp)
        {
            u64* arr = reinterpret_cast<u64*>(MemoryManager::allocAligned(sp*sizeof(u64), 4096));
            for(u32 i=0; i<sp; i++)
            {
                void* buf = MemoryManager::allocAligned(4096, 4096);
                arr[i] = reinterpret_cast<u64>(buf);
            }
            dcbaa[0] = reinterpret_cast<u64>(arr);
        }

        auto cmd_mem = reinterpret_cast<TRB*>(MemoryManager::allocAligned(256*sizeof(TRB), 4096));
        MemoryManager::fill(cmd_mem, 256*sizeof(TRB), 0);
        initRing(sCmdRing, cmd_mem, 256);
        store64(&op()->CRCR, (reinterpret_cast<u64>(cmd_mem) & ~0x3FULL) | (sCmdRing.Cycle));

        auto evt_mem = reinterpret_cast<TRB*>(MemoryManager::allocAligned(256 * sizeof(TRB), 4096));
        MemoryManager::fill(evt_mem, 256*sizeof(TRB), 0);
        initRing(sEventRing, evt_mem, 256);
        sERST = reinterpret_cast<ERSTEntry*>(MemoryManager::allocAligned(sizeof(ERSTEntry), 4096));
        sERST->RingBase = reinterpret_cast<u64>(evt_mem);
        sERST->RingSize = 256;
        sERST->Reserved = 0;

        auto it = intr0();
        it->IMAN = 0;
        it->IMOD = 0x0000FFFF;
        it->ERSTSZ = 1;
        it->ERSTBA = reinterpret_cast<u64>(sERST);
        it->ERDP = reinterpret_cast<u64>(evt_mem);
        it->IMAN = (1 << 1); //IE=1

        store32(&op()->USBCMD, load32(&op()->USBCMD) | 1); //RS=1
        sEventDeq = evt_mem;
        sEventCycle = 1;
        return true;
    }

    bool portResetWait(u32 portIndex)
    {
        auto p=port(portIndex);
        u32 v = load32(&p->PORTSC);
        if(!(v & (1 << 0)))
        {
            return false; //未接続
        }

        //PR=1
        store32(&p->PORTSC, (v & ~0x1F) | (1 << 4));

        //PRクリア待ち
        for(volatile int i = 0; i < 100000000; i++)
        {
            v = load32(&p->PORTSC);
            if(!(v & (1 << 4)))
            {
                break;
            }
        }

        //TODO:速度や状態を確認？↓とりあえず簡易ダンプ

        u32 v2 = load32(&port(portIndex)->PORTSC);
        bool ccs = v2 & (1 << 0);
        bool ped = v2 & (1 << 1);
        u32 speed = (v2 >> 10) & 0xF;
        Console::puts("Port ");
        Console::puts(StdLib::C::utos(portIndex));
        Console::puts(" : CCS = ");
        Console::puts(StdLib::C::utos(ccs));
        Console::puts(", PED = ");
        Console::puts(StdLib::C::utos(ped));
        Console::puts(", SPEED = ");
        Console::puts(StdLib::C::utos(speed));
        Console::puts(", raw = ");
        Console::puts(StdLib::C::utohexstr(v2));
        Console::puts("\n");

        return true;
    }

    static void postEnableSlot()
    {
        TRB* t = pushTRB(sCmdRing);
        t->P0 = t->P1 = t->P2 = 0;
        fillTypeCycle(*t, TRBType::EnableSlot, sCmdRing.Cycle);
        sMMIO.Db[0] = 0;
    }

    bool mmioInfoInit(PCI::BARInfo info)
    {
        sMMIO.Cap = reinterpret_cast<volatile u8*>(info.Base);
        auto len = cap()->CAPLENGTH;
        sMMIO.Op = sMMIO.Cap + len;
        sMMIO.Db = reinterpret_cast<volatile u32*>(sMMIO.Cap + (cap()->DBOFF & ~0x3));
        sMMIO.Rt = sMMIO.Cap + (cap()->RTSOFF & ~0x1F);
        return true;
    }

    static inline u8 xhciEventType(const TRB& t)
    {
        return (t.P3 >> 10) & 0x3F;
    }

    static inline bool xhciEventCycle(const TRB& t)
    {
        return (t.P3 & 1) != 0;
    }

    static inline u8 xhciCompletionCode(const TRB& t)
    {
        return (t.P2 >> 24) & 0xFF;
    }

    static inline u64 xhciCmdTRBPointer(const TRB& t)
    {
        return static_cast<u64>(t.P0) | (static_cast<u64>(t.P1) << 32);
    }

    inline u8 xhciEventSlotID(const TRB& t)
    {
        return (t.P3 >> 24) & 0xFF;
    }

    static inline void xhciAdvanceERDP(TRB* newDeq)
    {
        InterrupterRegs* it = intr0();
        it->ERDP = (reinterpret_cast<u64>(newDeq) | (1ULL << 3));
        it->IMAN = it->IMAN | 1u;
    }

    bool pollEvent(TRB* outEv)
    {
        TRB ev = *sEventDeq;

        if(xhciEventCycle(ev) != sEventCycle)
        {
            return false;
        }

        if(xhciEventType(ev) == static_cast<u8>(TRBType::Link))
        {
            TRB* next = reinterpret_cast<TRB*>(xhciCmdTRBPointer(ev));
            sEventDeq = next;
            sEventCycle ^= 1;
            xhciAdvanceERDP(sEventDeq);
            return pollEvent(outEv);
        }

        *outEv = ev;

        sEventDeq++;
        if(sEventDeq == &sEventRing.Base[sEventRing.Size - 1])
        {
            sEventDeq = sEventRing.Base;
            sEventCycle ^= 1;
        }

        xhciAdvanceERDP(sEventDeq);

        u8 type = xhciEventType(ev);

        switch(type)
        {
            case static_cast<u8>(EventType::CommandCompletion):
                {
                    u8 ccode = xhciCompletionCode(ev);
                    u8 slot = xhciEventSlotID(ev);
                    Console::puts("xHCI: Command completion, cc = ");
                    Console::puts(StdLib::C::utos(ccode));
                    Console::puts(", slot = ");
                    Console::puts(StdLib::C::utos(slot));
                    Console::puts("\n");
                    break;
                }
            case static_cast<u8>(EventType::PortStatusChange):
                {
                    u32 portId = ev.P0 & 0xFF;
                    Console::puts("xHCI: Port status changed at port ");
                    Console::puts(StdLib::C::utos(portId));
                    Console::puts("\n");
                    dumpPortStatus();
                    break;
                }
            case static_cast<u8>(EventType::Transfer):
                //未実装だけど使う
                {
                    u8 ccode = xhciCompletionCode(ev);
                    u8 slot = xhciEventSlotID(ev);
                    Console::puts("xHCI: Transfer event, cc = ");
                    Console::puts(StdLib::C::utos(ccode));
                    Console::puts(", slot = ");
                    Console::puts(StdLib::C::utos(slot));
                    Console::puts("\n");
                    break;
                }
            default:
                Console::puts("xHCI: Unknown event.\n");
                break;
        }

        return true;
    }

    static bool waitEnableSlotComplete(u8* outSlotId)
    {
        TRB ev{};
        for(int spin=0; spin<1000000; spin++)
        {
            if(!pollEvent(&ev))
            {
                continue;
            }

            if(xhciEventType(ev) == static_cast<u8>(EventType::CommandCompletion))
            {
                u8 ccode = xhciCompletionCode(ev); //1が成功
                u8 slot = xhciEventSlotID(ev);
                Console::puts("xHCI: CCE for EnableSlot ccode = ");
                Console::puts(StdLib::C::utos(ccode));
                Console::puts(" , slot = ");
                Console::puts(StdLib::C::utos(slot));
                Console::puts("\n");

                if(ccode == 1 && slot != 0)
                {
                    if(outSlotId)
                    {
                        *outSlotId = slot;
                        return true;
                    }

                    return false;
                }
            }

            //TODO:PortStatusChangeやTransferの場合
        }

        Console::puts("xHCI: waitEnableSlotComplete timeout.\n");
        return false;
    }

    bool xhciControllerSelfTest(void)
    {
        postEnableSlot();

        u8 slot = 0;
        if(!waitEnableSlotComplete(&slot))
        {
            Console::puts("xHCI: EnableSlot failed.");
            return false;
        }

        Console::puts("xHCI:EnableSlot OK, slot = ");
        Console::puts(StdLib::C::utos(slot));
        Console::puts("\n");
        return true;
    }

    bool initxHCI()
    {
        sControllerLoc = getControllerLocation();
        if(MemoryManager::memeq(&sControllerLoc, &NOT_FOUND, sizeof(PCI::PCILocation)))
        {
            setLastStatus(Status::STATUS_XHCI_CONTROLLER_NOT_FOUND);
            return false;
        }

        /*
        Console::puts("xHCI Controller found at PCI bus ");
        Console::puts(StdLib::C::utos(sControllerLoc.Bus));
        Console::puts(" device ");
        Console::puts(StdLib::C::utos(sControllerLoc.Device));
        Console::puts(" function ");
        Console::puts(StdLib::C::utos(sControllerLoc.Function));
        Console::puts("\n");
        */

        if(!PCI::getBAR0(sControllerLoc, sControllerBAR0))
        {
            setLastStatus(Status::STATUS_XHCI_ERROR);
            return false;
        }

        PCI::enableBusMasterMMIO(sControllerLoc);

        if(!mmioInfoInit(sControllerBAR0))
        {
            setLastStatus(Status::STATUS_XHCI_ERROR);
            return false;
        }

        /*
        {
            u16 ver = cap()->HCIVERSION;
            u32 hcs1 = cap()->HCSParams1;
            u32 slots = (hcs1 & 0xFF);
            u32 ports = (hcs1 >> 24) & 0xFF;
            Console::puts("xHCI version:");
            Console::puts(StdLib::C::utos(ver));
            Console::puts(", slots = ");
            Console::puts(StdLib::C::utos(slots));
            Console::puts(", ports = ");
            Console::puts(StdLib::C::utos(ports));
            Console::puts("\n");
        }
        */

        if(!startController())
        {
            setLastStatus(Status::STATUS_XHCI_CONTROLLER_START_FAILED);
            return false;
        }

        return true;
    }

    void dumpPortStatus()
    {
        u32 ports = (cap()->HCSParams1 >> 24) & 0xFF;
        for(u32 i=1; i<=ports; i++)
        {
            u32 v = load32(&port(i)->PORTSC);
            bool ccs = v & (1 << 0);
            bool ped = v & (1 << 1);
            u32 speed = (v >> 10) & 0xF;
            Console::puts("Port ");
            Console::puts(StdLib::C::utos(i));
            Console::puts(" : CCS = ");
            Console::puts(StdLib::C::utos(ccs));
            Console::puts(", PED = ");
            Console::puts(StdLib::C::utos(ped));
            Console::puts(", SPEED = ");
            Console::puts(StdLib::C::utos(speed));
            Console::puts(", raw = ");
            Console::puts(StdLib::C::utohexstr(v));
            Console::puts("\n");
        }
    }

    void usbEventLoop(void)
    {
        TRB ev{};
        while(true)
        {
            if(pollEvent(&ev))
            {
                //ログ
            }
        }
    }
}