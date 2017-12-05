#include "Timer23.hpp"

Timer23::Timer23( TMR23 tn ) :
    m_pt( (volatile uint32_t*)tn )
{
};

void Timer23::timer( uint16_t n ){       Reg::val( m_pt + TMRX, n ); }
uint16_t Timer23::timer( void ){         return Reg::val( m_pt + TMRX ); }

void Timer23::pr( uint16_t n ){          Reg::val( m_pt + PRX, n ); }
uint16_t Timer23::pr( void ){            return Reg::val( m_pt + PRX ); }

void Timer23::on( void ){                Reg::set( m_pt, ON ); }
void Timer23::off( void ){               Reg::clr( m_pt, ON ); }

void Timer23::stop_idle( void ){         Reg::set( m_pt, SIDL ); }
void Timer23::cont_idle( void ){         Reg::clr( m_pt, SIDL ); }

void Timer23::tgate_on( void ){          Reg::set( m_pt, TGATE ); }
void Timer23::tgate_off( void ){         Reg::clr( m_pt, TGATE ); }

void Timer23::pre_256( void ){           Reg::set( m_pt, TCKPS_256 ); }
void Timer23::pre_64( void ){            pre_1(); Reg::set( m_pt, TCKPS_64 ); }
void Timer23::pre_32( void ){            pre_1(); Reg::set( m_pt, TCKPS_32 ); }
void Timer23::pre_16( void ){            pre_1(); Reg::set( m_pt, TCKPS_16 ); }
void Timer23::pre_8( void ){             pre_1(); Reg::set( m_pt, TCKPS_8 ); }
void Timer23::pre_4( void ){             pre_1(); Reg::set( m_pt, TCKPS_4 ); }
void Timer23::pre_2( void ){             pre_1(); Reg::set( m_pt, TCKPS_2 ); }
void Timer23::pre_1( void ){             Reg::clr( m_pt, TCKPS_256 ); }

void Timer23::t32bit( void ){            Reg::set( m_pt, T32 ); }
void Timer23::t16bit( void ){            Reg::clr( m_pt, T32 ); }

void Timer23::ext_clk( void ){           Reg::set( m_pt, TCS ); }
void Timer23::int_clk( void ){           Reg::clr( m_pt, TCS ); }