#include "I2c.hpp"
#include "Osc.hpp"
#include "Reg.hpp"

enum :uint32_t {
    I2CX_SPACING = 0x40, //spacing in words
    I2C1CON = 0xBF801500,
        PCIE = 22,
        SCIE = 21,
        BOEN = 20,
        SDAHT = 19,
        SBCDE = 18,
        ON = 15,
        SIDL = 13,
        SCLREL = 12,
        STRICT = 11,
        A10M = 10,
        DISSLW = 9,
        SMEN = 8,
        GCEN = 7,
        STREN = 6,
        ACKDT = 5,
        ACKEN = 4,
        RCEN = 3,
        PEN = 2,
        RSEN = 1,
        SEN = 0,
    I2CXSTAT = 4, //offset from I2xCON in words
    I2CXADDR = 8,
    I2CXMSK = 12,
    I2CXBRG = 16,
    I2CXTRN = 20,
    I2CXRCV = 24
};

//=============================================================================
                I2c::I2c                (I2CX e)
//=============================================================================
    : m_i2cx_con((volatile uint32_t*)I2C1CON + (e * I2CX_SPACING)),
      m_speed(KHZ100)
{
}

//I2C1CON
//=============================================================================
    void        I2c::irq_stop           (bool tf)
//=============================================================================
{
    Reg::setbit(m_i2cx_con, 1<<PCIE, tf);
}

//=============================================================================
    void        I2c::irq_start          (bool tf)
//=============================================================================
{
    Reg::setbit(m_i2cx_con, 1<<SCIE, tf);
}

//=============================================================================
    void        I2c::overwrite          (bool tf)
//=============================================================================
{
    Reg::setbit(m_i2cx_con, 1<<BOEN, tf);
}

//=============================================================================
    void        I2c::hold_time          (HOLDTIM e)
//=============================================================================
{
    Reg::setbit(m_i2cx_con, 1<<SDAHT, e);
}

//=============================================================================
    void        I2c::irq_collision      (bool tf)
//=============================================================================
{
    Reg::setbit(m_i2cx_con, 1<<SBCDE, tf);
}

//=============================================================================
    void        I2c::on                 (bool tf)
//=============================================================================
{
    //always set brg in case clock changed
    //or speed not changed since init
    speed(m_speed);
    Reg::setbit(m_i2cx_con, 1<<ON, tf);
}

//=============================================================================
    void        I2c::stop_idle          (bool tf)
//=============================================================================
{
    Reg::setbit(m_i2cx_con, 1<<SIDL, tf);
}

//=============================================================================
    void        I2c::clk_release        (bool tf)
//=============================================================================
{
    Reg::setbit(m_i2cx_con, 1<<SCLREL, tf);
}

//=============================================================================
    void        I2c::strict             (bool tf)
//=============================================================================
{
    Reg::setbit(m_i2cx_con, 1<<STRICT, tf);
}

//=============================================================================
    void        I2c::addr_10bit         (bool tf)
//=============================================================================
{
    Reg::setbit(m_i2cx_con, 1<<A10M, tf);
}

//=============================================================================
    void        I2c::slew_rate          (bool tf)
//=============================================================================
{
    Reg::setbit(m_i2cx_con, 1<<DISSLW, not tf);
}

//=============================================================================
    void        I2c::smb_levels         (bool tf)
//=============================================================================
{
    Reg::setbit(m_i2cx_con, 1<<SMEN, tf);
}

//=============================================================================
    void        I2c::irq_gencall        (bool tf)
//=============================================================================
{
    Reg::setbit(m_i2cx_con, 1<<GCEN, tf);
}

//=============================================================================
    void        I2c::clk_stretch        (bool tf)
//=============================================================================
{
    Reg::setbit(m_i2cx_con, 1<<STREN, tf);
}

//=============================================================================
    void        I2c::ack                (bool tf)
//=============================================================================
{
    Reg::setbit(m_i2cx_con, 1<<ACKDT, not tf); //0=ACK,1=NACK
    Reg::setbit(m_i2cx_con, 1<<ACKEN);
}

//=============================================================================
    void        I2c::rx                 (bool tf)
//=============================================================================
{
    Reg::setbit(m_i2cx_con, 1<<RCEN, tf);
}

//=============================================================================
    void        I2c::stop               (bool tf)
//=============================================================================
{
    Reg::setbit(m_i2cx_con, 1<<PEN, tf);
}

//=============================================================================
    void        I2c::repstart           (bool tf)
//=============================================================================
{
    Reg::setbit(m_i2cx_con, 1<<RSEN, tf);
}

//=============================================================================
    void        I2c::start              (bool tf)
//=============================================================================
{
    Reg::setbit(m_i2cx_con, 1<<SEN, tf);
}

//I2CXSTAT
//=============================================================================
    bool        I2c::stat               (STAT e)
//=============================================================================
{
    return Reg::anybit(m_i2cx_con + I2CXSTAT, e);
}

//=============================================================================
    void        I2c::buscol_clr         ()
//=============================================================================
{
    Reg::clrbit(m_i2cx_con + I2CXSTAT, BUSCOL);
}

//=============================================================================
    void        I2c::txcol_clr          ()
//=============================================================================
{
    Reg::clrbit(m_i2cx_con + I2CXSTAT, TXCOL);
}

    //=============================================================================
    void        I2c::rxoflow_clr        ()
//=============================================================================
{
    Reg::clrbit(m_i2cx_con + I2CXSTAT, RXOFLOW);
}

//I2CXADDR
//=============================================================================
    void        I2c::addr               (uint16_t v)
//=============================================================================
{
    Reg::val(m_i2cx_con + I2CXADDR, v);
}

//I2CXMSK
//=============================================================================
    void        I2c::addr_mask          (uint16_t v)
//=============================================================================
{
    Reg::val(m_i2cx_con + I2CXMSK, v);
}

//I2CXBRG
//=============================================================================
    void        I2c::speed              (I2CSPEED e)
//=============================================================================
{
    uint32_t sck = e * 2;
    uint32_t clk = Osc::sysclk()<<6;
    clk = clk / sck - clk / 9615384; //1/9615384=104ns=Tpgd
    brg((clk>>6) - 2);
    m_speed = e;
}

//=============================================================================
    void        I2c::brg                (uint16_t v)
//=============================================================================
{
    if(v < 2) v = 2; //0,1 not allowed
    Reg::val(m_i2cx_con + I2CXBRG, v);
}

//I2CXTRN
//=============================================================================
    void        I2c::write              (uint8_t v)
//=============================================================================
{
    Reg::val(m_i2cx_con + I2CXTRN, v);
}

//I2CXRCV
//=============================================================================
    uint8_t     I2c::read               ()
//=============================================================================
{
    return Reg::val8(m_i2cx_con + I2CXRCV);
}