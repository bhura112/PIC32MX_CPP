#ifndef _TIMER1_H
#define _TIMER1_H

#include <cstdint>
#include "Reg.hpp"

namespace Timer1 {

    enum {
        SET = 2, CLR = 1,
        T1CON = 0xBF808000,
        TMR1 = 0xBF808010,
        PR1 = 0xBF808020,
        ON = 1<<15, SIDL = 1<<13, TWDIS = 1<<12, TWIP = 1<<11,
        EXT_RES = 3<<8, EXT_LPRC = 2<<8, EXT_T1CK = 1<<8, EXT_SOSC = 0<<8,
        TGATE = 1<<7,
        TCKPS_256 = 3<<4, TCKPS_64 = 2<<4, TCKPS_8 = 1<<4, TCKPS_1 = 0<<4,
        TSYNC = 1<<2,
        TCS = 1<<1
    };

    void timer1( uint16_t n ){ Reg::val( (volatile uint32_t*)TMR1, n ); }
    uint16_t timer1( void ){ return Reg::val( (volatile uint32_t*)TMR1 ); }

    void pr1( uint16_t n ){ Reg::val( (volatile uint32_t*)PR1, n ); }
    uint16_t pr1( void ){ return Reg::val( (volatile uint32_t*)PR1 ); }

    void on( void ){    Reg::set( (volatile uint32_t*)T1CON, ON ); }
    void off( void ){   Reg::clr( (volatile uint32_t*)T1CON, ON ); }

    void stop_idle( void ){ Reg::set( (volatile uint32_t*)T1CON, SIDL ); }
    void cont_idle( void ){ Reg::clr( (volatile uint32_t*)T1CON, SIDL ); }

    void async_wrdis( void ){ Reg::set( (volatile uint32_t*)T1CON, TWDIS ); }
    void async_wren( void ){ Reg::clr( (volatile uint32_t*)T1CON, TWDIS ); }

    bool wr_busy( void ){ return Reg::is_set( (volatile uint32_t*)T1CON, TWIP ); }

    void clk_sosc( void ){ Reg::clr( (volatile uint32_t*)T1CON, EXT_RES ); }
    void clk_lprc( void ){
        clk_sosc();
        Reg::set( (volatile uint32_t*)T1CON, EXT_LPRC );
    }
    void clk_t1ck( void ){
        clk_sosc();
        Reg::set( (volatile uint32_t*)T1CON, EXT_T1CK) ;
    }

    void tgate_on( void ){      Reg::set( (volatile uint32_t*)T1CON, TGATE ); }
    void tgate_off( void ){     Reg::clr( (volatile uint32_t*)T1CON, TGATE ); }

    void prescale_1( void ){
        Reg::clr( (volatile uint32_t*)T1CON, TCKPS_256 );
    }
    void prescale_256( void ){
        Reg::set( (volatile uint32_t*)T1CON, TCKPS_256 );
    }
    void prescale_64( void ){
        prescale_1();
        Reg::set( (volatile uint32_t*)T1CON, TCKPS_64 );
    }
    void prescale_8( void ){
        prescale_1();
        Reg::set( (volatile uint32_t*)T1CON, TCKPS_8 );
    }

    void tsync_on( void ){      Reg::set( (volatile uint32_t*)T1CON, TSYNC ); }
    void tsync_off( void ){     Reg::clr( (volatile uint32_t*)T1CON, TSYNC ); }

    void ext_clk( void ){       Reg::set( (volatile uint32_t*)T1CON, TCS ); }
    void int_clk( void ){       Reg::clr( (volatile uint32_t*)T1CON, TCS ); }

}

#endif //_TIMER1_H
