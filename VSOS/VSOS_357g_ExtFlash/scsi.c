/// \file scsi.c SCSI operations

#include <string.h>
#include <vo_stdio.h>
#include <stdlib.h>
#include <vstypes.h>
#include <usblowlib.h>
#include <mapperflash.h>
//#include <minifat.h>
#include <audio.h>
#include <usb.h>
#include "msc.h"

void MyScsiTaskHandler();
extern u_int16 minifatBuffer[256];
void MyInitUSBDescriptors(u_int16 initDescriptors);
void MyScsiReset();
void MyMSCPacketFromPC(USBPacket *inPacket);
void 	  CheckLines(register __a0 u_int16 ep);
void USBContinueTransmission2(const u_int16 endpoint);
int USBStartTransmission2(const u_int16 endpoint, const void *buf, u_int16 length, u_int16 requestedLength);

#include "scsi.h"
#include "usblowlib.h"

//extern struct Fsmapper *map2;

#define DEBUG_LEVEL 0
#include "debuglib.h"
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
u_int16 ok2reset;
extern struct USBVARS *usb;
#define USBXmitLeft(e) (usb->XmitLength[(e)])
#define USB_EP_OUT0 (USB_BASE+8)

struct SCSIVARS {
  /// Current SCSI State
  SCSIStageEnum State;
  /// Current SCSI Status
  SCSIStatusEnum Status;
  /// Buffer Pointer for sending SCSI data out to the host
  u_int16 *DataOutBuffer;
  /// Bytes left for sending in the DATA OUT stage
  u_int16 DataOutSize;
  /// Generic SCSI data buffer
  u_int16 DataBuffer[32];
  /// Buffer Pointer for receiving SCSI data from the host
  u_int16 *DataInBuffer;
  /// Number of bytes received into the data in buffer
  u_int16 DataInSize;
  /// Number of disk blocks that need sending
  unsigned int BlocksLeftToSend;
  /// Number of disk blocks that need sending
  unsigned int BlocksLeftToReceive;
  /// Current disk sector number
  u_int32 CurrentDiskSector;
  u_int32 mapperNextFlushed;
  /** MSC variables */
  u_int16 cswPacket[7];  /*< command reply (Command Status Wrapper) data */
  u_int32 DataTransferLength; /*< what is expected by CBW */
  s_int32 Residue; /*< difference of what is actually transmitted by SCSI */
  s_int16 DataDirection;
} SCSI;

//u_int16 minifatBuffer2[256];

extern struct FsMapper usbMapper;
struct FsMapper *map2 = &usbMapper;

#define map ERRORORORO
//#define minifatBuffer ERROROOOR

USB_MEM_TYPE u_int16 * const send_map2[4] ={
  (USB_MEM_TYPE u_int16 *)(USB_SEND_MEM),
  (USB_MEM_TYPE u_int16 *)(USB_SEND_MEM+64),
  (USB_MEM_TYPE u_int16 *)(USB_SEND_MEM+128),
  (USB_MEM_TYPE u_int16 *)(USB_SEND_MEM+192)
};

u_int16 MyUSBReadLines(void);
u_int16 MyUSBWantsSuspend(void);
u_int16 MyUSBIsDetached(void);

enum SCSIStageEnum ScsiState2(void) {
  return SCSI.State;
}

#define BRESET_START (1<<6)
#define RESET_CHECK_COUNT 30

/**
 * Reset selected endpoint
 */
void MyUSBResetEndpoint(register __c0 int ep) {
  register u_int16 *p = (void *)(USB_EP_ST0 + (ep & 7));
  if (ep == 0) {
    /* control endpoint: both IN and OUT */
#if 0
    PERIP(p) = 0;
    PERIP(p) = USB_STF_IN_ENABLE;
#endif
  }
  if ((ep & 0x80)) {
    /* Input Endpoint (device writes to PC) */
    PERIP(p) &= ~USB_STF_IN_ENABLE;
    PERIP(p) |= USB_STF_IN_ENABLE;
  } else {
    /* Output endpoint (PC writes to device) */
    PERIP(p) &= ~USB_STF_OUT_ENABLE;
    PERIP(p) |=  USB_STF_OUT_ENABLE;
  }
}

u_int16 MyUSBReadLines(void) {
    if (PERIP(USB_CONFIG) & USB_CFF_MASTER) {
	return PERIP(USB_UTMIR)>>14;
    } else {
#if 0
	register u_int16 t = PERIP(USB_UTMIW), r;
	PERIP(USB_UTMIW) = 0x8001|(1<<3);
	PERIP(USB_CONFIG) |= USB_CFF_MASTER;
	PERIP(USB_UTMIR); /*delay*/
	r = PERIP(USB_UTMIR) >> 14;
	PERIP(USB_UTMIW) = t;
	PERIP(USB_CONFIG) &= ~USB_CFF_MASTER;
	return r;
#else
	return PERIP(USB_UTMIR)>>14;
#endif
    }
}

#define USB_SUSPEND 0x100
#define USB_RESUME  0x080

u_int16 MyUSBWantsSuspend(void) {
  if (PERIP(USB_STATUS) & USB_SUSPEND) {
	  PERIP(USB_STATUS) = USB_SUSPEND;
      return 1;
  }
  return 0;
}

void MyUSBResetStall(register __c0 int ep) {
  register u_int16 *p = (void *)(USB_EP_ST0 + (ep & 7));
#if DEBUG_LEVEL > 1
  puthex(ep);
  puts("=ep reset stall");
#endif
  if (ep & 0x80) {
    /* Input Endpoint (device writes to PC) */
    PERIP(p) &= ~USB_STF_IN_STALL;
  } else {
    /* Output endpoint (PC writes to device) */
    PERIP(p) &= ~USB_STF_OUT_STALL;
  }
}

void MyResetBulkEndpoints(void) {
  PERIP(USB_EP_ST0 + MSC_BULK_OUT_ENDPOINT) = 0;
  PERIP(USB_EP_ST0 + MSC_BULK_IN_ENDPOINT) = 0;
  PERIP(USB_EP_ST0 + MSC_BULK_OUT_ENDPOINT) =
    USB_STF_OUT_BULK | USB_STF_OUT_ENABLE | USB_STF_OUT_EP_SIZE * 3;
  PERIP(USB_EP_ST0 + MSC_BULK_IN_ENDPOINT) =
    USB_STF_IN_BULK | USB_STF_IN_ENABLE;

  usb->EPReady[MSC_BULK_IN_ENDPOINT] = 1;
  usb->EPReady[MSC_BULK_OUT_ENDPOINT] = 1;
}

u_int16 USBIsEndpointStalled2(register int ep) {
  register u_int16 *p = (void *)(USB_EP_ST0 + (ep & 7));
  if (ep & 0x80) {
    /* Input Endpoint (device writes to PC) */
    return PERIP(p) & USB_STF_IN_STALL;
  } else {
    /* Output endpoint (PC writes to device) */
    return PERIP(p) & USB_STF_OUT_STALL;
  }
}

u_int16 MyUSBIsDetached(void) {
   u_int16 x = MyUSBReadLines();
   return (MyUSBReadLines() == 3); /* is detached if both DN or DP are high */
}

void MyUSBSendZeroLengthPacketToEndpoint0(void) {

  PERIP(USB_STATUS) = TX_INT;
  while (USBStartTransmission2(0, 0, 0, 1))
    ;
#if 1
  while (!(PERIP(USB_EP_ST0) & USB_STF_IN_EMPTY)) {
    if (PERIP(USB_UTMIR) & 4) {
      /*puts("BRST");*/
      break;
    }
  }
#else
  // Wait for ack();
  while (!(PERIP(USB_STATUS) & TX_INT)) { /** \bug: allows any IN */
    if (PERIP(USB_STATUS) & 0x8000U) {
      /*puts("BRST");*/
      break;
    }
  }
#endif
  PERIP(USB_STATUS) = TX_INT;

  USBContinueTransmission2(0);
}



void USBContinueTransmission2(const u_int16 endpoint) {
	/** Send next packet to endpoint fifo. */
	register u_int16 n = usb->XmitLength[endpoint];
	static int map2 = 0;
	static int toggle=0;
	//See if endpoint is ready to accept new packet
	/// \todo streamline!

	if (!(PERIP(USB_EP_ST0 + endpoint) & USB_STF_IN_EMPTY))
	return;

	
	// 0 bytes left to send, ZLP not needed
	if (n == 0 && usb->ExtraZeroLengthPacketNeeded[endpoint] == 0) {
		//transmission is complete, release EP for next write
		usb->EPReady[endpoint] = 1;
		return;
	}
  	
	if (n >= ENDPOINT_SIZE_0) {
		n = ENDPOINT_SIZE_0;
	} else {
		//If we send less-than-max size packet, zlp not needed.
		usb->ExtraZeroLengthPacketNeeded[endpoint] = 0;
		/* Releases it early! The last part is still being sent. */
		usb->EPReady[endpoint] = 1;
	}
	
	usb->XmitLength[endpoint] -= n;

#if 0
	if (!PERIP(USB_STATUS)&USB_ST_NAK)
	  return;
	  
	PERIP(USB_STATUS) = USB_ST_NAK;
#endif
	
	if (endpoint == MSC_BULK_IN_ENDPOINT) {
	  u_int16 x;
	  volatile u_int16 delay;

	  if (toggle) {
	    map2 = 2;
	    toggle = 0;
	    x =  (endpoint<<10);
	  } else {
	    map2 = 3;
	    toggle = 1;
	    x =  ((endpoint+1)<<10);    
	  }
	 

	  delay = 0;
	  PERIP(INT_GLOB_DIS) = 1;

	  memcpyXY(send_map2[map2], usb->XmitBuf[endpoint], (n+1)>>1);
	  
#if 0
	  while (PERIP(USB_EP_ST0+MSC_BULK_IN_ENDPOINT) & 2) {
	    if (delay++ > 100) {
	      break;
	    }
	    ;
	  }
	  PERIP(USB_EP_ST0+MSC_BULK_IN_ENDPOINT) &= ~2;
#endif      
	  
	  PERIP(USB_EP_SEND0 + endpoint) = (0x8000U | x | n);

	} else {
	  memcpyXY(send_map2[endpoint], usb->XmitBuf[endpoint], (n+1)>>1);
	  PERIP(USB_EP_SEND0 + endpoint) = (0x8000U | (endpoint<<10) | n);

	}     

	usb->XmitBuf[endpoint] += ((n+1)>>1);	

	PERIP(INT_GLOB_ENA) = 1;

#if 0
	{
	  
	  u_int16 addr;
  
#if 0
	  while (PERIP(USB_EP_SEND0 + endpoint) & 0x8000U);
	  if (endpoint != 0) {
	    PERIP(GPIO2_ODATA) |= 1;
	  }
#endif

#if 1

	  PERIP(INT_GLOB_DIS) = 1;
	  CheckLines(endpoint);
	  PERIP(INT_GLOB_ENA) = 1;
#else
	  addr = USB_EP_ST0 + endpoint;
	  while (! (PERIP(addr)&USB_STF_IN_EMPTY)) {
	    if (PERIP(USB_STATUS) & BRESET_START) {
	      PERIP(GPIO2_ODATA) |= 1;	      
	      break;
	    }
	  }
#endif
	}

	// PERIP(GPIO2_ODATA) &= ~3;
#endif
	
}

int USBStartTransmission2(const u_int16 endpoint, const void *buf, u_int16 length, u_int16 requestedLength) {  
  if (!usb->EPReady[endpoint]) {
    return 1;    
  }
  /* Is required! USBContinueTransmission2() releases EPReady early.
     ep should be ready for new packet but it's not. */
  if (endpoint == MSC_BULK_IN_ENDPOINT) {
    if (!(PERIP(USB_EP_ST0 + MSC_BULK_IN_ENDPOINT) & USB_STF_IN_EMPTY)) {
      return 1;
    }
  }
  usb->EPReady[endpoint] = 0;
  usb->XmitBuf[endpoint] = buf;
  usb->XmitLength[endpoint] = MIN(length, requestedLength); 
  usb->ExtraZeroLengthPacketNeeded[endpoint] = 0;  
  if (requestedLength > length) {
    //A zero-length packet may be needed
    usb->ExtraZeroLengthPacketNeeded[endpoint] = 1;
  }
  USBContinueTransmission2(endpoint); //Call transmitter once
  return 0;
}



void USBSingleStallEndpoint2(register __c0 u_int16 ep) {
  register u_int16 addr = USB_EP_ST0 + (ep & 7);
  register u_int16 flags = USB_STF_IN_STALL_SENT|USB_STF_OUT_STALL_SENT;
  if (ep & 0x80) {
    flags = USB_STF_IN_STALL_SENT;
  } else if (ep) {
    flags = USB_STF_OUT_STALL_SENT;
  }
  // Clear old stall info bits
  PERIP(addr) &= ~flags;
  // Set endpoint stall bits
  PERIP(addr) |= (flags<<1);
  // Wait for sent stall
  while (!(PERIP(addr) & flags))
    ;
  // Clear stall bits
  PERIP(addr) &= ~(flags<<1);
}

void MyUSBStallEndpoint(register __c0 int ep) {
  register u_int16 *p = (void *)(USB_EP_ST0 + (ep & 7));
  if (ep & 0x80) {
    /* Input Endpoint (device writes to PC) */
    PERIP(p) |= USB_STF_IN_STALL;
  } else {
    /* Output endpoint (PC writes to device) */
    PERIP(p) |= USB_STF_OUT_STALL;
  }
}

#define USB_DEVICE_DESCRIPTOR_INDEX 0
#define USB_CONFIGURATION_DESCRIPTOR_INDEX 1
#define USB_STRING_DESCRIPTOR_START 2
#define USB_REQUEST_MAX_LUN 254

void RealDecodeSetupPacket2(void) {
  u_int16 tempbuf[2];
  register __y u_int16 rqLength = SwapWord(usb->pkt.payload[USB_SP_LENGTH]);

  //EP0 shalt not be transmitting now.
  usb->EPReady[0] = 1; //EP0 shalt be ready now.
    

  // Mandatory Chapter 9 code starts here
  //Check SETUP xaction direction
  if ((s_int16)usb->pkt.payload[USB_SP_REQUEST] < 0) {

    // GET type setup packet (datastage: device->pc)
    switch (usb->pkt.payload[USB_SP_REQUEST] & 0xff) {
    case USB_REQUEST_GET_DESCRIPTOR:     
 
      switch (usb->pkt.payload[USB_SP_VALUE] & 0xff) { //descriptorType

      case USB_DEVICE_DESCRIPTOR_TYPE:

 	USBStartTransmission2(0, usb->descriptorTable[USB_DEVICE_DESCRIPTOR_INDEX],
			     usb->descriptorTable[USB_DEVICE_DESCRIPTOR_INDEX][0]>>8, rqLength);
	break;


      case USB_CONFIGURATION_DESCRIPTOR_TYPE:

 	USBStartTransmission2(0, usb->descriptorTable[USB_CONFIGURATION_DESCRIPTOR_INDEX],
			     usb->configurationDescriptorSize, rqLength);
	break;

      case USB_STRING_DESCRIPTOR_TYPE:
	{
	  register u_int16 r = usb->pkt.payload[USB_SP_VALUE] >> 8;
	  if (r < 5) {
	    USBStartTransmission2(0, usb->descriptorTable[r+USB_STRING_DESCRIPTOR_START],
				 usb->descriptorTable[r+USB_STRING_DESCRIPTOR_START][0]>>8, rqLength);
	    
	  } else {
	    goto unknown;
	    //USBSingleStallEndpoint2(0); //Unknown string descriptor
	  }
	}
	break;

      }       
      break; // break of USB_REQUEST_GET_DESCRIPTOR

    case USB_REQUEST_GET_STATUS:
      tempbuf[0] = 0; //Must send reply, so
      //00 00 shalt be a valid reply.
      //      if ((usb->pkt.payload[0]>>8) == (0x80|USB_ENDPOINT_REQUEST))
      if ((usb->pkt.payload[0] & 0xff00U) == ((0x80|USB_ENDPOINT_REQUEST)<<8))

	{
	if (USBIsEndpointStalled2(SwapWord(usb->pkt.payload[USB_SP_INDEX]))) {
	  tempbuf[0] = 0x0100; /* endpoint halted */
	}
      }
      USBStartTransmission2(0, tempbuf, 2, 2);
      break;      

    case USB_REQUEST_MAX_LUN:
      tempbuf[0] = 0;
      goto sendone;
      
    case USB_REQUEST_GET_CONFIGURATION:
      tempbuf[0] = usb->configuration << 8;
      goto sendone;

    case USB_REQUEST_GET_INTERFACE:
      /* must return both current and alternate setting */
      /* value = 0 */
      /* index = interface number */
      // if ((usb->pkt.payload[USB_SP_INDEX] >> 8) == (usb->interfaces >> 8))
      tempbuf[0] = usb->interfaces; /* return alternate setting */
    sendone:
      USBStartTransmission2(0, tempbuf, 1, 1);
      break;

    default:
      goto unknown;
      //USBSingleStallEndpoint2(0); //Unknown request
    }
    
  } else {
    
    // SET type setup packet (datastage: pc->device)
    
    switch (usb->pkt.payload[USB_SP_REQUEST] & 0xff) {
      
    case USB_REQUEST_SET_ADDRESS:
    {
	register __y u_int16 addr = (usb->pkt.payload[USB_SP_VALUE] >> 8);
	MyUSBSendZeroLengthPacketToEndpoint0();
	PERIP(USB_CONFIG) |= addr & 0x7f;
    }
      break;
      
    case USB_REQUEST_SET_CONFIGURATION:
      usb->configuration = (usb->pkt.payload[USB_SP_VALUE] >> 8);
      MyResetBulkEndpoints();
      MyUSBSendZeroLengthPacketToEndpoint0();
      PERIP(USB_CONTROL) |= 1; /* configured? */
      break;
      
    case USB_REQUEST_SET_INTERFACE:
      MyUSBResetEndpoint(0);
      /* index = interface, value = alternate setting */
      usb->interfaces =
      	(usb->pkt.payload[USB_SP_VALUE] & 0xff00U) |
      	(usb->pkt.payload[USB_SP_INDEX] >> 8);
      MyResetBulkEndpoints();
    sendzero:
      MyUSBSendZeroLengthPacketToEndpoint0(); 
      break;

    case USB_REQUEST_CLEAR_FEATURE:
      if ((usb->pkt.payload[0]>>8) == USB_ENDPOINT_REQUEST
	  && usb->pkt.payload[USB_SP_VALUE] == 0/*no swap for 0 needed*/) {
	register u_int16 myep = SwapWord(usb->pkt.payload[USB_SP_INDEX]);
	/* Clear Feature clears data toggles and stalls */

	/* IN packet waiting in the endpoint? */
	if ((myep & 0x80)/*IN*/ &&
	    !(PERIP(USB_EP_ST0 + (myep & 7)) & USB_STF_IN_EMPTY)) {
	  MyUSBResetEndpoint(myep);
	  /* Re-arm the IN that was waiting.. */
	  PERIP(USB_EP_SEND0 + (myep & 7)) |= 0x8000U;
	} else {
	  MyUSBResetEndpoint(myep);
	}
	/* if SCSI not in consistent state, keep it stalled */
	if (ScsiState2() == SCSI_INVALID_CBW &&
	    (myep == (0x80|MSC_BULK_IN_ENDPOINT) ||
	     myep == (0x00|MSC_BULK_OUT_ENDPOINT))) {
	  /* wait until BOT reset.. */
	  MyUSBStallEndpoint(myep);
	}
	goto sendzero;
      } else {
	goto unknown;
      }
      break;

    case USB_REQUEST_SET_FEATURE:
      if ((usb->pkt.payload[0] & 0xff00U) == (USB_ENDPOINT_REQUEST<<8))
	{
	/* index=ep/interface  value&1 = HALT */

	/* no swap needed for 0 */
	if (usb->pkt.payload[USB_SP_VALUE] == 0) {
	  MyUSBStallEndpoint(SwapWord(usb->pkt.payload[USB_SP_INDEX]));
	  goto sendzero;
	  //MyUSBSendZeroLengthPacketToEndpoint0();
	  //break;
	}
      }

      goto unknown;
      //USBSingleStallEndpoint2(0); //Unknown
      //break;

    case 0xFF:
      if (usb->pkt.payload[USB_SP_REQUEST] == 0x21ff &&
	  (usb->pkt.payload[USB_SP_VALUE] | usb->pkt.payload[USB_SP_INDEX])
	  == 0x0000 /* no swap needed for 0's */) {
	/*BOT requesttype: 0x21 request: 0xff value: 0 index: interface */

	MyScsiReset();

	MyUSBResetStall(0x80|MSC_BULK_IN_ENDPOINT);
	MyUSBResetStall(0x00|MSC_BULK_OUT_ENDPOINT);
	goto sendzero;
	//MyUSBSendZeroLengthPacketToEndpoint0();
	//break;
      }
      /* (if not BOT reset) */
      goto unknown;
      
    default:
    unknown:
      USBSingleStallEndpoint2(0); //Unknown request
    }
  }  
}



void RealDecodeSetupPacket2(void);

#define USB_SP_REQUEST 0
#define USB_SP_VALUE 1
#define USB_SP_INDEX 2
#define USB_SP_LENGTH 3


u_int16 MyUSBReceivePacket(USBPacket /*__y*/ *packet) {
  register __c0 u_int16 st;
  register __c1 u_int16 ep;
  register __b0 u_int16 tmp;
  register __b1 u_int16 lenW;
  
  /*
   * There should be now some data waiting at receive buffer
   */
  st = USB_MEM(PERIP(USB_RDPTR) + USB_RECV_MEM);
  ep = (s_int16)(st & 0x3C00) >> 10;
  st &= 0x03FF;
  packet->length = st;
  usb->totbytes += st;

  lenW = (st + 1)>>1;
  tmp = (PERIP(USB_RDPTR) + 1) & 0x03FF; 

  if (lenW <= (ENDPOINT_SIZE_MAX+1)>>1) {
    RingBufCopyXfromY(packet->payload, (USB_MEM_TYPE void *)(tmp + USB_RECV_MEM), lenW);
    PERIP(USB_RDPTR) = (lenW + tmp) & 0x03FF;
  }
  PERIP(USB_STATUS) = (RX_INT|SETUP_INFO);
  return ep & 0x7f;
}




void MyInitUSB(u_int16 initDescriptors) {
  /* Give clock to the high-speed parts so they will reset properly. */
  {

    PERIP(ANA_CF2) |= ANA_CF2_REF_ENA | ANA_CF2_UTM_ENA | ANA_CF2_2G_ENA;
    // PERIP(ANA_CF3) |= ANA_CF3_480_ENA | (1<<14);
    PERIP(ANA_CF3) |= 0xC000;
    PERIP(ANA_CF3); //Wait
    PERIP(ANA_CF3); //Wait
    PERIP(ANA_CF3);

    PERIP(USB_CONFIG) = USB_CFF_RESET|USB_CFF_ENABLE|USB_CFF_NOHIGHSPEED;
    //Restore whatever settings there were before */

    PERIP(USB_CONFIG) = USB_CFF_ENABLE|USB_CFF_NOHIGHSPEED;
  }
  
  if (initDescriptors) {
      MyInitUSBDescriptors(initDescriptors);
      memsetY((USB_MEM_TYPE void *)USB_SEND_MEM, 0x0000, 1024);
  }

  /* reset USB */
  usb->totbytes = 0;
  usb->configuration = 0;
  usb->interfaces = 0;
  usb->lastSofTime = 0;
  usb->setValue = 0;
  usb->hidWriteCount = 0;

  {
    register u_int16 i;
    for (i=0; i < 4; i++) {
      usb->XmitLength[i] = 0;
      usb->ExtraZeroLengthPacketNeeded[i] = 0;
    }  
  }

  PERIP(USB_CONTROL) = SOF_INT; //same as USB_STF_SOF;
  PERIP(INT_ENABLE0_HP) |= INTF_USB;

  usb->EPReady[0] = usb->EPReady[1] = 1;
  /*
   * Control endpoint
   */
  PERIP(USB_EP_ST0) =
    USB_STF_OUT_BULK | USB_STF_OUT_ENABLE | USB_STF_OUT_EP_SIZE*3 |
    USB_STF_IN_BULK | USB_STF_IN_ENABLE;

  MyResetBulkEndpoints();
  
  /* UTMI handles pull-ups automatically */
  PERIP(ANA_CF2) |= ANA_CF2_UTM_ENA;
  MyScsiReset();
}

u_int16 MyUSBIsDetached(void);

#define XXX (1<<8)


void MyUSBHandler() {
  __y u_int16 ep,i;
  
  
  if (PERIP(USB_STATUS) & BRESET_START) {
    PERIP(USB_STATUS) = BRESET_START;
    for (i=0; i < RESET_CHECK_COUNT; i++) {
      if ((PERIP(USB_UTMIR)>>14) != 0)
	break;
    }
    if (i == RESET_CHECK_COUNT) {
      ok2reset = 1;
    } else {
      ok2reset = 0;
    }
  }
 
  if (PERIP(USB_STATUS) & BRESET_INT) {
    PERIP(USB_STATUS) = BRESET_INT;
    if (ok2reset) {
      MyInitUSB(0);
      printf("BRST\n");
    } else {
      //printf(".");
    }
  }
  
  if (PERIP(USB_STATUS) & USB_STF_SOF) {
    PERIP(USB_STATUS) = USB_STF_SOF; /** clear SOF */
  }
    
  if (usb->EPReady[0] == 0) {
    USBContinueTransmission2(0);
  }
  
  if (usb->EPReady[MSC_BULK_IN_ENDPOINT] == 0) {
    USBContinueTransmission2(MSC_BULK_IN_ENDPOINT);
  }

  // Are there packet(s) in the receive buffer?
  if (PERIP(USB_RDPTR) != PERIP(USB_WRPTR)) {
    ep = MyUSBReceivePacket(&usb->pkt);
    
    if (usb->pkt.length) { //Does it actually contain data?
      /* This order prevents wrong SETUP detection when isochronous
	 packets are received in the middle of transactions.. */
      if (ep == AUDIO_ISOC_OUT_EP) {
	AudioPacketFromUSB(usb->pkt.payload, (usb->pkt.length+1)/2);
      } else if (ep==0 || usb->setValue) {
	RealDecodeSetupPacket2();	
      } else if (ep == MSC_BULK_OUT_ENDPOINT) {
	/*Real*/MyMSCPacketFromPC(&usb->pkt);
      }
    } 
  }
 
  MyScsiTaskHandler();
}


/// Handle any pending SCSI operations
void MyScsiTaskHandler() {
	
	/*
	if (ReadTimeCount() >= SCSI.mapperNextFlushed) {
		SCSI.mapperNextFlushed = -1;
		if (map2->Flush)
		map2->Flush(map2, 1);
	}
	*/
	switch (SCSI.State) {
	
		case SCSI_DATA_TO_HOST:
		//printf("Xmit(%d)",SCSI.DataOutSize);
		if (!USBStartTransmission2(MSC_BULK_IN_ENDPOINT,SCSI.DataOutBuffer,
		SCSI.DataOutSize, SCSI.DataOutSize)) {
			// EP1 was ready to transmit
			SCSI.State = SCSI_TRANSMITTING;
			SCSI.Residue -= SCSI.DataOutSize;
			//printf("OK        \n");
		} else {
			//printf("NotReady ");
		}
		break;
	
		case SCSI_TRANSMITTING:
		if (USBXmitLeft(MSC_BULK_IN_ENDPOINT) == 0) {
			if (SCSI.BlocksLeftToSend == 0) {
				SCSI.State = SCSI_SEND_STATUS;
			} else {
				SCSI.BlocksLeftToSend--;
				SCSI.Status = SCSI_OK;
				PERIP(GPIO1_ODATA) |= (1<<3);//LED2 = SO;
	
				if (map2->Read &&
				map2->Read(map2, SCSI.CurrentDiskSector, 1, minifatBuffer) != 1) {
					/* 0 also when does not exist, so do not send error.. */
					//SCSI.Status = SCSI_REQUEST_ERROR; //read failed
	
				}
	
				SCSI.CurrentDiskSector++;
				SCSI.DataOutBuffer = minifatBuffer;
				SCSI.DataOutSize = 512;
				SCSI.State = SCSI_DATA_TO_HOST;
			}
		} else {
			USBContinueTransmission2(MSC_BULK_IN_ENDPOINT);
		}
		break;
	
		case SCSI_SEND_STATUS:
		if (!MscSendCsw(SCSI.Status)) {
			SCSI.State = SCSI_READY_FOR_COMMAND;
		} else {
			//puts("not able to xmit status yet");
		}
		break;
	}
}
	
	
u_int32 ScsiLbab(register __i0 u_int16 *res__lbab3) {
  return ((u_int32)res__lbab3[0] << 24) 
      | ((u_int32)res__lbab3[1] << 8)
      | ((res__lbab3[2] >> 8) & 0x00ff);
}



/// SCSI Inquiry (0x12) Command Starter
void ScsiInquiry (register ScsiInquiryCdb *cmd) {
  SCSI.DataOutBuffer = SCSI.DataBuffer;
  //SCSI.Status = SCSI_OK;
  SCSI.State = SCSI_DATA_TO_HOST;

  //Data Buffer is cleared when we come here    
  if ( (cmd->flags__pageCode & 0x01ff) == 0x0180) {
    //Extended page: Serial Number page requested
    //See SCSI2 chapter 8.3.4.5

    SCSI.DataBuffer[0] = 0x0080; //Peripheral qualifier,type | page code
    SCSI.DataBuffer[1] = 0x0001; // Response Format is 1 (Common Command Set)
    SCSI.DataBuffer[2] = 0x2700;

    SCSI.DataBuffer[4] =  ('V'<<8)|'L';
    SCSI.DataBuffer[5] =  ('S'<<8)|'I';

    SCSI.DataBuffer[8] =  ('V'<<8)|'S';
    SCSI.DataBuffer[9] =  ('1'<<8)|'0';
    SCSI.DataBuffer[10] = ('0'<<8)|'5';
    SCSI.DataBuffer[11] = ('-'<<8)|'1';
    
    SCSI.DataBuffer[16] = ('1'<<8)|'.';
    SCSI.DataBuffer[17] = ('4'<<8)|' ';

#if 1
    SCSI.DataBuffer[20] = 0x3130; /* "10001014" */
    SCSI.DataBuffer[21] = 0x3030;
    SCSI.DataBuffer[22] = 0x3130;
    SCSI.DataBuffer[23] = 0x3134;
#endif

    SCSI.DataOutSize = 36;
   
  } else if (cmd->flags__pageCode & 0x0100) {
    //Some other extended page was requested
    SCSI.Status = SCSI_REQUEST_ERROR;
    SCSI.State = SCSI_SEND_STATUS;
    USBSingleStallEndpoint2(0x80|MSC_BULK_IN_ENDPOINT);
  } else {
      // Default page was requested
      SCSI.DataBuffer[0] = 0x0080; // RMB=1 : Removable Device 
      SCSI.DataBuffer[1] = 0x0001; // Response Format is 1 (Common Command Set)
      SCSI.DataBuffer[2] = 0x1f00;

      SCSI.DataBuffer[4] =  ('V'<<8)|'L';
      SCSI.DataBuffer[5] =  ('S'<<8)|'I';

      SCSI.DataBuffer[8] =  ('V'<<8)|'S';
      SCSI.DataBuffer[9] =  ('1'<<8)|'0';
      SCSI.DataBuffer[10] = ('0'<<8)|'5';
      SCSI.DataBuffer[11] = ('-'<<8)|'1';

      SCSI.DataBuffer[16] = ('1'<<8)|'.';
      SCSI.DataBuffer[17] = ('3'<<8)|' ';
      SCSI.DataOutSize = 36; 
  }
  if (cmd->allocationLength < SCSI.DataOutSize) {
    SCSI.DataOutSize = cmd->allocationLength;
  }
}


/** This gets called when there is an incoming packet that's 
    not a Commnd Block Wrapper.
    It should only be disk block data.    
 */

void DiskDataReceived(int length, u_int16 *datablock){
	
	//Are we not expecting data from host?
	if (SCSI.State != SCSI_DATA_FROM_HOST)
	return; /* Ignore any data blocks when not expected */
	
	SCSI.Residue    -= length;
	SCSI.DataInSize += length;
	
	//Read data to diskbuffer (buf was set by command phase)
	memcpy(SCSI.DataInBuffer, datablock, length/2);
	SCSI.DataInBuffer += length/2;
	
	if (SCSI.DataInSize >= 512) { //A blockful of data received
		if (SCSI.Status == SCSI_OK) {
			if (map2->Write(map2, SCSI.CurrentDiskSector, 1, minifatBuffer) != 1) {
				SCSI.Status = SCSI_REQUEST_ERROR; //write failed
			}
		}
	
		SCSI.CurrentDiskSector++;
		// Reset input buffer pointer
		SCSI.DataInBuffer = minifatBuffer;
		SCSI.DataInSize = 0;
		SCSI.BlocksLeftToReceive--;
	
		//Was this the last block that we expected?
		if (SCSI.BlocksLeftToReceive == 0) {
			SCSI.State = SCSI_SEND_STATUS;
		}
	
	}//A blockful of data received
}
	
extern u_int16 ScsiIdle;
 

///Handle a protocol command
void DiskProtocolCommand(u_int16 *cmd) {
  __y int length = (cmd[0]>>8) & 0x1f;
  static u_int16 oldcmd,ocmd;
  static u_int16 opcount = 0;

  SCSI.BlocksLeftToSend = 0;
  SCSI.BlocksLeftToReceive = 0;
  /* Default residue size. */
  SCSI.Residue = SCSI.DataTransferLength;
  /* Default result */
  SCSI.Status = SCSI_OK;

  //Clear SCSI data buffer
  memset(SCSI.DataBuffer, 0, 32);

  oldcmd = ocmd;
  ocmd = (cmd[OPERATION_CODE] & 0x00ff);

  switch (cmd[OPERATION_CODE] & 0x00ff) {


  case ATAPI_READ_FORMAT_CAPACITIES:
    {
      u_int32 diskSizeBlocks = (map2->blocks-1) / geometry.sectorsPerBlock;
      //Data Buffer is cleared when we come here    
      SCSI.DataBuffer[1] = 8; //length
      SCSI.DataBuffer[2] = diskSizeBlocks >> 16;
      SCSI.DataBuffer[3] = diskSizeBlocks;
      SCSI.DataBuffer[4] = 0x0200; // formatted media, current capacity
      SCSI.DataBuffer[5] = 512 * geometry.sectorsPerBlock;
      SCSI.DataOutBuffer = SCSI.DataBuffer;
      SCSI.DataOutSize = 12;
      //SCSI.Status = SCSI_OK;
      SCSI.State = SCSI_DATA_TO_HOST;
    }
    break;
	
  case SCSI_INQUIRY:
    ScsiInquiry((ScsiInquiryCdb *)cmd);
    break;

  case SCSI_READ_CAPACITY_10:
    {
      u_int32 diskSizeSectors = (map2->blocks-1) / geometry.sectorsPerBlock;
      SCSI.DataOutBuffer = SCSI.DataBuffer;
      //Data Buffer is cleared when we come here
      SCSI.DataBuffer[0] = diskSizeSectors >> 16;
      SCSI.DataBuffer[1] = diskSizeSectors;
      SCSI.DataBuffer[3] = 512 * geometry.sectorsPerBlock;
      SCSI.DataOutSize = 8;
      //SCSI.Status = SCSI_OK;
      SCSI.State = SCSI_DATA_TO_HOST;
    }

    break;
    
  case SCSI_READ_10:
    ScsiIdle = 0;
    SCSI.CurrentDiskSector = geometry.sectorsPerBlock * ScsiLbab(&((ScsiRead10Cdb*)cmd)->res__lbab3);
    SCSI.BlocksLeftToSend = geometry.sectorsPerBlock * ((ScsiRead10Cdb*)cmd)->wLength;
    SCSI.Residue = (s_int32) SCSI.BlocksLeftToSend * 512;
    //SCSI.Status = SCSI_OK;
    //printf("SCSI RD g=%d,%ld,%d..",geometry.sectorsPerBlock,SCSI.CurrentDiskSector,SCSI.BlocksLeftToSend);
    SCSI.State = SCSI_TRANSMITTING;


    if (SCSI.DataDirection >= 0) {
      /* Wrong data direction */
      MyUSBStallEndpoint(0x00|MSC_BULK_OUT_ENDPOINT);
      goto stallIn;
      //USBStallEndpoint(0x80|MSC_BULK_IN_ENDPOINT);
      //SCSI.Status = SCSI_REQUEST_ERROR;
      //SCSI.State = SCSI_SEND_STATUS;
    }
    if (SCSI.BlocksLeftToSend == 0) {
      goto requestError;
      //SCSI.Status = SCSI_REQUEST_ERROR;
      //SCSI.State = SCSI_SEND_STATUS;
    }
    break;

  case SCSI_MODE_SENSE_6:
  case SCSI_MODE_SENSE_10: /* if ModeSense10() not implemented, iMac fails.. */
    SCSI.DataOutBuffer = SCSI.DataBuffer;
    SCSI.DataOutSize = 4; /* Windows expects 12 bytes for ModeSense6 */
    //SCSI.Status = SCSI_OK;
    SCSI.State = SCSI_DATA_TO_HOST;   
    break;

  case SCSI_REQUEST_SENSE:
    //Data Buffer is cleared when we come here
    SCSI.DataOutBuffer[0] = 0x7000;           //Response code: Current, Fixed.
    //The command must always return one sense key in this fixed location:
    SCSI.DataOutBuffer[1] = SK_NO_SENSE << 8; //See SPC-4 4.5.6 for Sense Keys
    SCSI.DataOutBuffer[3] = 0;                //No additional sense data

    SCSI.DataOutBuffer = SCSI.DataBuffer;
    SCSI.DataOutSize = 18;//8; /* Windows requires 18. */
    //SCSI.Status = SCSI_OK;
    SCSI.State = SCSI_DATA_TO_HOST;
    break;    

      
  case SCSI_PREVENT_ALLOW_MEDIUM_REMOVAL:
#if 0
    if (oldcmd == SCSI_TEST_UNIT_READY) {
      if ((ScsiIdle&1) && cmd[2]&1) {
	ScsiIdle++;
	printf("Prevent removal");
      } else if ((ScsiIdle&0x1) == 0 && (cmd[2]&1) == 0) {
	printf("Allow removal");
	ScsiIdle++;
      }
    } else {
      ScsiIdle=0;
    }
#else
    if (oldcmd == SCSI_START_STOP_UNIT && (cmd[2]&1) == 0) {
      //printf("Allow removal");
      ScsiIdle = 10;
    } else {
      ScsiIdle = 0;
    }
#endif
    SCSI.State = SCSI_SEND_STATUS;   
    break;

  case SCSI_START_STOP_UNIT: /* 0x1b */
    /* 0:OP 1:immed0 2:res 3:res 4:power7-4 res3-2 loej1 start0 5:control */
    // Linux send stop unit when device ejected
    if ((cmd[2]&0x03) == 2) {
      //printf("NOW YOU CAN EJECT DEVICE %X %X %X\n",cmd[1],cmd[2],cmd[3]);      
#if 0
      if (oldcmd == SCSI_PREVENT_ALLOW_MEDIUM_REMOVAL) {
	ScsiIdle = 10;
      }
#endif
    }
    SCSI.State = SCSI_SEND_STATUS;   
    break;
    

  case SCSI_SYNCHRONIZE_CACHE:
    //printf("\nSCSI SYNCHRONIZE CACHE\n");
    ScsiIdle = 0;
    if (map2->Flush)
	map2->Flush(map2, 1);
    SCSI.mapperNextFlushed = -1;
    /* Fall through! */
    /* vvvvvvvvvvvvv */
  case SCSI_TEST_UNIT_READY:
  case SCSI_VERIFY: /** \todo: check media size */
  case SCSI_MODE_SELECT:
    //SCSI.Status = SCSI_OK;
    SCSI.State = SCSI_SEND_STATUS;   
    break;

  case SCSI_WRITE_10:    
    ScsiIdle = 0;
    SCSI.CurrentDiskSector = geometry.sectorsPerBlock * ScsiLbab(&((ScsiWrite10Cdb *)cmd)->flags__lbab3);
    SCSI.BlocksLeftToReceive = geometry.sectorsPerBlock * ((ScsiWrite10Cdb *)cmd)->wLength;
    SCSI.DataInBuffer = minifatBuffer;
    SCSI.DataInSize = 0;
    //SCSI.Status = SCSI_OK;    
    SCSI.State = SCSI_DATA_FROM_HOST;
    if (SCSI.DataDirection < 0) {
      /* Wrong data direction */
      /*USBStallEndpoint(0x80|MSC_BULK_IN_ENDPOINT);*/
      MyUSBStallEndpoint(0x00|MSC_BULK_OUT_ENDPOINT);
      goto requestError;
      //SCSI.Status = SCSI_REQUEST_ERROR;
      //SCSI.State = SCSI_SEND_STATUS;
    }
    if (SCSI.BlocksLeftToReceive == 0) {
      goto requestError;
      //SCSI.Status = SCSI_REQUEST_ERROR;
      //SCSI.State = SCSI_SEND_STATUS;
    }
    break;

  default: //unknown command
//      USBStallEndpoint(0x00|MSC_BULK_OUT_ENDPOINT);
    goto stallIn;
    //USBStallEndpoint(0x80|MSC_BULK_IN_ENDPOINT);
    //SCSI.Status = SCSI_REQUEST_ERROR;
    //SCSI.State = SCSI_SEND_STATUS;

    break;
  } //end switch (operation code)
  //puthex(ScsiState2);


  /*
    TRANSMITTING:   ready to send file data
    + ok: SCSI.BlocksLeftToSend > 0, SCSI.DataDirection < 0
    DATA_TO_HOST:   ready to send command result data
    + ok: SCSI.DataOutSize > 0, SCSI.DataDirection < 0
    DATA_FROM_HOST: ready to receive file data
    + ok: SCSI.BlocksLeftToReceive > 0, SCSI.DataDirection >= 0
    READY_FOR_COMMAND: waiting for command
    + ok:
   */
  if (SCSI.State == SCSI_DATA_TO_HOST) {
    /* Windows does not like it increased! */
    if (SCSI.DataOutSize > SCSI.DataTransferLength)
	SCSI.DataOutSize = SCSI.DataTransferLength;

    if (SCSI.DataDirection >= 0) {
      /* Wrong data direction */
      /*USBStallEndpoint(0x00|MSC_BULK_OUT_ENDPOINT);*/
      goto stallIn;
      //USBStallEndpoint(0x80|MSC_BULK_IN_ENDPOINT);
      //SCSI.Status = SCSI_REQUEST_ERROR;
      //SCSI.State = SCSI_SEND_STATUS;
    }
    if (SCSI.DataOutSize == 0) {
      goto sendStatus;
      //SCSI.State = SCSI_SEND_STATUS;
    }
  }
  return;
 stallIn:
  MyUSBStallEndpoint(0x80|MSC_BULK_IN_ENDPOINT);
 requestError:
  SCSI.Status = SCSI_REQUEST_ERROR;
 sendStatus:
  SCSI.State = SCSI_SEND_STATUS;
}



/// Reset the SCSI state machine
void MyScsiReset() {
  memset(&SCSI, 0, sizeof(SCSI));
  if (map2->Flush)
      map2->Flush(map2, 1);
  SCSI.mapperNextFlushed = -1;
  //  puts("scsiReset");
}


/// Handle incoming packet from PC.
/// All packets sent by pc to the bulk out endpoint should be sent here.
void MyMSCPacketFromPC(USBPacket *inPacket) {
  // Check if it's a Command Block Wrapper
  if (inPacket->length == 31) {
    if (*(u_int32 *)inPacket->payload == 0x42435553UL) {

      // COMMAND BLOCK WRAPPER RECEIVED
      // Store Tag (word order copied as-is, no need to swap twice)
      *(u_int32 *)&SCSI.cswPacket[2] = *(u_int32*)&inPacket->payload[2];
      SCSI.DataTransferLength =
	((u_int32)SwapWord(inPacket->payload[5]) << 16) |
	SwapWord(inPacket->payload[4]);
      SCSI.DataDirection = inPacket->payload[6]; /*highest bit=1 device->host*/

    } else {
      SCSI.State = SCSI_INVALID_CBW; /* wait until BOT resets.. */
      MyUSBStallEndpoint(0x80|MSC_BULK_IN_ENDPOINT);
      MyUSBStallEndpoint(0x00|MSC_BULK_OUT_ENDPOINT);
      return;
    }
    // Call SCSI subsystem.
    // SCSI command is not aligned to word boundary so we need to 
    // send pointer to the preceding byte. As it turns out to be
    // the length of the command, it's not a disaster.

	DiskProtocolCommand(&(inPacket->payload[7])); /* ptr to len+cmd */

    MyScsiTaskHandler();//This is an extra function call to the
    //...scsi task handler to speed up time critical responses
    //to scsi requests. PC seems to have short nerve (3ms) sometimes
  } else {
    //NON-CBW BLOCK RECEIVED, must be a data block
    DiskDataReceived(inPacket->length, &(inPacket->payload[0]));
  }
}


/// Send a Command Status Wrapper to USB (return MSC Status);
/// \param status 0:ok 1:fail 2:phase_error
u_int16 MscSendCsw(u_int16 status) {
  SCSI.cswPacket[0] = 0x5553; //csw magic ident
  SCSI.cswPacket[1] = 0x4253; //csw magic ident
  //tag is already at [2] and [3], copied when receiving the command block

  SCSI.cswPacket[4] = SwapWord(labs(SCSI.Residue)); //data residue
  SCSI.cswPacket[5] = SwapWord(labs(SCSI.Residue)>>16); //data residue

  SCSI.cswPacket[6] = status << 8;

  if (ScsiIdle == 10) {
    ScsiIdle = 20;
  }
  
  return USBStartTransmission2(MSC_BULK_IN_ENDPOINT, SCSI.cswPacket, 13, 13);
}



