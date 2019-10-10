#ifndef _USB_HOST_H_
#define _USB_HOST_H_


#define EP_0   0
#define EP_IN  1
#define EP_OUT 2 

#define E_FATAL -1
#define E_STALL -3

#ifndef E_AGAIN
#define E_AGAIN -2
#endif

/*
 * Interrupts
 */
#define BRESET_INT   0x8000
#define SOF_INT      0x4000
#define RX_INT       0x2000
#define TX_HOLD_INT  0x1000
#define TX_INT       0x0800
#define NAK_SENT_INT 0x0400
#define TIMEOUT_INT  0x0200
#define TOGGLE_ERR   0x0040
#define SETUP_INFO   0x0080

/*
 * PID tokens
 */
#define P_SETUP   0x10
#define P_DATA    0x20
#define PID_SOF   0x05
#define PID_SETUP 0x0D
#define PID_IN    0x09
#define PID_OUT   0x01
#define PID_DATA0 0x03
#define PID_DATA1 0x0B
#define PID_ACK   0x02
#define PID_NACK  0x0A
#define PID_STALL 0x0E
#define PID_NYET  0x06
#define PID_PING  0x04


/*
 * USB addresses
 */

#ifndef USB_BASE
#define USB_BASE    0xFC80U
#endif
#define USB_CONFIG  (USB_BASE)
#define USB_CONTROL (USB_BASE+1)
#define USB_STATUS  (USB_BASE+2)

#define USB_STF_BUS_RESET (1<<15)
#define USB_STF_SOF       (1<<14)
#define USB_STF_RX        (1<<13)
#define USB_STF_TX_READY  (1<<12)
#define USB_STF_TX_EMPTY  (1<<11)
#define USB_STF_NAK       (1<<10)
#define USB_STF_SETUP     (1<<7)
#define USB_STM_LAST_EP   (15<<0)



#define USB_EP_OUT0 (USB_BASE+8)
#define USB_EP_OUT1 (USB_BASE+9)
#define USB_EP_OUT2 (USB_BASE+10)
#define USB_EP_OUT3 (USB_BASE+11)
#define USB_EP_OUT4 (USB_BASE+12)
#define USB_EP_OUT5 (USB_BASE+13)
#define USB_EP_OUT6 (USB_BASE+14)
#define USB_EP_OUT7 (USB_BASE+15)


#define USB_STF_OUT_BULK (0<<14)
#define USB_STF_OUT_INT  (1<<14)
#define USB_STF_OUT_ISO  (3<<14)
#define USB_STF_OUT_ENABLE (1<<13)
#define USB_STF_OUT_STALL  (1<<12)
#define USB_STF_OUT_STALL_SENT (1<<11)
#define USB_STF_OUT_EP_SIZE (1<<8)
#define USB_STF_IN_BULK (0<<6)
#define USB_STF_IN_INT  (1<<6)
//#define USB_STF_IN_ISO  (2<<6)
#define USB_STF_IN_ENABLE (1<<5)
#define USB_STF_IN_STALL (1<<4)
#define USB_STF_IN_STALL_SENT (1<<3)
#define USB_STF_IN_NACK_SENT (1<<2)
#define USB_STF_IN_EMPTY (1<<1)
#define USB_STF_IN_FORCE_NACK (1<<0)


#define RX_FLAG     0x01
#define TX_FLAG     0x02
#define SETUP_FLAG  0x04
#define BRESET_FLAG 0x08




#define DATA1_EXPECTED (1<<11)
#define DATA0_EXPECTED 0
#define SEND_PACKET    (1<<9)
#define TIME_OUT       (1<<9)

#define  GET_LINES() (PERIP(USB_UTMIR)>>14)

#define LINE_J 0x01
#define LINE_K 0x02

#endif
