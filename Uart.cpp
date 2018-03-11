#include "Uart.hpp"
#include "Reg.hpp"
#include "Osc.hpp"
#include "Pins.hpp"

enum {
    UARTX_SPACING = 0x40,  //spacing in words
    U1MODE = 0xBF801800,
        SLPEN = 23,
        ACTIVE = 22,
        CLKSEL_SHIFT = 17, CLKSEL_MASK = 3,
        OVFDIS = 16,
        ON = 15,
        SIDL = 13,
        IREN = 12,
        RTSMD = 11,
        WAKE = 7,
        LPBACK = 6,
        ABAUD = 5,
        RXINV = 4,
        BRGH = 3,
        MODE_SHIFT = 0, MODE_MASK = 7,
    UXSTA = 4, //offset from UXMODE in words
        MASK_SHIFT = 24, MASK_MASK = 255, //is byte3
        ADDR_SHIFT = 16, ADDR_MASK = 255, //is byte2
        UTXISEL_SHIFT = 14, UTXISEL_MASK = 3,
        UTXINV = 13,
        URXEN = 12,
        UTXBRK = 11,
        UTXEN = 10,
        UTXBF = 9,
        TRMT = 8,
        URXISEL_SHIFT = 6, URXISEL_MASK = 3,
        ADDEN = 5,
        RIDLE = 4,
        PERR = 3,
        FERR = 2,
        OERR = 1,
        URXDA = 0,
    UXTXREG = 8,
    UXRXREG = 12,
    UXBRG = 16
};

//Uart

//setup pins manually
//=============================================================================
    Uart::Uart      (UARTX e)
//=============================================================================
    : m_uartx_base((vu32ptr)U1MODE + ((e bitand 3) * UARTX_SPACING)),
      m_uartx_tx(*((vu32ptr)U1MODE + ((e bitand 3) * UARTX_SPACING) + UXTXREG)),
      m_uartx_rx(*((vu32ptr)U1MODE + ((e bitand 3) * UARTX_SPACING) + UXRXREG)),
      m_uartx_baud(0)
{}

//specify pins
//=============================================================================
    Uart::Uart      (UARTX e, Pins::RPN tx, Pins::RPN rx, uint32_t baud)
//=============================================================================
    : Uart(e)
{
    //UART1 is fixed pins, no pps
    if(e == Uart::UART2){
        Pins r(rx); r.pps_in(Pins::U2RX);
        Pins t(tx); t.pps_out(Pins::U2TX);
    }
    else if(e == Uart::UART3){
        Pins r(rx); r.pps_in(Pins::U3RX);
        Pins t(tx); t.pps_out(Pins::U3TX);
    }
    m_uartx_baud = baud;
    baud_set();
    rx_on(true);
    tx_on(true);
}

//tx or rx only
//=============================================================================
    Uart::Uart      (UARTX e, Pins::RPN trx, uint32_t baud)
//=============================================================================
    : Uart(e)
{
    //UART1 is fixed pins, no pps
    if(e == Uart::UART2){
        if(e bitand (1<<2)){
            Pins t(trx); t.pps_out(Pins::U2TX);
            tx_on(true);
        } else {
            Pins r(trx); r.pps_in(Pins::U2RX);
            rx_on(true);
        }
    }
    else if(e == Uart::UART3){
        if(e bitand (1<<2)){
            Pins t(trx); t.pps_out(Pins::U3TX);
            tx_on(true);
        } else {
            Pins r(trx); r.pps_in(Pins::U3RX);
            rx_on(true);
        }
    }
    m_uartx_baud = baud;
    baud_set();
}

//uxtxreg
//=============================================================================
    void            Uart::write     (uint16_t v)
//=============================================================================
{
    m_uartx_tx = v;
}

//uxrxreg
//=============================================================================
    uint16_t        Uart::read      ()
//=============================================================================
{
    return m_uartx_rx;
}


//uxmode
//=============================================================================
    void        Uart::stop_sleep        (bool tf)
//=============================================================================
{
    Reg::setbit(m_uartx_base, 1<<SLPEN, not tf);
}

//=============================================================================
    bool        Uart::active            ()
//=============================================================================
{
    return Reg::anybit(m_uartx_base, 1<<ACTIVE);
}

//=============================================================================
    void        Uart::clk_sel           (CLKSEL e)
//=============================================================================
{
    Reg::clrbit(m_uartx_base, CLKSEL_MASK<<CLKSEL_SHIFT);
    Reg::setbit(m_uartx_base, e<<CLKSEL_SHIFT);
    baud_set();
}

//=============================================================================
    void        Uart::oflow_stop        (bool tf)
//=============================================================================
{
    Reg::setbit(m_uartx_base, 1<<OVFDIS, not tf);
}

//=============================================================================
    void        Uart::on                (bool tf)
//=============================================================================
{
    baud_set(); //in case not set
    Reg::setbit(m_uartx_base, 1<<ON, tf);
}

//=============================================================================
    void        Uart::stop_idle         (bool tf)
//=============================================================================
{
    Reg::setbit(m_uartx_base, 1<<SIDL, tf);
}

//=============================================================================
    void        Uart::irda              (bool tf)
//=============================================================================
{
    Reg::setbit(m_uartx_base, 1<<IREN, tf);
}

//=============================================================================
    void        Uart::rts_mode          (RTSMODE e)
//=============================================================================
{
    Reg::setbit(m_uartx_base, 1<<RTSMD, e);
}

//=============================================================================
    void        Uart::wake              (bool tf)
//=============================================================================
{
    Reg::setbit(m_uartx_base, 1<<WAKE, tf);
}

//=============================================================================
    void        Uart::loopback          (bool tf)
//=============================================================================
{
    Reg::setbit(m_uartx_base, 1<<LPBACK, tf);
}

//=============================================================================
    void        Uart::autobaud          (bool tf)
//=============================================================================
{
    Reg::setbit(m_uartx_base, 1<<ABAUD, tf);
}

//=============================================================================
    void        Uart::rx_pol            (RXPOL e)
//=============================================================================
{
    Reg::setbit(m_uartx_base, 1<<RXINV, e);
}

//=============================================================================
    void        Uart::hispeed           (bool tf)
//=============================================================================
{
    Reg::setbit(m_uartx_base, 1<<BRGH, tf);
}

//=============================================================================
    void        Uart::mode              (MODESEL e)
//=============================================================================
{
    Reg::clrbit(m_uartx_base, MODE_MASK<<MODE_SHIFT);
    Reg::setbit(m_uartx_base, e<<MODE_SHIFT);
}

//=============================================================================
//uxsta
    void        Uart::rx_mask           (uint8_t v)
//=============================================================================
{
    Reg::val((vu8ptr)m_uartx_base + (UXSTA * 4) + 3, v);
}

//=============================================================================
    void        Uart::rx_addr           (uint8_t v)
//=============================================================================
{
    Reg::val((vu8ptr)m_uartx_base + (UXSTA * 4) + 2, v);
}

//=============================================================================
    void        Uart::tx_irq            (UTXISEL e)
//=============================================================================
{
    Reg::clrbit(m_uartx_base + UXSTA, UTXISEL_MASK<<UTXISEL_SHIFT);
    Reg::setbit(m_uartx_base + UXSTA, e<<UTXISEL_SHIFT);
}

//=============================================================================
    void        Uart::tx_pol            (RXPOL e)
//=============================================================================
{
    bool b = Reg::anybit(m_uartx_base, 1<<IREN) ? not e : e;
    Reg::setbit(m_uartx_base + UXSTA, 1<<UTXINV, b);
}

//=============================================================================
    void        Uart::rx_on             (bool tf)
//=============================================================================
{
    Reg::setbit(m_uartx_base + UXSTA, 1<<URXEN, tf);
}

//=============================================================================
    void        Uart::tx_break          ()
//=============================================================================
{
    Reg::setbit(m_uartx_base + UXSTA, 1<<UTXBRK);
}

//=============================================================================
    void        Uart::tx_on             (bool tf)
//=============================================================================
{
    Reg::setbit(m_uartx_base + UXSTA, 1<<UTXEN, tf);
}

//=============================================================================
    bool        Uart::tx_full           ()
//=============================================================================
{
    return Reg::anybit(m_uartx_base + UXSTA, 1<<UTXBF);
}

//=============================================================================
    bool        Uart::tx_done           ()
//=============================================================================
{
    return Reg::anybit(m_uartx_base + UXSTA, 1<<TRMT);
}

//=============================================================================
    void        Uart::rx_irq            (URXISEL e)
//=============================================================================
{
    Reg::clrbit(m_uartx_base + UXSTA, URXISEL_MASK<<URXISEL_SHIFT);
    Reg::setbit(m_uartx_base + UXSTA, e<<URXISEL_SHIFT);
}

//=============================================================================
    void        Uart::rx_addren         (bool tf)
//=============================================================================
{
    Reg::setbit(m_uartx_base + UXSTA, 1<<ADDEN, tf);
}

//=============================================================================
    bool        Uart::rx_busy           ()
//=============================================================================
{
    return not Reg::anybit(m_uartx_base + UXSTA, 1<<RIDLE);
}

//=============================================================================
    bool        Uart::rx_perr           ()
//=============================================================================
{
    return Reg::anybit(m_uartx_base + UXSTA, 1<<PERR);
}

//=============================================================================
    bool        Uart::rx_ferr           ()
//=============================================================================
{
    return Reg::anybit(m_uartx_base + UXSTA, 1<<FERR);
}

//=============================================================================
    bool        Uart::rx_oerr           ()
//=============================================================================
{
    return Reg::anybit(m_uartx_base + UXSTA, 1<<OERR);
}

//=============================================================================
    bool        Uart::rx_empty          ()
//=============================================================================
{
    return not Reg::anybit(m_uartx_base + UXSTA, 1<<URXDA);
}

//uxbrg
//=============================================================================
    void        Uart::baud_set          (uint32_t v)
//=============================================================================
{
    m_uartx_baud = v;
    uint32_t bclk = baud_clk();
    v = (bclk / 16 / (m_uartx_baud / 100));
    if( ((v / 100 * 100) / (v % 100)) < 50 ){ //<50 = >2% error,
        v *= 4; //so switch to hispeed
        hispeed(true);
    } else { hispeed(false); }
    Reg::val(m_uartx_base + UXBRG, (v/100)-1);
}
// 115200 8MHz
//8000000 / 16 / 1152 = 434,
// (434 / 100 * 100) / (434 % 100) = 11 = 8.5%error -> need hispeed
//8000000 / 4 / 1152 = 1736,
// (1736 / 100 * 100) / (1736 % 100) = 47 = 2.1%error -> ok (best we can get)
// 115200 24MHz
//24000000 / 16 / 1152 = 1302,
// (1302 / 100 * 100) / (1302 % 100) = 650 = 0.15%error -> ok
// 230400 24MHz
//24000000 / 16 / 2304 = 651,
// (651 / 100 * 100) / (651 % 100) = 11 = 8.5%error -> need hispeed
//24000000 / 4 / 2304 = 2604,
// (2604 / 100 * 100) / (2604 % 100) = 650 = 0.15%error -> ok

//called by clk_sel(), on()
//=============================================================================
    void        Uart::baud_set          ()
//=============================================================================
{
    //if baud not set, set it to 115200
    baud_set(m_uartx_baud ? m_uartx_baud : 115200);
}

//=============================================================================
    uint32_t    Uart::baud_clk          ()
//=============================================================================
{
    CLKSEL e = (CLKSEL)((Reg::val(m_uartx_base)>>17) bitand CLKSEL_MASK);
    if(e == REFO1) return Osc::refo_freq();
    else if(e == FRC) return Osc::frcclk();
    return Osc::sysclk(); //pb/sys are the same
}


//misc
//=============================================================================
    void        Uart::putchar           (const char c)
//=============================================================================
{
    while(tx_full());
    write(c);
}

//=============================================================================
    void        Uart::puts              (const char* s)
//=============================================================================
{
    while(s && *s) putchar(*s++);
}

//=============================================================================
    int         Uart::getchar           ()
//=============================================================================
{
    return rx_empty() ? -1 : read();
}
