#ifndef _TIMER23_H
#define _TIMER23_H

#include <cstdint>
#include "Reg.hpp"

class Timer23 {

    public:
        
        enum TMR23 : uint32_t {
            T2 = 0xBF808040,
            T3 = 0xBF808080
        };

    private:

        enum {
            TMRX = 4,
            PRX = 8,
            ON = 1<<15, SIDL = 1<<13,
            EXT_RES = 3<<8, EXT_LPRC = 2<<8, EXT_T1CK = 1<<8, EXT_SOSC = 0<<8,
            TGATE = 1<<7,
            TCKPS_256 = 7<<4, TCKPS_64 = 6<<4, TCKPS_32 = 5<<4, TCKPS_16 = 4<<4,
            TCKPS_8 = 3<<4,   TCKPS_4 = 2<<4,  TCKPS_2 = 1<<4,  TCKPS_1 = 0<<4,
            T32 = 1<<3,
            TCS = 1<<1
        };

    public:

        Timer23( TMR23 );

        void timer( uint16_t );
        uint16_t timer( void );

        void pr( uint16_t );
        uint16_t pr( void );

        void on( void );
        void off( void );

        void stop_idle( void );
        void cont_idle( void );

        void tgate_on( void );
        void tgate_off( void );

        void pre_256( void );
        void pre_64( void );
        void pre_32( void );
        void pre_16( void );
        void pre_8( void );
        void pre_4( void );
        void pre_2( void );
        void pre_1( void );

        void t32bit( void );
        void t16bit( void );

        void ext_clk( void );
        void int_clk( void );

    private:

        volatile uint32_t * m_pt;
};

#endif //_TIMER1_H