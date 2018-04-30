//USB peripheral - PIC32MM0256GPM064

#include <cstdint>
#include <cstring>
#include "Usb.hpp"
#include "Reg.hpp"
#include "Pins.hpp"

//registers - all registers use only first 8bits
enum {
    U1OTGIR = 0xBF808440,   //no SET, INV - all bits write-1-to-clear
    U1OTGIE = 0xBF808450,
    U1OTGSTAT = 0xBF808460, //no SET, INV, CLR
    U1OTGCON = 0xBF808470,
    U1PWRC = 0xBF808480,
    U1IR = 0xBF808600,      //no SET, INV
    U1IE = 0xBF808610,
    U1EIR = 0xBF808620,     //no SET, INV
    U1EIE = 0xBF808630,
    U1STAT = 0xBF808640,    //no SET, INV, CLR
        ENDPT_SHIFT = 4, ENDPT_MASK = 15,
        DIR = 3,
        PPBI = 2,
    U1CON = 0xBF808650,
    U1ADDR = 0xBF808660,
        LSEN = 7,
    U1BDTP1 = 0xBF808670,
    U1FRML = 0xBF808680,    //no SET, INV, CLR
    U1FRMH = 0xBF808690,    //no SET, INV, CLR
    U1TOK = 0xBF8086A0,
    U1SOF = 0xBF8086B0,
    U1BDTP2 = 0xBF8086C0,
    U1BDTP3 = 0xBF8086D0,
    U1CNFG1 = 0xBF8086E0,
    U1EP0 = 0xBF808700, U1EP_SPACING = 0x10
};

volatile Usb::bdt_t Usb::bdt_table[][2][2] = {0};
Pins vbus_pin(Usb::vbus_pin_n, Pins::INPD);

//=============================================================================
    void        Usb::bdt_init           ()
//=============================================================================
{
    memset((void*)&bdt_table, 0, sizeof(bdt_table));
    bdt_addr((uint32_t)bdt_table);
}

//=============================================================================
    bool        Usb::vbus_ison          ()
//=============================================================================
{
    return vbus_pin.ison();
}

//U1OTGIR/IE, U1IR/IE, U1EIR/IE
//=============================================================================
    uint32_t    Usb::flags              ()
//=============================================================================
{
    return Reg::val8(U1OTGIR)<<16 bitor
           Reg::val8(U1EIR)<<8 bitor
           Reg::val8(U1IR);
}

//=============================================================================
    bool        Usb::flag               (FLAGS e)
//=============================================================================
{
    return flags() bitand e;
}

//=============================================================================
    void        Usb::flags_clr          (uint32_t v)
//=============================================================================
{
    Reg::val(U1OTGIR, (uint8_t)(v>>16));
    Reg::val(U1EIR, (uint8_t)(v>>8));
    Reg::val(U1IR, (uint8_t)v);
}

//=============================================================================
    uint32_t    Usb::irqs               ()
//=============================================================================
{
    return Reg::val8(U1OTGIE)<<16 bitor
           Reg::val8(U1EIE)<<8 bitor
           Reg::val8(U1IE);
}

//=============================================================================
    bool        Usb::irq                (FLAGS e)
//=============================================================================
{
    return irqs() bitand e;
}

//=============================================================================
    void        Usb::irqs               (uint32_t v)
//=============================================================================
{
    Reg::val(U1OTGIE, (uint8_t)(v>>16));
    Reg::val(U1EIE, (uint8_t)(v>>8));
    Reg::val(U1IE, (uint8_t)v);
}

//=============================================================================
    void        Usb::irq                (FLAGS e, bool tf)
//=============================================================================
{
    if(e > (1<<15)) Reg::setbit(U1OTGIE, e>>16, tf);
    else if(e > (1<<7))Reg::setbit(U1EIE, e>>8, tf);
    else Reg::setbit(U1IE, e, tf);
}


//U1OTGSTAT
//=============================================================================
    uint32_t    Usb::otg_stat           ()
//=============================================================================
{
    return Reg::val8(U1OTGSTAT)<<16;
}

//=============================================================================
    bool        Usb::otg_stat           (FLAGS e)
//=============================================================================
{
    return Reg::anybit(U1OTGSTAT, e>>16);
}

//U1OTGCON
//=============================================================================
    void        Usb::otg                (OTG e, bool tf)
//=============================================================================
{
    Reg::setbit(U1OTGCON, e, tf);
}

//=============================================================================
    void        Usb::otg                (uint8_t v)
//=============================================================================
{
    Reg::val(U1OTGCON, v);
}

//U1PWRC
//=============================================================================
    bool        Usb::power              (POWER e)
//=============================================================================
{
    return Reg::anybit(U1PWRC, e);
}

//=============================================================================
    void        Usb::power              (POWER e, bool tf)
//=============================================================================
{
    Reg::setbit(U1PWRC, e, tf);
    if(e == USBPWR and tf) bdt_init();
}



//U1STAT
//=============================================================================
    uint8_t     Usb::stat               ()
//=============================================================================
{
    return Reg::val8(U1STAT)>>2;
}

//U1CON
//=============================================================================
    bool        Usb::control            (CONTROL e)
//=============================================================================
{
    return Reg::anybit(U1CON, e);
}

//=============================================================================
    void        Usb::control            (CONTROL e, bool tf)
//=============================================================================
{
    Reg::setbit(U1CON, e, tf);
}

//=============================================================================
    void        Usb::control            (uint8_t v)
//=============================================================================
{
    Reg::val(U1CON, v);
}

//U1ADDR
//=============================================================================
    void        Usb::low_speed          (bool tf)
//=============================================================================
{
    Reg::setbit(U1ADDR, 1<<LSEN, tf);
}

//=============================================================================
    uint8_t     Usb::dev_addr           ()
//=============================================================================
{
    return Reg::val8(U1ADDR) bitand 127;
}

//=============================================================================
    void        Usb::dev_addr           (uint8_t v)
//=============================================================================
{
    Reg::clrbit(U1ADDR, 127);
    Reg::setbit(U1ADDR, v bitand 127);
}

//U1FRML,H
//=============================================================================
    uint16_t    Usb::frame              ()
//=============================================================================
{
    return (Reg::val8(U1FRMH)<<8) bitor Reg::val8(U1FRML);
}

//U1TOK
//=============================================================================
    void        Usb::tok_pid            (TOKPID e)
//=============================================================================
{
    Reg::clrbit(U1TOK, 15<<4);
    Reg::setbit(U1TOK, e);
}

//=============================================================================
    void        Usb::tok_ep             (uint8_t v)
//=============================================================================
{
    Reg::clrbit(U1TOK, 15);
    Reg::setbit(U1TOK, v bitand 15);
}

//U1SOF
//=============================================================================
    void        Usb::sof_cnt            (SOFVALS e)
//=============================================================================
{
    Reg::val(U1SOF, e);
}

//U1BDTP1,2,3
//=============================================================================
    void        Usb::bdt_addr           (uint32_t v)
//=============================================================================
{
    v = Reg::k2phys(v); //physical address
    Reg::val(U1BDTP1, (uint8_t)(v>>8)); //512byte aligned (bit0 unused)
    Reg::val(U1BDTP2, (uint8_t)(v>>16));
    Reg::val(U1BDTP3, (uint8_t)(v>>24));
}

//=============================================================================
    volatile Usb::bdt_t*    Usb::bdt_addr           (uint8_t n, bool trx, bool eveodd)
//=============================================================================
{
    if(n > max_endpoint) return 0;          //invalid endpoint
    uint32_t v = Reg::p2kseg0(              //check if bdt table address set
        Reg::val8(U1BDTP3)<<24 bitor
        Reg::val8(U1BDTP2)<<16 bitor
        Reg::val8(U1BDTP1)<<8 );
    if(v != (uint32_t)bdt_table) return 0;  //something wrong
    return &bdt_table[n][trx][eveodd];
}

//U1CNFG1
//=============================================================================
    void        Usb::config             (CONFIG e, bool tf)
//=============================================================================
{
    Reg::setbit(U1CNFG1, e, tf);
}

//=============================================================================
    bool        Usb::config             (CONFIG e)
//=============================================================================
{
    return Reg::anybit(U1CNFG1, e);
}

//=============================================================================
    void        Usb::config             (uint8_t v)
//=============================================================================
{
    Reg::val(U1CNFG1, v);
}

//U1EP0-15
//=============================================================================
    void        Usb::epcontrol          (uint8_t n, EPCTRL e, bool tf)
//=============================================================================
{
    n and_eq 15;
    Reg::setbit(U1EP0+(n * U1EP_SPACING), e, tf);
}

//=============================================================================
    bool        Usb::epcontrol          (uint8_t n, EPCTRL e)
//=============================================================================
{
    n and_eq 15;
    return Reg::anybit(U1EP0+(n * U1EP_SPACING), e);
}

//=============================================================================
    void        Usb::epcontrol          (uint8_t n, uint8_t v)
//=============================================================================
{
    n and_eq 15;
    Reg::val(U1EP0+(n * U1EP_SPACING), v);
}

//=============================================================================
    uint8_t     Usb::epcontrol          (uint8_t n)
//=============================================================================
{
    n and_eq 15;
    return Reg::val8(U1EP0+(n * U1EP_SPACING));
}

//regs to reset state
//=============================================================================
    void        Usb::reset              ()
//=============================================================================
{
    irqs(0);
    Reg::val(U1CON, 0);     //usben
    Reg::val(U1PWRC, 0);    //usbpwr
    while(power(USBBUSY));
    flags_clr(0xFFFFFF);
    Reg::val(U1ADDR, 0);
    Reg::val(U1TOK, 0);
    Reg::val(U1SOF, 0);
    Reg::val(U1OTGCON, 0);
    Reg::val(U1CNFG1, 1);
    for(auto i = 0; i < 16; i++) epcontrol(i, 0);
    Reg::val(U1BDTP1, 0);
    Reg::val(U1BDTP2, 0);
    Reg::val(U1BDTP3, 0);
}
