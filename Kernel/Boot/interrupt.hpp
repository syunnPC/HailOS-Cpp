#pragma once

#include "basetype.hpp"

namespace HailOS::Kernel::Boot
{
    struct InterruptFrame
    {
        u64 RIP;
        u64 CS;
        u64 RFlags;
        u64 RSP;
        u64 SS;
    };

    void handler(InterruptFrame* frame, u64 vec);

    __attribute__((naked)) void isrKeyboard(void);
    __attribute__((naked)) void isrMouse(void);
    __attribute__((naked)) void isrIRQ7(void);

    __attribute__((interrupt)) void handler0(HailOS::Kernel::Boot::InterruptFrame *frame); //#DE
    __attribute__((interrupt)) void handler1(HailOS::Kernel::Boot::InterruptFrame *frame); //#DB
    __attribute__((interrupt)) void handler2(HailOS::Kernel::Boot::InterruptFrame *frame); //#NMI
    __attribute__((interrupt)) void handler3(HailOS::Kernel::Boot::InterruptFrame *frame); //#BP
    __attribute__((interrupt)) void handler4(HailOS::Kernel::Boot::InterruptFrame *frame); //#OF
    __attribute__((interrupt)) void handler5(HailOS::Kernel::Boot::InterruptFrame *frame); //#BR
    __attribute__((interrupt)) void handler6(HailOS::Kernel::Boot::InterruptFrame *frame); //#UD
    __attribute__((interrupt)) void handler7(HailOS::Kernel::Boot::InterruptFrame *frame); //#NM
    __attribute__((interrupt)) void handler8(HailOS::Kernel::Boot::InterruptFrame *frame, u64 errorcode); //#DF
    __attribute__((interrupt)) void handler9(HailOS::Kernel::Boot::InterruptFrame *frame); //Coprocessor Segment Overrun, 未使用
    __attribute__((interrupt)) void handler10(HailOS::Kernel::Boot::InterruptFrame *frame, u64 errorcode); //#TS
    __attribute__((interrupt)) void handler11(HailOS::Kernel::Boot::InterruptFrame *frame, u64 errorcode); //#NP
    __attribute__((interrupt)) void handler12(HailOS::Kernel::Boot::InterruptFrame *frame, u64 errorcode); //#SS
    __attribute__((interrupt)) void handler13(HailOS::Kernel::Boot::InterruptFrame *frame, u64 errorcode); //#GP
    __attribute__((interrupt)) void handler14(HailOS::Kernel::Boot::InterruptFrame *frame, u64 errorcode); //#PF
    __attribute__((interrupt)) void handler15(HailOS::Kernel::Boot::InterruptFrame *frame); //予約
    __attribute__((interrupt)) void handler16(HailOS::Kernel::Boot::InterruptFrame *frame); //#MF
    __attribute__((interrupt)) void handler17(HailOS::Kernel::Boot::InterruptFrame *frame, u64 errorcode); //#AC
    __attribute__((interrupt)) void handler18(HailOS::Kernel::Boot::InterruptFrame *frame); //#MC
    __attribute__((interrupt)) void handler19(HailOS::Kernel::Boot::InterruptFrame *frame); //#XF
    __attribute__((interrupt)) void handler20(HailOS::Kernel::Boot::InterruptFrame *frame); //#VE
    __attribute__((interrupt)) void handler21(HailOS::Kernel::Boot::InterruptFrame *frame); //#CP
    __attribute__((interrupt)) void handler22(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler23(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler24(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler25(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler26(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler27(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler28(HailOS::Kernel::Boot::InterruptFrame *frame); //#HV
    __attribute__((interrupt)) void handler29(HailOS::Kernel::Boot::InterruptFrame *frame, u64 errorcode); //#VC
    __attribute__((interrupt)) void handler30(HailOS::Kernel::Boot::InterruptFrame *frame, u64 errorcode); //#SX
    __attribute__((interrupt)) void handler31(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler32(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler33(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler34(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler35(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler36(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler37(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler38(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler39(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler40(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler41(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler42(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler43(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler44(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler45(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler46(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler47(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler48(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler49(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler50(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler51(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler52(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler53(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler54(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler55(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler56(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler57(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler58(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler59(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler60(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler61(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler62(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler63(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler64(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler65(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler66(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler67(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler68(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler69(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler70(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler71(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler72(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler73(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler74(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler75(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler76(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler77(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler78(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler79(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler80(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler81(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler82(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler83(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler84(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler85(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler86(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler87(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler88(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler89(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler90(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler91(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler92(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler93(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler94(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler95(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler96(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler97(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler98(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler99(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler100(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler101(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler102(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler103(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler104(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler105(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler106(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler107(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler108(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler109(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler110(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler111(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler112(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler113(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler114(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler115(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler116(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler117(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler118(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler119(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler120(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler121(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler122(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler123(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler124(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler125(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler126(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler127(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler128(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler129(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler130(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler131(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler132(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler133(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler134(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler135(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler136(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler137(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler138(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler139(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler140(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler141(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler142(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler143(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler144(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler145(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler146(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler147(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler148(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler149(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler150(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler151(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler152(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler153(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler154(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler155(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler156(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler157(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler158(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler159(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler160(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler161(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler162(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler163(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler164(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler165(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler166(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler167(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler168(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler169(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler170(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler171(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler172(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler173(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler174(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler175(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler176(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler177(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler178(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler179(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler180(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler181(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler182(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler183(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler184(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler185(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler186(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler187(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler188(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler189(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler190(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler191(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler192(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler193(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler194(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler195(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler196(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler197(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler198(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler199(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler200(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler201(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler202(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler203(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler204(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler205(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler206(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler207(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler208(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler209(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler210(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler211(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler212(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler213(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler214(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler215(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler216(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler217(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler218(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler219(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler220(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler221(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler222(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler223(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler224(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler225(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler226(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler227(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler228(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler229(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler230(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler231(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler232(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler233(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler234(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler235(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler236(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler237(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler238(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler239(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler240(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler241(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler242(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler243(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler244(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler245(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler246(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler247(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler248(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler249(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler250(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler251(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler252(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler253(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler254(HailOS::Kernel::Boot::InterruptFrame *frame);
    __attribute__((interrupt)) void handler255(HailOS::Kernel::Boot::InterruptFrame *frame);
}