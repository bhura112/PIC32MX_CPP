#include "Pins.hpp"
#include "Syskey.hpp"

/*=============================================================================
 Pins functions
=============================================================================*/

void Pins::lowison(bool tf){ m_lowison = tf; }
void Pins::digital_in() const { r.setb(m_pt+TRIS, m_pn); r.setb(m_pt, m_pn, 0); }
void Pins::analog_in() const { r.setb(m_pt+TRIS, m_pn); r.setb(m_pt, m_pn); }
void Pins::digital_out() const { r.setb(m_pt+TRIS, m_pn, 0); r.setb(m_pt, m_pn, 0); }
void Pins::odrain(bool tf) const { r.setb(m_pt+ODC, m_pn, tf); }
void Pins::pullup(bool tf) const { r.setb(m_pt+CNPU, m_pn, tf); }
void Pins::pulldn(bool tf) const { r.setb(m_pt+CNPD, m_pn, tf); }
void Pins::icn(bool tf) const { r.setb(m_pt+CNCON, ON, tf); }
void Pins::icn_rising() const {
    r.setb(m_pt+CNCON, CNSTYLE);
    r.setb(m_pt+CNEN0, m_pn);
    r.setb(m_pt+CNEN1, m_pn, 0);
}
void Pins::icn_risefall() const {
    r.setb(m_pt+CNCON, CNSTYLE);
    r.setb(m_pt+CNEN0, m_pn);
    r.setb(m_pt+CNEN1, m_pn, 0);
}
void Pins::icn_falling() const {
    r.setb(m_pt+CNCON, CNSTYLE);
    r.setb(m_pt+CNEN1, m_pn);
    r.setb(m_pt+CNEN0, m_pn, 0);
}
void Pins::icn_mismatch() const {
    r.setb(m_pt+CNEN0, m_pn);
    r.setb(m_pt+CNCON, CNSTYLE, 0);
}
bool Pins::icn_flag() const { return r.anybit(m_pt+CNF, m_pn); }
bool Pins::icn_stat() const { return r.anybit(m_pt+CNSTAT, m_pn); }

/*=============================================================================
 Pins functions - static
=============================================================================*/
//unlock, write byte, lock
void Pins::pps_do(uint32_t r, uint8_t v){
    Syskey::unlock();
    Reg::setb(RPCON, IOLOCK, 0);
    Reg::val(r, v);
    Reg::setb(RPCON, IOLOCK);
    Syskey::lock();
}
//pps off for peripheral
void Pins::pps_off(PPSIN e){ pps_in(e, (RPN)0); }
//pin -> pps peripheral in
void Pins::pps_in(PPSIN e, RPN n){
    pps_do(RPINR1+((e/4)*16)+(e%4), n&31);
    //Reg::setb(ANSELA + TRIS + ((n>>8)/16)*0x100, 1<<((n>>8)%16));  //tris=1
    //Reg::setb(ANSELA + ((n>>8)/16)*0x100, 1<<((n>>8)%16), 0);      //ansel=0
}
//pin output not using pps
void Pins::pps_off(RPN n){ pps_out(PPSOFF, n); }
//pps peripheral out -> pin
void Pins::pps_out(PPSOUT e, RPN n){
    pps_do(RPOR0+((((n&31)-1)/4)*16)+(((n&31)-1)%4), e);
}

