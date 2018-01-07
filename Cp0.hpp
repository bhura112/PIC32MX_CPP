#pragma once

/*=============================================================================
 Coprocessor0  Count, Compare
=============================================================================*/

#include <cstdint>
#include "Irq.hpp"
#include "Reg.hpp"

struct Cp0 {

    //|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

    static uint32_t     count           ();
    static uint32_t     compare         ();
    static void         count           (uint32_t);
    static void         compare         (uint32_t);
    static void         compare_reload  (bool = false);
    static void         compare_us      (uint32_t);
    static void         compare_ms      (uint16_t);

};

/*=============================================================================
 all functions inline
=============================================================================*/
#include "Osc.hpp" //put after class, to prevent circular include problem

//vars
static uint32_t m_compare_count; //save count for reloads

uint32_t Cp0::count(){
    return __builtin_mfc0(9,0);
} //get
uint32_t Cp0::compare(){
    return __builtin_mfc0(11,0);
} //get
void Cp0::count(uint32_t v){
    __builtin_mtc0(9,0,v);
} //set
void Cp0::compare(uint32_t v){
    __builtin_mtc0(11,0,v);
} //set

//true = clear flag and enable cp0 irq
void Cp0::compare_reload(bool tf){
    compare(count() + m_compare_count);
    if(tf){
        Irq::flag_clr(Irq::CORE_TIMER);
        Irq::on(Irq::CORE_TIMER, true);
    }
}
void Cp0::compare_us(uint32_t v){
    m_compare_count = Osc::sysclk() / 2000000 * v; //cpu speed from Osc::
    compare_reload();
}
void Cp0::compare_ms(uint16_t v){
    compare_us(v * 1000);
}
