#include "Delay.hpp"
#include "Cp0.hpp"
#include "Osc.hpp"

//Delay functions

//=============================================================================
            auto Delay::
expired     () -> bool
            {
            if( m_expired ) return true; //do not calc, already know
            if((Cp0::count() - m_start) >= m_countn) m_expired = true;
            return m_expired;
            }

//force time expired
//=============================================================================
            auto Delay::
expire      () -> void
            {
            m_expired = true;
            }

//=============================================================================
            auto Delay::
restart     () -> void
            {
            m_start = Cp0::count();
            m_expired = false;
            }

//=============================================================================
            auto Delay::
set_us      (uint32_t n) -> void
            {
            if(n > 300000000) n = 300000000;
            set_count(n);
            }

//=============================================================================
            auto Delay::
set_ms      (uint32_t n) -> void
            {
            if(n > 300000) n = 300000;
            set_us(1000 * n);
            }

//=============================================================================
            auto Delay::
set_s       (uint16_t n) -> void
            {
            if(n > 300) n = 300;
            set_ms(1000 * n);
            }

//private
//=============================================================================
            auto Delay::
set_count   (uint32_t n) -> void
            {
            restart();
            m_countn = Osc::sysclk() / 2000000 * n;
            }

//static
//=============================================================================
            auto Delay::
wait_us     (uint32_t n) -> void
            {
            if(n > 300000000) n = 300000000;
            n = Osc::sysclk() / 2000000 * n;
            uint32_t start = Cp0::count();
            while((Cp0::count() - start) < n);
            }

//=============================================================================
            auto Delay::
wait_ms     (uint32_t n) -> void
            {
            if(n > 300000) n = 300000;
            wait_us(1000 * n);
            }

//=============================================================================
            auto Delay::
wait_s      (uint16_t n) -> void
            {
            if(n > 300) n = 300;
            wait_ms(1000 * n);
            }
