#include "Ccp.hpp"
#include "Osc.hpp"

enum : uint32_t {
CCPX_SPACING = 0x40,  //spacing in words
CCP1CON1 = 0xBF800100,
    OPSSRC = 31,
    RTRGEN = 30,
    OPS_SHIFT = 24, OPS_MASK = 15,
    TRIGEN = 23,
    ONESHOT = 22,
    ALTSYNC = 21,
    SYNC_SHIFT = 16, SYNC_MASK = 31,
    ON = 15,
    SIDL = 13,
    CCPSLP = 12,
    TMRSYNC = 11,
    CLKSEL_SHIFT = 8, CLKSEL_MASK = 7,
    TMRPS_SHIFT = 6, TMRPS_MASK = 3,
    T32 = 5, //combined into MODE
    CCSEL = 4, //combined into MODE
    MOD_SHIFT = 0, MOD_MASK = 15,
    MODE_SHIFT = 0, MODE_MASK = 63,
CCPXCON2 = 4, //offset in words from ccp1con1
    OENSYNC = 31,
    OCFEN = 29,
    OCEEN = 28,
    OCDEN = 27,
    OCCEN = 26,
    OCBEN = 25,
    OCAEN = 24,
    OCPINS_SHIFT = 24, OCPINS_MASK = 63,
    ICGSM_SHIFT = 22, ICGSM_MASK = 3,
    AUXOUT_SHIFT = 19, AUXOUT_MASK = 3,
    ICS_SHIFT = 16, ICS_MASK = 7,
    PWMRSEN = 15,
    ASDGM = 14,
    SSDG = 12,
    ASDG_SHIFT = 0, ASDG_MASK = 255,
CCPXCON3 = 8, //offset in words from ccp1con1
    OETRIG = 31,
    OSCNT_SHIFT = 28, OSCNT_MASK = 7,
    OUTM_SHIFT = 24, OUTM_MASK = 7,
    POLACE = 21,
    POLBDF = 20,
    PSSACE_SHIFT = 18, PSSACE_MASK = 3,
    PSSBDF_SHIFT = 16, PSSBDF_MASK = 3,
    DT_SHIFT = 0, DT_MASK = 63,
CCPXSTAT = 12, //offset in words from ccp1con1
    PRLWIP = 20,
    TMRHWIP = 19,
    TMRLWIP = 18,
    RBWIP = 17,
    RAWIP = 16,
    ICGARM = 10,
    CCPTRIG = 7,
    TRSET = 6,
    TRCLR = 5,
    ASEVT = 4,
    SCEVT = 3,
    ICDIS = 2,
    ICOV = 1,
    ICBNE = 0,
CCPXTMR = 16, //offset in words from ccp1con1
CCPXPR = 20, //offset in words from ccp1con1
CCPXRA = 24, //offset in words from ccp1con1
CCPXRB = 28, //offset in words from ccp1con1
CCPXBUF = 32 //offset in words from ccp1con1
};

//MCCP/SCCP

//=============================================================================
            Ccp::
Ccp         (uint8_t n)
            : m_ccpx_con((vu32ptr)CCP1CON1 + (n * CCPX_SPACING))
            {
            }

//ccpxcon1
//=============================================================================
            auto Ccp::
outscale_src (OPOSTSRC e) -> void
            {
            setbit(m_ccpx_con, 1<<OPSSRC, e);
            }

//=============================================================================
            auto Ccp::
retrigger   (bool tf) -> void
            {
            setbit(m_ccpx_con, 1<<RTRGEN, tf);
            }

//=============================================================================
            auto Ccp::
outscale    (uint8_t v) -> void
            {
            clrbit(m_ccpx_con, OPS_MASK<<OPS_SHIFT);
            setbit(m_ccpx_con, (v bitand OPS_MASK)<<OPS_SHIFT);
            }

//=============================================================================
            auto Ccp::
trig_mode   () -> void
            {
            setbit(m_ccpx_con, 1<<TRIGEN);
            }

//=============================================================================
            auto Ccp::
sync_mode   () -> void
            {
            clrbit(m_ccpx_con, 1<<TRIGEN);
            }

//=============================================================================
            auto Ccp::
oneshot     (bool tf) -> void
            {
            setbit(m_ccpx_con, 1<<ONESHOT, tf);
            }

//=============================================================================
            auto Ccp::
sync_altout (bool tf) -> void
            {
            setbit(m_ccpx_con, 1<<ALTSYNC, tf);
            }

//=============================================================================
            auto Ccp::
sync_src    (SYNC e) -> void
            {
            clrbit(m_ccpx_con, SYNC_MASK<<SYNC_SHIFT);
            setbit(m_ccpx_con, e<<SYNC_SHIFT);
            }

//=============================================================================
            auto Ccp::
on          (bool tf) -> void
            {
            setbit(m_ccpx_con, 1<<ON, tf);
            }

//=============================================================================
            auto Ccp::
stop_idle   (bool tf) -> void
            {
            setbit(m_ccpx_con, 1<<SIDL, tf);
            }

//=============================================================================
            auto Ccp::
run_sleep   (bool tf) -> void
            {
            setbit(m_ccpx_con, 1<<CCPSLP, tf);
            }

//=============================================================================
            auto Ccp::
sync_tmr    (bool tf) -> void
            {
            setbit(m_ccpx_con, 1<<TMRSYNC, tf);
            }

//=============================================================================
            auto Ccp::
clk_src     (CLKSEL e) -> void
            {
            clrbit(m_ccpx_con, CLKSEL_MASK<<CLKSEL_SHIFT);
            setbit(m_ccpx_con, e<<CLKSEL_SHIFT);
            if(e == SOSC) Osc::sosc(true);
            }

//=============================================================================
            auto Ccp::
tmr_prescale (TMRPS e) -> void
            {
            clrbit(m_ccpx_con, TMRPS_MASK<<TMRPS_SHIFT);
            setbit(m_ccpx_con, e<<TMRPS_SHIFT);
            }

//=============================================================================
            auto Ccp::
mode        (MODE e) -> void
            {
            bool ison = anybit(m_ccpx_con, 1<<ON);
            on(false);
            clrbit(m_ccpx_con, MODE_MASK<<MODE_SHIFT);
            setbit(m_ccpx_con, e<<MODE_SHIFT);
            if(ison) on(true);
            }

//ccpxcon2
//=============================================================================
            auto Ccp::
out_sync    (bool tf) -> void
            {
            setbit(m_ccpx_con + CCPXCON2, 1<<OENSYNC, tf);
            }

// MCCP use fixed pins OCMnA-F
//=============================================================================
            auto Ccp::
out_pins    (OUTPINS e) -> void
            {
            clrbit(m_ccpx_con + CCPXCON2, OCPINS_MASK<<OCPINS_SHIFT);
            setbit(m_ccpx_con + CCPXCON2, e<<OCPINS_SHIFT);
            }

// SCCP uses PPS OCM4-9 (OCAEN also has to be set, reset val is 1 though)
// to set a pps pin back to lat, use optional bool value of false
// sccp4.out_pin(Pins::RP13); //set pin to use pps out OCM4
// sccp4.out_pin(Pins::RP13, false); //set pin to use pps out PPSLAT
// can set multiple pins to use the same OCMn, need to call this function
// for each pin to set
//=============================================================================
            auto Sccp::
out_pin     (Pins::RPN e, bool tf) -> void
            {
            out_pins(OCA); //OCAEN
            Pins p{e, Pins::OUT};
            //OCM4-9, figure out which one
//            uint8_t n = ccp_num() - 4 + (uint8_t)p.OCM4;
//            p.pps_out( tf ? (Pins::PPSOUT)n : Pins::PPSLAT );
            }

//=============================================================================
            auto Ccp::
gate_mode   (ICGSM e) -> void
            {
            clrbit(m_ccpx_con + CCPXCON2, ICGSM_MASK<<ICGSM_SHIFT);
            setbit(m_ccpx_con + CCPXCON2, e<<ICGSM_SHIFT);
            }

//=============================================================================
            auto Ccp::
out_aux     (AUXOUT e) -> void
            {
            clrbit(m_ccpx_con + CCPXCON2, AUXOUT_MASK<<AUXOUT_SHIFT);
            setbit(m_ccpx_con + CCPXCON2, e<<AUXOUT_SHIFT);
            }

//=============================================================================
            auto Ccp::
cap_src     (ICS e) -> void
            {
            clrbit(m_ccpx_con + CCPXCON2, ICS_MASK<<ICS_SHIFT);
            setbit(m_ccpx_con + CCPXCON2, e<<ICS_SHIFT);
            }

//=============================================================================
            auto Ccp::
pwm_restart (bool tf) -> void
            {
            setbit(m_ccpx_con + CCPXCON2, 1<<PWMRSEN, tf);
            }

//=============================================================================
            auto Ccp::
gate_auto   (bool tf) -> void
            {
            setbit(m_ccpx_con + CCPXCON2, 1<<ASDGM, tf);
            }

//=============================================================================
            auto Ccp::
gate_now    (bool tf) -> void
            {
            setbit(m_ccpx_con + CCPXCON2, 1<<SSDG, tf);
            }

//=============================================================================
            auto Ccp::
gate_autosrc (GATEAUTOSRC e) -> void
            {
            val((vu8ptr)m_ccpx_con + (CCPXCON2 * 4), e);
            }

//ccpxcon3
//=============================================================================
            auto Ccp::
out_trigwait (bool tf) -> void
            {
            setbit(m_ccpx_con + CCPXCON3, 1<<OETRIG, tf);
            }

//=============================================================================
            auto Ccp::
oneshot_extend (uint8_t v) -> void
            {
            clrbit(m_ccpx_con + CCPXCON3, OSCNT_MASK<<OSCNT_SHIFT);
            setbit(m_ccpx_con + CCPXCON3,
                        (v bitand OSCNT_MASK)<<OSCNT_SHIFT);
            }

//=============================================================================
            auto Ccp::
out_mode    (OUTM e) -> void
            {
            clrbit(m_ccpx_con + CCPXCON3, OUTM_MASK<<OUTM_SHIFT);
            setbit(m_ccpx_con + CCPXCON3, e<<OUTM_SHIFT);
            }

//=============================================================================
            auto Ccp::
polarity_ace (POLARITY e) -> void
            {
            setbit(m_ccpx_con + CCPXCON3, 1<<POLACE, e);
            }

//=============================================================================
            auto Ccp::
polarity_bdf (POLARITY e) -> void
            {
            setbit(m_ccpx_con + CCPXCON3, 1<<POLBDF, e);
            }

//=============================================================================
            auto Ccp::
shutdown_ace (SHUTDOWN e) -> void
            {
            clrbit(m_ccpx_con + CCPXCON3, PSSACE_MASK<<PSSACE_SHIFT);
            setbit(m_ccpx_con + CCPXCON3, e<<PSSACE_SHIFT);
            }

//=============================================================================
            auto Ccp::
shutdown_bdf (SHUTDOWN e) -> void
            {
            clrbit(m_ccpx_con + CCPXCON3, PSSBDF_MASK<<PSSBDF_SHIFT);
            setbit(m_ccpx_con + CCPXCON3, e<<PSSBDF_SHIFT);
            }

//=============================================================================
            auto Ccp::
dead_time   (uint8_t v) -> void
            {
            clrbit(m_ccpx_con + CCPXCON3, DT_MASK<<DT_SHIFT);
            setbit(m_ccpx_con + CCPXCON3, (v bitand DT_MASK)<<DT_SHIFT);
            }

//ccpxstat
//=============================================================================
            auto Ccp::
pr16_busy   () -> bool
            {
            return anybit(m_ccpx_con + CCPXSTAT, 1<<PRLWIP);
            }

//=============================================================================
            auto Ccp::
tmr32_busy  () -> bool
            {
            return anybit(m_ccpx_con + CCPXSTAT, 1<<TMRHWIP);
            }

//=============================================================================
            auto Ccp::
tmr16_busy  () -> bool
            {
            return anybit(m_ccpx_con + CCPXSTAT, 1<<TMRLWIP);
            }

//=============================================================================
            auto Ccp::
compb_busy  () -> bool
            {
            return anybit(m_ccpx_con + CCPXSTAT, 1<<RBWIP);
            }

//=============================================================================
            auto Ccp::
compa_busy  () -> bool
            {
            return anybit(m_ccpx_con + CCPXSTAT, 1<<RAWIP);
            }

//=============================================================================
            auto Ccp::
gate_arm    () -> void
            {
            setbit(m_ccpx_con + CCPXSTAT, 1<<ICGARM);
            }

//=============================================================================
            auto Ccp::
stat_trig   () -> bool
            {
            return anybit(m_ccpx_con + CCPXSTAT, 1<<CCPTRIG);
            }

//=============================================================================
            auto Ccp::
trig_set    () -> void
            {
            setbit(m_ccpx_con + CCPXSTAT, 1<<TRSET);
            }

//=============================================================================
            auto Ccp::
trig_clr    () -> void
            {
            setbit(m_ccpx_con + CCPXSTAT, 1<<TRCLR);
            }

//=============================================================================
            auto Ccp::
stat_shutdown () -> bool
            {
            return anybit(m_ccpx_con + CCPXSTAT, 1<<ASEVT);
            }

//=============================================================================
            auto Ccp::
stat_secomp () -> bool
            {
            return anybit(m_ccpx_con + CCPXSTAT, 1<<SCEVT);
            }

//=============================================================================
            auto Ccp::
stat_capdis () -> bool
            {
            return anybit(m_ccpx_con + CCPXSTAT, 1<<ICDIS);
            }

//=============================================================================
            auto Ccp::
stat_oflow  () -> bool
            {
            return anybit(m_ccpx_con + CCPXSTAT, 1<<ICOV);
            }

//=============================================================================
            auto Ccp::
stat_bufany () -> bool
            {
            return anybit(m_ccpx_con + CCPXSTAT, 1<<ICBNE);
            }

//ccpxtmr
//=============================================================================
            auto Ccp::
tmr16       (uint16_t v) -> void
            {
            val(m_ccpx_con + CCPXTMR, v);
            }

//=============================================================================
            auto Ccp::
tmr16h      (uint16_t v) -> void
            {
            val((vu8ptr)m_ccpx_con + (CCPXTMR * 4) + 2, v);
            }

//=============================================================================
            auto Ccp::
tmr32       (uint32_t v) -> void
            {
            val(m_ccpx_con + CCPXTMR, v);
            }

//=============================================================================
            auto Ccp::
tmr16       () -> uint16_t
            {
            return val16(m_ccpx_con + CCPXTMR);
            }

//=============================================================================
            auto Ccp::
tmr16h      () -> uint16_t
            {
            return val16((vu8ptr)m_ccpx_con + (CCPXTMR * 4) + 2);
            }

//=============================================================================
            auto Ccp::
tmr32       () -> uint32_t
            {
            return val(m_ccpx_con + CCPXTMR);
            }

//ccpxpr
//=============================================================================
            auto Ccp::
pr16        (uint16_t v) -> void
            {
            val(m_ccpx_con + CCPXPR, v);
            }

//=============================================================================
            auto Ccp::
pr16h       (uint16_t v) -> void
            {
            val((vu8ptr)m_ccpx_con + (CCPXPR * 4) + 2, v);
            }

//=============================================================================
            auto Ccp::
pr32        (uint32_t v) -> void
            {
            val(m_ccpx_con + CCPXPR, v);
            }

//=============================================================================
            auto Ccp::
pr16        () -> uint16_t
            {
            return val16(m_ccpx_con + CCPXPR);
            }

//=============================================================================
            auto Ccp::
pr16h       () -> uint16_t
            {
            return val16((vu8ptr)m_ccpx_con + (CCPXPR * 4) + 2);
            }

//=============================================================================
            auto Ccp::
pr32        () -> uint32_t
            {
            return val(m_ccpx_con + CCPXPR);
            }

//ccpxra, ccpxrb
//=============================================================================
            auto Ccp::
compa       (uint16_t v) -> void
            {
            val(m_ccpx_con + CCPXRA, v);
            }

//=============================================================================
            auto Ccp::
compb       (uint16_t v) -> void
            {
            val(m_ccpx_con + CCPXRB, v);
            }

//=============================================================================
            auto Ccp::
comp32      (uint32_t v) -> void
            {
            compb(v >> 16);
            compa(v);
            }

//=============================================================================
            auto Ccp::
compa       () -> uint16_t
            {
            return val16(m_ccpx_con + CCPXRA);
            }

//=============================================================================
            auto Ccp::
compb       () -> uint16_t
            {
            return val16(m_ccpx_con + CCPXRB);
            }

//=============================================================================
            auto Ccp::
comp32      () -> uint32_t
            {
            return compb()<<16 | compa();
            }

//ccpxbuf
//=============================================================================
            auto Ccp::
buf16       () -> uint16_t
            {
            return val16(m_ccpx_con + CCPXBUF);
            }

//=============================================================================
            auto Ccp::
buf32       () -> uint32_t
            {
            return val(m_ccpx_con + CCPXBUF);
            }

//misc
//=============================================================================
            auto Ccp::
ccp_num     () -> uint8_t
            {
            return ((uint32_t)m_ccpx_con - CCP1CON1) / (CCPX_SPACING * 4)  + 1;
            }
