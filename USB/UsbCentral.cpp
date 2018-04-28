#include "UsbCentral.hpp"
#include "Usb.hpp"
#include "UsbBuf.hpp"
#include "Irq.hpp"
#include "UsbEP.hpp"
#include "Delay.hpp"


#include <cstdio> //debug printf

//private vars
//=============================================================================
uint32_t                    timer1ms = 0;
uint32_t                    sof_count = 0;
UsbEP0                      ep0;
//=============================================================================

//class vars
//=============================================================================
const uint8_t*              UsbCentral::m_descriptor = 0;
UsbCentral::service_t       UsbCentral::m_service = 0;
uint8_t                     UsbCentral::current_config = 1; //config 1
//=============================================================================


//private
//=============================================================================
    void detach()
//=============================================================================
{
    Usb usb; UsbBuf buf; Irq irq;

    irq.on(irq.USB, false);             //usb irq off
    usb.power(usb.USBPWR, false);       //power off
    buf.init();                         //reclaim/clear buffers
}

//private
//=============================================================================
    void attach()
//=============================================================================
{
    Usb usb; Irq irq;

    timer1ms = 0;                       //reset 1ms timer
    sof_count = 0;                      //and sof count
    while(usb.power(usb.USBBUSY));      //wait for busy

    //no writes to usb regs until powered on
    usb.power(usb.USBPWR, true);        //power on (also inits bdt table)

    //init all ep (can only do now,since need usb power first)
    ep0.init();                         //endpoint 0 handled here

    //others, call registered service with ustat set to 0xFF- which will
    //reset any other endpoints
    UsbCentral::service(0xFF);

    //start irq's
    usb.irqs(usb.SOF|usb.T1MSEC|usb.TRN|usb.IDLE);   //enable some irqs
    irq.init(irq.USB, Usb::usb_irq_pri, Usb::usb_irq_subpri, true); //usb irq on

    usb.control(usb.USBEN, true);       //enable usb module
    irq.global(true);                   //global irq's on if not already
}

//init usb (or reinit, or detach) true=init/reinit, false=detach
//return true if attached, false if detached
//global irq's enabled if attached
//=============================================================================
    bool        UsbCentral::init         (bool tf)
//=============================================================================
{
printf("\r\n\r\nUsbCentral::init(%d)\r\n",tf);
    detach();
    //if no vbus pin voltage or tf=false (wanted only detach)
    if(not Usb::vbus_ison() || not tf){
printf("false\r\n");
        return false;
    }
    attach();
    return true;                        //true=attached
}




//called from UsbISR with irq flags which were set (and had irq enabled)
//and ustat array from up to 4 TRN's, 0xFF marks end of array


//=============================================================================
    void        UsbCentral::service     (uint32_t flags, uint8_t ustat)
//=============================================================================
{
    Usb usb;

//if(flags bitand compl (usb.T1MSEC bitor usb.SOF)){
//    printf("\r\nISR> frame: %d  flags: %08x\r\n",usb.frame(),flags);
//}
if(ustat != 0xFF)
    printf("\r\nUsbCentral::service  frame: %d  flags: %08x  ustat: %d\r\n",usb.frame(),flags,ustat);

    if(flags bitand usb.T1MSEC) timer1ms++;
    if(flags bitand usb.SOF) sof_count++;
    if(flags bitand usb.RESUME) usb.irq(usb.RESUME, false);
    if(flags bitand usb.IDLE) usb.irq(usb.RESUME, true); //idle (>3ms)


    //if for ep0, and ep0 serviced request ok, continue
    if(ustat != 0xFF){
        if(ustat < 4){
            if(not ep0.service(ustat)) //if not std request
                UsbCentral::service(ustat, &ep0); //try others
        }
        else UsbCentral::service(ustat);
    }

    //do last (when debugging, can see trn's before a reset)
    if(flags bitand usb.URST) init(true);

}







//=============================================================================
    bool            UsbCentral::service (uint8_t ustat, UsbEP0* ep)
//=============================================================================
{
    bool ret = false;
    if(m_service){
        ret = m_service(ustat, ep);
    }
    return ret;
}



//pkt.wValue high=descriptor type, low=index
//device=1, config=2, string=3
//=============================================================================
    const uint8_t*  UsbCentral::get_desc (uint16_t wValue, uint16_t* siz)
//=============================================================================
{
    if(not m_descriptor){ *siz = 0; return 0; }
    uint8_t idx = wValue;
    uint8_t typ = wValue>>8;
    uint16_t i = 0;
    //find header type, if idx > 0, also find header[idx]
    for(;;){
         if(m_descriptor[i+1] == typ){
             if(not idx) break; //found it
             idx--; //wrong index, dec until 0
         }
         i += m_descriptor[i];
         if(not i){ *siz = 0; return 0; } //at end of descriptor (0 marker)
    }
    //we now have index into descriptor
    //get size of type, if config get total size
    uint16_t actsiz = typ == UsbCh9::CONFIGURATION ?
        m_descriptor[i+2] + (m_descriptor[i+3]<<8) : //total size of config
        m_descriptor[i]; //size of others
    //limit to actual size
    if(*siz > actsiz) *siz = actsiz;
    return &m_descriptor[i];
}

//set to descriptor wanted (set before usb init)
//=============================================================================
    bool    UsbCentral::set_device      (const uint8_t* d, service_t f)
//=============================================================================
{
printf("UsbCentral::set_device(%08x, %08x)\r\n",(uint32_t)d,(uint32_t)f);
    m_descriptor = d;
    m_service = f;
    return init(d and f);
}

//do nothing, return descriptor ptr for config if found, 0 if not
//=============================================================================
    const uint8_t*  UsbCentral::set_config  (uint16_t wValue)
//=============================================================================
{
    wValue and_eq 0xFF;                             //low byte only
    if(not wValue) return 0;                        //no 0 config#
    uint16_t wv = UsbCh9::CONFIGURATION<<8;         //get first config
    uint16_t siz;                                   //needed, not used
    for(;; wv++){                                   //until config# match
        const uint8_t* p = get_desc(wv, &siz);      //get pointer to config
        if(not p) break;                            //no more configs left
        if(p[5] == wValue){                         //matching number?
            current_config = wValue;                //save it
            return p;                               //return pinter to config
        }
    }
    return 0;                                       //bad config#
}

//=============================================================================
    uint16_t    UsbCentral::get_epsiz       (uint8_t n, bool tr)
//=============================================================================
{
    if(not m_descriptor) return 0;
    if(n == 0) return m_descriptor[7];                //tx/rx same for ep0
    if(tr) n += 128;                                //if tx, set bit7
    const uint8_t* p = set_config(current_config);  //get config offset
    for(; p[1] != UsbCh9::STRING; p += p[0]){       //until into STRINGs
        if(p[1] != UsbCh9::ENDPOINT or m_descriptor[2] != n) continue;
        return p[4] bitor (p[5]<<8);                //found ep, return size
    }
    return 0;                                       //not found
}

//=============================================================================
    uint8_t     UsbCentral::get_epctrl      (uint8_t n)
//=============================================================================
{
    if(not m_descriptor) return 0;
    if(n == 0) return Usb::EPTXEN|Usb::EPRXEN|Usb::EPHSHK; //ep0 always this
    uint8_t ret = Usb::EPCONDIS; //assume no control pkts on all others
    const uint8_t* p = set_config(current_config);  //get config offset
    for(; p[1] != UsbCh9::STRING; p += p[0]){       //until into STRINGs
        if(p[1] != UsbCh9::ENDPOINT) continue;      //not an endpoint, continue
        if(p[2] == n) ret or_eq Usb::EPRXEN;        //if rx, enable rx
        if(p[2] == n+128) ret or_eq Usb::EPTXEN;    //if tx, enbale tx
        //enable handshake unless isochronous
        if(p[3] != UsbCh9::ISOCHRONOUS) ret or_eq Usb::EPHSHK;
    }
    return ret;
}

//=============================================================================
    uint8_t     UsbCentral::get_status      ()
//=============================================================================
{
    if(not m_descriptor) return 0;
    const uint8_t* p = set_config(current_config);  //get config offset
    uint8_t ret = 0;
    if(p[7] bitand 0x40) ret = 1;                   //self-powered
    if(p[7] bitand 0x20) ret or_eq 2;               //remote wakeup
    return ret;
}

