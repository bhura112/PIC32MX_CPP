#pragma once

#include <cstdint>

//Compare 1/2/3

struct Comp {

    //instantiate Comp with comparator number
    enum CMX { CMP1, CMP2, CMP3 };
    Comp(CMX);

    void            on              (bool);
    void            out             (bool);
    void            out_inv         (bool);
    bool            evt_bit         ();
    bool            out_bit         (void);

    enum EVPOL { OFF, LH, HL, ANY };
    void            evt_sel         (EVPOL);

    void            cref_cxina      (bool);

    enum CCH { CXINB, CXINC, CXIND, BGAP };
    void            ch_sel          (CCH);

    //common for all instances
    static void     stop_idle       (bool);

    enum CVREF { INT_BGAP, EXT_CVREF };
    static void     cref_sel        (CVREF);

    private:

    volatile uint32_t* m_cmpx_con;

};
