#ifndef USBLOWLIB_H
#define USBLOWLIB_H
#include <vstypes.h>

#define AUDIO_ISOC_OUT_EP 0x01
#ifndef MSC_H
//below endpoints are already defined in msc.h
#define MSC_BULK_OUT_ENDPOINT 0x03
#define MSC_BULK_IN_ENDPOINT  0x02
#endif

#define ENDPOINT_SIZE_0    64
#define ENDPOINT_SIZE_1   540  /* See also: usbdescriptors.c */
#define ENDPOINT_SIZE_MAX ENDPOINT_SIZE_1

/*
 * Interrupts
 */
#define BRESET_INT   0x8000
#define SOF_INT      0x4000
#define RX_INT       0x2000
#define TX_HOLD_INT  0x1000
#define TX_INT       0x0800
#define NAK_SENT_INT 0x0400
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

/*
 * USB addresses
 */


#define USB_BASE USB_CF
#define USB_CONFIG  (USB_BASE)

#define USB_CFF_RESET  (1<<15)
#define USB_CFF_NOHIGHSPEED (1<<11)
#define USB_CFF_MASTER (1<<9)
#define USB_CFF_ENABLE (1<<7)

#define USB_CONTROL (USB_BASE+1)
#define USB_STATUS  (USB_BASE+2)

#define USB_STF_BUS_RESET (1<<15)
#define USB_STF_SOF       (1<<14)
#define USB_STF_RX        (1<<13)
#define USB_STF_TX_READY  (1<<12)
#define USB_STF_TX_EMPTY  (1<<11)
#define USB_STF_NAK       (1<<10)
#define USB_STF_SETUP     (1<<7)
#define USB_STF_SUSPEND   (1<<6)
#define USB_STF_SPEED     (1<<5)
#define USB_STM_LAST_EP   (15<<0)


#define USB_UW_OPMODE_NORMAL     0
#define USB_UW_OPMODE_NONDRIVING 1
#define USB_UW_OPMODE_NOBS_NONRZI 2
#define USB_UW_OPMODE_RESERVED   3
#define USB_UW_OPMODE_MASK       3
#define USB_UW_TERMSELECT      (1<<2)
#define USB_UW_RCVRSELECT      (1<<3)
#define USB_UW_DRIVE_CHIRP_K   (1<<4)
#define USB_UW_RESET_HANDSHAKE (1<<5)
#define USB_UW_DRIVE_CHIRP_J   (1<<6)

#define USB_EP_SEND4 (USB_BASE+12)
#define USB_EP_SEND5 (USB_BASE+13)
#define USB_EP_SEND6 (USB_BASE+14)
#define USB_EP_SEND7 (USB_BASE+15)

#define USB_EP_ST4 (USB_BASE+20)
#define USB_EP_ST5 (USB_BASE+21)
#define USB_EP_ST6 (USB_BASE+22)
#define USB_EP_ST7 (USB_BASE+23)

#define USB_STF_OUT_BULK (0<<14)
#define USB_STF_OUT_INT  (1<<14)
#define USB_STF_OUT_ISO  (3<<14)
#define USB_STF_OUT_ENABLE (1<<13)
#define USB_STF_OUT_STALL  (1<<12)
#define USB_STF_OUT_STALL_SENT (1<<11)
#define USB_STF_OUT_EP_SIZE (1<<8)
#define USB_STF_IN_BULK (0<<6)
#define USB_STF_IN_INT  (1<<6)
#define USB_STF_IN_ISO  (3<<6) /** \todo \bug TODO: check!*/
#define USB_STF_IN_ENABLE (1<<5)
#define USB_STF_IN_STALL (1<<4)
#define USB_STF_IN_STALL_SENT (1<<3)
#define USB_STF_IN_NACK_SENT (1<<2)
#define USB_STF_IN_EMPTY (1<<1)
#define USB_STF_IN_FORCE_NACK (1<<0)


#define USB_MEM_IN_Y
#define USB_MEM_TYPE __y
#define USB_MEM(a) USEY(a)
#define RING_BUF_SIZE 1024

#ifdef ASM
/* Assembler-specific stuff */

/* stp = -64 .. 63, bufsz 1..64 */
#macro MAKEMOD stp,bufsz
	(0x2000|(((stp)&127)<<6)|(((bufsz)-1)&63))
#endm

/* stp = 1, bufsz 1..8192 */
#macro MAKEMODF bufsz
	(0x8000|(((bufsz)-1)&8191))
#endm

/* stp = -1, bufsz 1..8192 */
#macro MAKEMODB bufsz
	(0xA000|(((bufsz)-1)&8191))
#endm

#else
/* Here you could put C-specific definitions, like typedefs, C macros, prototypes etc... */

auto void RingBufCopyX(register __i2 u_int16 *d,
		      register __i0 const u_int16 *s,
		      register __a0 u_int16 n);
auto void RingBufCopyY(register __i2 __y u_int16 *d,
		      register __i0 const u_int16 *s,
		      register __a0 u_int16 n);
auto void RingBufCopyXfromY(register __i2 u_int16 *d,
			    register __i0 __y const u_int16 *s,
			    register __a0 u_int16 n);
auto void RingBufCopyYfromY(register __i2 __y u_int16 *d,
			    register __i0 __y const u_int16 *s,
			    register __a0 u_int16 n);


typedef struct usbpkt {
  u_int16 length;
  u_int16 payload[(ENDPOINT_SIZE_MAX+1)>>1];
} USBPacket;


extern struct USBVARS {
  /** Descriptor Pointer Table
      Members are:
      - *stringDescriptor0
      - *stringDescriptor1
      - *stringDescriptor2,
      - *stringDescriptor3,
      - *deviceDescriptor,
      - *configurationDescriptor

      For others than configurationDescriptor, descriptor size is
      first octet of descriptor. 
  */
  const u_int16 /*__y*/ *descriptorTable[16];

  /** Length of Configuration Descriptor. 
      (needed because configuration descriptor is actually
      a collection of many descriptors so the first octet
      does not specify length of the entire descriptor) */
  u_int16 configurationDescriptorSize;


  USBPacket /*__y*/ pkt;

  u_int32 totbytes; /*< total transferred bytes */

  /** Is an extra zero-length packet needed after transmission? */
  u_int16 ExtraZeroLengthPacketNeeded[4];

  /** Current USB Endpoint transmit buffer pointers */
  const /*__y*/ u_int16  *XmitBuf[4];

  /** Current USB Endpoints' bytes left to transmit */  
  u_int16 XmitLength[4];

  /** Is endpoint ready to transmit new block? */
  u_int16 EPReady[4];// = {1,1,1,1};

  u_int16 lastSofTimeout;

  u_int16 configuration;
  u_int16 interfaces;
  u_int16 lastSofFill; /*< Audio buf fullness at last SOF */
  u_int32 lastSofTime; /*< When last SOF was received */

  u_int16 setIndex; /*< If != 0, SET_CUR active */
  u_int16 setValue; /*< If != 0, SET_CUR active */

  u_int16 hidWriteCount;
} USB;


int USBStartTransmission(u_int16 ep, const void *buf, 
			 u_int16 length, u_int16 requestedLength);

void USBContinueTransmission(u_int16 ep);
void USBSingleStallEndpoint(register __c0 u_int16 ep);
void InitUSBDescriptors(u_int16 initDescriptors);
void InitUSB(u_int16 initDescriptors);
void USBResetEndpoint(register __c0 int ep);
u_int16 USBPacketReady();
u_int16 USBReceivePacket(USBPacket *packet);
void USBSendZeroLengthPacketToEndpoint(u_int16 endpoint);
void USBHandler();
void USBCheckForSetupPacket(void);
u_int16 USBXmitLeft(u_int16 endpoint);
void USBStallEndpoint(register __c0 int ep);
u_int16 SwapWord(register __a1 u_int16 d);

auto u_int16 USBIsAttached(void); /*Attach detected */
auto u_int16 USBIsDetached(void); /*Snapshot only, USBWantsSuspend() required*/
auto u_int16 USBWantsSuspend(void);


#define USB_MASS_STORAGE 1
#define USB_AUDIO 2

/** MSC */
void MSCPacketFromPC(/*__y*/ USBPacket *setupPacket);
u_int16 MscSendCsw(u_int16 status);
void DiskProtocolError(char errorcode);


#define AUDIO_DELAY_FRAMES 3 /* 3*44.1 = 132.3 samples */
#define AUDIO_DELAY_FRAMES_STR "\x03"
/** Must be provided */
void AudioPacketFromUSB(u_int16 *data, s_int16 words);

#endif
#endif
