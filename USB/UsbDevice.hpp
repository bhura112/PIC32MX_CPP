#pragma once

#include <cstdint>

struct UsbDevice {

    //really no need to keep track of usb states
    //all info is available elsewhere, such as-
    //if(Usb::power(Usb::USBPWR)){ /*>=ATTACHED*/}
    //else { /*DETACHED*/}
    //if(Usb::dev_addr()){ /*>ADDRESS*/ }
    //else { /*<ADDRESS*/ }
    //if(Usb::irq(Usb::RESUME)){ /*SUSPENDED*/ }
    //etc.


    static uint32_t timer1ms;
    static uint32_t sof_count;

    //init- true=detach/then attach, false=detach
    //return true if attached, false if not
    static bool         init        (bool);

};
