#pragma once

/*=============================================================================
 Compare1/2/3 functions
=============================================================================*/

#include <cstdint>
#include "Reg.hpp"

namespace CompALL { //CMSTAT applies to all compares- C1/C2/C3

    enum
    {
        CMSTAT = 0xBF802300,
        SIDL = 1<<13, CVREFSEL = 1<<8
    };

    void stop_idle( bool tf ){      Reg::set( CMSTAT, SIDL, tf ); }
    void ref_ext( bool tf ){        Reg::set( CMSTAT, CVREFSEL, tf ); }
};

class Comp123  {

    public:

        enum CMXCON : uint32_t
        {
            C1 = 0xBF802310,
            C2 = 0xBF802320,
            C3 = 0xBF802330
        };

        enum EVPOL {   ANY = 3<<6, HL = 2<<6, LH = 1<<6, OFF = 0 };
        enum CCH {     BGAP = 3, CXIND = 2, CXINC = 1, CXINB = 0 };

    private:

        enum
        {
            ON = 1<<15, COE = 1<<14, CPOL = 1<<13,
            CEVT = 1<<9, COUT = 1<<8, CREF = 1<<4
        };

    public:

        constexpr Comp123( CMXCON n ) :
            m_pt( (volatile uint32_t*)n )
        {
        };

        void on( bool tf ){             Reg::set( m_pt, ON, tf ); }
        void out( bool tf ){            Reg::set( m_pt, COE, tf ); }
        void out_inv( bool tf ){        Reg::set( m_pt, CPOL, tf ); }
        bool event_bit( void ){         return Reg::is_set( m_pt, CEVT ); }
        bool out_bit( void ){           return Reg::is_set( m_pt, COUT ); }
        void event_sel( EVPOL e ){ Reg::clr( m_pt, ANY ); Reg::set( m_pt, e ); }
        void cref_refsel( void ){       Reg::set( m_pt, CREF ); }
        void cref_cxina( void ){        Reg::clr( m_pt, CREF ); }
        void ch_sel( CCH e ){ Reg::clr( m_pt, BGAP ); Reg::set( m_pt, e ); }

    private:

        volatile uint32_t * m_pt;       //register CMxCON
};
