#pragma once

#include "Usb.hpp"


class UsbEndpt {
/*______________________________________________________________________________

    endpoint class
    handles all functions for an endpoint

    UsbEndpt ep0(0,UsbEndpt::TRX); //endpoint 0 with rx+tx
    UsbEndpt ep1rx(1,UsbEndpt::TX); //endpoint 1 with tx only
    UsbEndpt ep2tx(2,UsbEndpt::RX); //endpoint 2 with rx only
        endpoint descriptors (4) for endpoint cleared
        endpoint register cleared
        rxbuffer addresses received from UsbBuf (if endpoint has rx)
        endpoint descriptors init


    ep0.deinit(); //give up any buffers, turn off endpoint
    ep0.reinit(); //deinit, then run constructor (init)
        (cannot change endpoint number or rx/tx properties)

    ep0.on(true); //turn on endpoint with handshake
    ep0.on(false); //turn off endpoint
    ep1rx.on(ep1rx.RXEN|ep1rx.CTRLDIS); //turn on with specific options


______________________________________________________________________________*/
    public:

    //capabilities (shifted into U1EPn bit positions)
    enum TR { NONE = 0, TX = 1<<2, RX = 2<<2, TRX = 3<<2 };

    //endpoint control register bits
    enum U1EP {
        //LS = 1<<7,            /*HOST mode and U1EP0 only*/
        //RETRYDIS = 1<<6,      /*HOST mode and U1EP0 only*/
        CTRLDIS = 1<<4,         /*only when TXEN=1 && RXEN=1*/
        RXEN = 1<<3, TXEN = 1<<2, ESTALL = 1<<1, HSHAKE = 1<<0
    };

    //public functions
    UsbEndpt(uint8_t, TR); //n, TX|RX|TRX

    void deinit ();         //'destructor'-like
    void reinit ();         //deinit, then run constructor
    void on     (bool);     //enable/disable endpoint ([tx],[rx],handshake)
    void on     (uint8_t);  //enable endpoint with specified U1EP reg bits
    void token  (uint8_t);  //process transaction complete, called by isr

    //public static function
    static uint32_t bdt_addr(); //to get bdt address (for Usb::bdt_addr() use)



    private:

    //bd entry struct
    typedef union {
        struct { uint32_t dat; uint32_t addr; };
        struct { unsigned :16; uint16_t count; };
        struct {
            unsigned :2;
            unsigned bstall:1;
            unsigned dts:1;
            unsigned ninc:1;
            unsigned keep:1;
            unsigned data01:1;
            unsigned uown:1;
        };
        struct { unsigned :2; unsigned pid:4; };
        uint8_t control;
        uint64_t all;
    } bdt_t;

    //endpoint register address, spacing
    enum { U1EP0 = 0xBF808700, U1EP_SPACING = 0x10 };

    //bd entry control bits (out to usb sie)
    enum CTRLOUT {
        UOWN = 1<<7, DATA01 = 1<<6, KEEP = 1<<5,
        NINC = 1<<4, DTS = 1<<3, BSTALL = 1<<1
    };

    //private functions
    void                bd_ctrl     (uint8_t);
    volatile uint8_t    bd_ctrl     ();
    volatile uint16_t   bd_count    ();
    void                bd_count    (uint16_t);
    volatile uint8_t    bd_pid      () const;
    void                bd_addr     (volatile uint8_t*);
    uint8_t*            bd_addr     () const;
    volatile bool       bd_uown     () const;
    volatile bool       bd_data01   () const;
    volatile bool       bd_stall    () const;
    void                bd_uown     (bool);
    void                bd_data01   (bool);
    void                bd_stall    (bool);
    void                bd_setup    (volatile uint8_t*, uint16_t, uint8_t);
    void                epreg       (U1EP, bool);
    volatile bool       epreg       (U1EP) const;
    void                epreg       (uint8_t);
    void                bd_init     ();
    void                setup_token ();
    void                in_token    ();
    void                out_token   ();
    void                rx_giveup   ();
    void                tx_cancel   ();

    //in/out matches setup_pkt.dir- 0=from host,1=to host
    enum SETUPXFER { COMPLETE = 3, IN = 1, OUT = 0, STATUS = 2 };

    static bdt_t        m_bdt[(my_last_endp+1)*4] __attribute__((aligned(512)));
    uint8_t             m_ep_n;         //0-15
    TR                  m_ep_trx;       //TX|RX|TRX (aligned for U1EPn <<2)
    volatile uint8_t*   m_ep_reg;       //U1EPn sfr register
    volatile uint8_t*   m_rxbuf[2];     //pointers to rx-even/odd buffers
    uint16_t            m_buf_len;      //fixed length from UsbBuf (64)
    uint8_t             m_tx_ep;        //current tx endpoint (2=even/3=odd)
    volatile bdt_t*     m_bd[4];        //rx/tx buffer descriptor even/odd
    UsbCh9::SetupPacket_t setup_pkt;    //copy of setup data packet (8bytes)
    SETUPXFER           setup_stage;    //setup transaction stages
    uint8_t             m_bdi;          //current bd index (used in functions)
    uint8_t             m_idx;          //save idx (from isr call)    
    uint8_t             m_data01;       //tx data0/data1, uses bit6 (toggle bit6)
    volatile uint8_t*   m_tx_ptr;       //pointer to tx data
};


//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//static
//UsbHandlers::attach will get bdt address, and set the address in the sfr reg
UsbEndpt::bdt_t UsbEndpt::m_bdt[(my_last_endp+1)*4] = {0};
uint32_t UsbEndpt::bdt_addr(){ return (uint32_t)m_bdt; }

//public
UsbEndpt::UsbEndpt(uint8_t n, TR tr) :
    m_tx_ep(2),
    m_ep_n(n&15),
    m_ep_trx(tr),
    m_bd{&m_bdt[n*4],&m_bdt[n*4+1],&m_bdt[n*4+2],&m_bdt[n*4+3]},
    m_ep_reg((volatile uint8_t*)U1EP0+n*U1EP_SPACING),
    m_buf_len(UsbBuf::buf_len()),
    setup_pkt{0},
    setup_stage(COMPLETE),
    m_bdi(0),
    m_idx(0),
    m_data01(0),
    m_rxbuf{0,0}
{
    epreg(0);
    if(tr&RX){ m_rxbuf[0] = UsbBuf::get(); m_rxbuf[1] = UsbBuf::get();}
    bd_init();
}
//UsbEndpt::UsbEndpt(uint8_t n, TR tr){}
void UsbEndpt::deinit(){
    epreg(0);
    UsbBuf::release(m_rxbuf[0]);
    UsbBuf::release(m_rxbuf[1]);
    tx_cancel();
}
void UsbEndpt::reinit(){ deinit(); UsbEndpt(m_ep_n,m_ep_trx); }
void UsbEndpt::bd_ctrl(uint8_t v){ m_bd[m_bdi]->control = v; }
volatile uint8_t UsbEndpt::bd_ctrl(){ return m_bd[m_bdi]->control; }
volatile uint16_t UsbEndpt::bd_count(){ return m_bd[m_bdi]->count; }
void UsbEndpt::bd_count(uint16_t v){ m_bd[m_bdi]->count = v; }
volatile uint8_t UsbEndpt::bd_pid() const { return m_bd[m_bdi]->pid; }
void UsbEndpt::bd_addr(volatile uint8_t* v){
    m_bd[m_bdi]->addr = Reg::k2phys(v);
}
uint8_t* UsbEndpt::bd_addr() const {
    return (uint8_t*)Reg::p2kseg0(m_bd[m_bdi]->addr);
}
volatile bool UsbEndpt::bd_uown() const { return m_bd[m_bdi]->uown; }
volatile bool UsbEndpt::bd_data01() const { return m_bd[m_bdi]->data01; }
volatile bool UsbEndpt::bd_stall() const { return m_bd[m_bdi]->bstall; }
void UsbEndpt::bd_uown(bool tf){ m_bd[m_bdi]->uown = tf; }
void UsbEndpt::bd_data01(bool tf){ m_bd[m_bdi]->data01 = tf; }
void UsbEndpt::bd_stall(bool tf){ m_bd[m_bdi]->bstall = tf; }
void UsbEndpt::bd_setup(volatile uint8_t* a, uint16_t c, uint8_t f){
    bd_addr(a); bd_count(c); bd_ctrl(f);
}
void UsbEndpt::epreg(U1EP e, bool tf){ Reg::set(m_ep_reg, e, tf); }
volatile bool UsbEndpt::epreg(U1EP e) const {
    return Reg::is_set8(m_ep_reg, e);
}
void UsbEndpt::epreg(uint8_t v){ Reg::val8(m_ep_reg, v); }
void UsbEndpt::bd_init(){
    for(auto i = 3; i >= 0; i--){
        m_bdi = i;
        bd_setup(0, 0, 0);
        if((m_ep_trx & RX) && (i < 2)) bd_addr(m_rxbuf[i]);
    }
}
void UsbEndpt::on(bool tf){ if(tf) epreg(m_ep_trx|HSHAKE); else epreg(0); }
void UsbEndpt::on(uint8_t v){ epreg(v); }
void UsbEndpt::rx_giveup(){ bd_count(m_buf_len); bd_uown(true); }
void UsbEndpt::tx_cancel(){
    for(m_bdi = 2; m_bdi < 4; m_bdi++){
        if(bd_uown()) m_tx_ep ^= 1; //toggle, since tx uown 1->0 (?)
        bd_ctrl(0);
        UsbBuf::release(bd_addr());
    }
    UsbBuf::release(m_tx_ptr);
    m_tx_ptr = 0;
    m_bdi = m_idx; //restore
}


/*..............................................................................
 process transaction complete
    called from ISR with index into this endpoint of which caused the irq
    (0-3, rx-even,rx-odd,tx-even,tx-odd)
..............................................................................*/
void UsbEndpt::token(uint8_t idx){
    Usb u; //just for style, using u. instead of Usb::

    //check if we even have an endpoint in use
    //if this is an unused endpoint below a higher used endpoint
    //turn off (should already be off, and shouldn't be here)
    if(m_ep_trx = NONE){
        on(false);
        return;
    }

    //which bd entry was used for this TRNIF
    //m_idx backup copy, when we need to temp change m_bdi
    m_bdi = m_idx = idx;

    //pid from bd entry (should only see 1,9,13)
    switch(bd_pid()){

        case UsbCh9::SETUP:
            //save setup data packet for later
            setup_pkt = *(UsbCh9::SetupPacket_t*)&m_rxbuf[m_bdi];
            rx_giveup(); //done with rx buffer
            tx_cancel(); //anything in tx, cancel
            setup_token(); //process
            u.control(u.PKTDIS, false); //enable packets in control transfer
            break;

        case UsbCh9::IN:
            m_tx_ep ^= 1; //tx endpoint toggled, keep track
            in_token();
            break;

        case UsbCh9::OUT:
            out_token();
            rx_giveup(); //done with rx buffer
            break;
    }
}

/*..............................................................................

..............................................................................*/
void UsbEndpt::setup_token(){

    bool dir = setup_pkt.dir;
    uint8_t typ = setup_pkt.type;
    uint8_t recip = setup_pkt.recip;

    m_tx_ptr = UsbBuf::get();
    if(!m_tx_ptr){} //do something if can't get buffer- stall?

    //standard requests
    switch(setup_pkt.bRequest){
        case UsbCh9::CLEAR_FEATURE:
            //setup_pkt.wValue
            break;
        case UsbCh9::SET_FEATURE:
            //setup_pkt.wValue
            break;
        //case UsbCh9::SET_ADDRESS:
            //do after status
            //break;
        case UsbCh9::SET_CONFIGURATION:
            //setup_pkt.wValue
            break;
        case UsbCh9::GET_STATUS:
            //setup_pkt.wLength should = 2
            //2bytes- byte0/bit0 = self-powered?, byte0/bit1=remote wakeup?
            m_tx_ptr[0] = my_self_powered<<0;
            m_tx_ptr[0] |= my_remote_wakeup<<1;
            m_tx_ptr[1] = 0;
            break;
        case UsbCh9::GET_DESCRIPTOR:
            //
            break;
        case UsbCh9::SET_DESCRIPTOR:
            //
            break;
        case UsbCh9::GET_CONFIGURATION:
            //return 1 byte- 0=not configured
            //
            break;
    }

    m_data01 = 0<<6;                //first data0
    setup_stage = (SETUPXFER)dir;   //0=out,1=in
    if(setup_pkt.wLength == 0){     //if no data stage
        setup_stage = STATUS;       //in status stage
        m_data01 = 1<<6;            //and status 0byte is data1
    }

    m_bdi = m_tx_ep; //set bd to match tx endpoint
    bd_setup(m_tx_ptr, setup_pkt.wLength, UOWN|m_data01);
    m_tx_ep ^= 1;
    m_bdi = m_idx;

    //if no data stage,
    //  now in STATUS state and tx is ready for IN status
    //if OUT data stage is next,
    //  now in OUT state and tx is ready for IN status
    //if IN data stage, tx is ready for IN

}
/*..............................................................................

..............................................................................*/
void UsbEndpt::in_token(){
    if(setup_stage == STATUS){
        setup_stage = COMPLETE;
        UsbBuf::release(m_tx_ptr);
        m_tx_ptr = 0;
    }
    if(setup_stage == IN){
    //send more, or go to STATUS stage and setup tx for 0byte OUT
    }
    if(setup_stage == OUT){
    //problem
    }

}
/*..............................................................................

..............................................................................*/
void UsbEndpt::out_token(){
    if(setup_stage == STATUS){
        setup_stage = COMPLETE;
        UsbBuf::release(m_tx_ptr);
        m_tx_ptr = 0;
    }
    if(setup_stage == IN){
    //problem
    }
    if(setup_stage == OUT){
    //rx more, or go to STATUS stage (already setup)
    }

}
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::


/*..............................................................................

..............................................................................*/