/*
 * USB bus lowlevel routines: $Id: devUsb.c,v 1.11 2013-07-19 16:24:32+03 erkki Exp erkki $
 */

#include <stdlib.h>
#include <string.h>
#include <vo_stdio.h>

#ifndef NO_VSIDE_SETUP

#include "vsos.h"
#include "exec.h"
#include "devUsb.h"
#include "vo_fat.h"   
#include "vs1005h.h"
#include "mmcCommands.h"
#include "devboard.h"
//#include "forbid_stdout.h"
#endif
#include "power.h"
#include "usb_host.h"
#include "scsi_host.h"
#include "timers.h"

//#define debug_printf(...); /* (...) */

#if 1
#define debug_printf4(x,y,z,w) ;
#define debug_printf3(x,y,z) ;
#define debug_printf2(x,y) ;
#define debug_printf1(x) ;
#else
#define debug_printf4(x,y,z,w) printf(x,y,z,w)
#define debug_printf3(x,y,z) printf(x,y,z)
#define debug_printf2(x,y) printf(x,y)
#define debug_printf1(x) printf(x)
#endif

// Max number of nack before fail
#define MAX_USB_NACKS     50000
#define MAX_USB_TRIES     50001

// Decriptor types
#define T_DEV_DESCRIPTOR   1
#define T_CONF_DESCRIPTOR  2
#define T_IFACE_DESCRIPTOR 4
#define T_EP_DESCRIPTOR    5
#define T_STATUS_DESC      6

#define MASS_STORAGE_CLASS 0x08
#define SCSI_CMD_SET 0x06
#define BULK_ONLY 0x50

#define UTM_RES_ADDR     0xFED1

#define GET_SOF_CNT() (PERIP(USB_UTMIR)&0x3FFF)

u_int16  WaitForUsbInt(u_int16 mask);
ioresult init_usb_bus();


// Internal Data buffer for usb/scsi transfers
u_int16 buffer[256];
u_int16 sofwait;

u_int16 SwapWord(register __a1 u_int16 d);

void  send_cmd(u_int16 addr, u_int16 ep, u_int16 pid, u_int16 flags);
int ResetUsbBus();
void WaitSendDelay(u_int16 i);
s_int16 configure_device(UsbDeviceInfo *usbdev);
void enable_autosofs(u_int16);
s_int16 get_descriptor(u_int16 addr, u_int16 type, u_int16 length);
u_int16 WaitForUsbInt(u_int16 mask);


//void UsbZeroLenPacketToEndpoint(register void *ep) {
//  UsbWriteToEndpoint(ep, NULL, 0, 0);
//}

/*
void UsbDelay(u_int32 n){
  while (n--) {
    n = ~n;
    n = ~n;
  }
}
*/

// Return byte from offset
u_int16 getbyte(u_int16 off,u_int16 *p) {
  u_int16 woff = (off>>1);
  if (off&1) {
    return p[woff]&0x00FF;
  }
  return p[woff]>>8;
}

// Set byte at offset
void setbyte(u_int16 *p,u_int16 off,u_int16 byte) {
  u_int16 woff = (off>>1);
  if (off&1) {
    p[woff] = (p[woff]&0xFF00)|byte;
  }
  p[woff] = (p[woff]&0x00FF)|((byte<<8)&0xFF00);
}

#if 0
void dump_devsc(UsbDeviceInfo *usbdev) {
  printf("Address %d\n",usbdev->addr);
  printf("Interface %d\n",usbdev->iface);

  printf("EP0Size %d\n",usbdev->ep[EP_0].size);
  printf("Toggle host %d, dev, %d\n",usbdev->ep[EP_0].host_toggle,
	 usbdev->ep[EP_0].dev_toggle);
  printf("EPout.size %d, Epout.num %d\n",usbdev->ep[EP_OUT].size,
	 usbdev->ep[EP_OUT].num);
  printf("Toggle host %d, dev, %d\n",usbdev->ep[EP_OUT].host_toggle,
	 usbdev->ep[EP_OUT].dev_toggle);
  printf("EpIn.size %d, EpIn.num %d\n",usbdev->ep[EP_IN].size,
	 usbdev->ep[EP_IN].num);
  printf("Toggle host %d, dev, %d\n",usbdev->ep[EP_OUT].host_toggle,
	 usbdev->ep[EP_OUT].dev_toggle);
}
#endif


/*
 * Bus allocation routine:
 *   If SOF cnt < 850 there is no room for
 *   slow FS device with endpoint size 64
*/

#define CHECKSOFMASK (TX_INT|SOF_INT)

void CheckSOF(u_int16 flag) {
  u_int16 i;

  // Wait for Sof
  if (flag) {
    PERIP(USB_STATUS) = CHECKSOFMASK;        
    WaitForUsbInt(SOF_INT);
    WaitForUsbInt(TX_INT);
    while (GET_SOF_CNT() > 11964) ;
    sofwait++;
   
  }  else if (GET_SOF_CNT() < 2000 || (PERIP(USB_STATUS)&SOF_INT) == SOF_INT) {
    PERIP(USB_STATUS) = CHECKSOFMASK;        
    sofwait++;
    while ((PERIP(USB_STATUS)&CHECKSOFMASK) != CHECKSOFMASK) ;        
    PERIP(USB_STATUS) = CHECKSOFMASK;

    while (GET_SOF_CNT() > 11964) ;
  }
}



ioresult init_usb_bus() {
  volatile u_int16 i,j,cnt;
  u_int16 jcnt;

  // bits 10 - 7 current drive
  //  USEY(UTM_RES_ADDR) = 0x4002|(1<<5)|(1<<6);

  enable_autosofs(0);

  USEY(UTM_RES_ADDR) = 0x8000;

  PERIP(USB_UTMIW)   = (1<<15);
  PERIP(USB_CONFIG)  = 0x8000;       
  PERIP(USB_WRPTR)   = 0x0000;       
  PERIP(USB_RDPTR)   = 0x0000;
  PERIP(USB_CONFIG)  = (1<<10)|(1<<9)|(1<<7);  

  
  //Power off
  //SetPower(PCL_USB_A|PCL_USB_5V,0);
  //Delay(200);
  
  // Poweron BUS
  SetPower(PCL_USB_A|PCL_USB_5V,1);
  Delay(2000);
  // Wait for device powerup
  // UsbDelay(7000000);
  PERIP(USB_UTMIW)   = (1<<15)|(1<<3)|(1<<2);  
  Delay(100);

  j = GET_LINES(); 
  //printf("Lines: %d\n",j);
  if (j != LINE_J) {
  	return S_ERROR; //No pull-up detected
  }
	

  
#if 0
  // Wait for device DP pullup
  for (cnt=0,jcnt=0; cnt < 65000 && jcnt < 100; ++cnt) {
    j = GET_LINES();
    if (j == LINE_J) { //D+ pull up detected
	++jcnt;
    } else {
      jcnt = 0;
      UsbDelay(2000);
    }
  }

  if (jcnt < 100) {
    //    printf("NO STICK\n");
    return S_ERROR;
  }
#endif


  // Reset device
  ResetUsbBus();

  // Turn automatic sof transmitting on
  enable_autosofs(1);
  
  return S_OK;
}


s_int16 parse_descriptor(UsbDeviceInfo *usbdev,u_int16 off,u_int16 *pkt) {
  u_int16 l = getbyte(off+0,pkt);   // pituus
  u_int16 t = getbyte(off+1,pkt);   // tyyppi

  switch (t) {
   
  case T_DEV_DESCRIPTOR:
    debug_printf3("Device descriptor %d, Length %d\n",t,l);
    debug_printf2("EP0SIZE %d\n",getbyte(off+7,pkt));
    usbdev->ep[EP_0].size = getbyte(off+7,pkt);
    break;

  case T_CONF_DESCRIPTOR:
    debug_printf3("Configuration descriptor %d, Length %d\n",t,l);
    debug_printf2("Interface Number %d\n",getbyte(off+4,pkt));
    debug_printf2("Interface Configuration Value %d\n",getbyte(off+5,pkt));
    debug_printf2("Max Power (getbyte) %d mA\n",2*getbyte(off+8,pkt));
    break;
    
  case T_IFACE_DESCRIPTOR:
    debug_printf3("Interface descriptor %d, Length %d\n",t,l);
    debug_printf2("InterfaceNumber %d\n",getbyte(off+2,pkt));
    debug_printf2("bAlternateSetting %d\n",getbyte(off+3,pkt));
    debug_printf2("bNumEndpoints %d\n",getbyte(off+4,pkt));

    // interface is ok if 
    // - numendpoints == 2;
    // - interfaceclass == MASS_STORAGE (0x08)
    // - interface subclass == SCSI_TRANSPARENT_COMMAND_SET (0x06)
    // - interface protocol == BULK_ONLY_TRANSPORT (0x50)

    if (getbyte(off+4,pkt) != 2 || getbyte(off+5,pkt) != MASS_STORAGE_CLASS ||
	getbyte(off+6,pkt) != SCSI_CMD_SET || getbyte(off+7,pkt) != BULK_ONLY) {
      usbdev->iface = -1;
    } else {
      usbdev->iface = getbyte(off+2,pkt);
    }
    break;

  case T_EP_DESCRIPTOR:
    debug_printf3("Endpoint descriptor %d, Length %d\n",t,l);
    debug_printf2("EndPoint number %X\n",getbyte(off+2,pkt));
    debug_printf2("TransferType %X\n",getbyte(off+3,pkt));
    debug_printf2("MaxPacketSize %X\n",getbyte(off+4,pkt));  
	if (getbyte(off+3,pkt) == 0x02) { //consider only bulk endpoints for MSC
  	
		if (getbyte(off+2,pkt) & 0x80) {
			usbdev->ep[EP_IN].size = getbyte(off+4,pkt);
			usbdev->ep[EP_IN].num  = getbyte(off+2,pkt)&0x7F;
		} else {
			usbdev->ep[EP_OUT].size = getbyte(off+4,pkt);
			usbdev->ep[EP_OUT].num  = getbyte(off+2,pkt);
		}
	}
    break;
  default:
    debug_printf2("Negative %d\n",t);
    return -1;
  }
  return l;
}

void UsbSetupReadOperation(register EpInfo *ep, register char *opString, 
			      register void *inBuffer, register u_int16 inBytes) {
  u_int16 result = 0;
  UsbWriteToEndpoint(ep, opString, 8, 1);
  if (UsbReadFromEndpoint(ep, inBuffer, inBytes) != E_STALL) {
    UsbWriteToEndpoint(ep, NULL, 0, 0);
  }
}


void parse_conf_packet(UsbDeviceInfo *usbdev,u_int16 len,u_int16 *pkt) {
  u_int16 i;
  u_int16 off = 0;
  u_int16 l,l1,tmp,tmp1,tmp2;
	u_int16 timeoutCounter =0;
	
  off = 0;
  while (off < len) {
    u_int16 x = parse_descriptor(usbdev,off,pkt);//MT Really bad the function return a signed int and when it return -1 it can be stuck in this while loop
    #if debugUSB
	printf("conf: %u %u %u\n",x,off,len);
  #endif
    if (x > 0 && x != 65535) { //MT added && x != 65535 because i think we do not want to decrement on a negative value it should be a s_error?
      off += x;
    }
    if(timeoutCounter>1000){
   	break; 
	}
    timeoutCounter++;
  }
  return;
}

void PrintStringDescriptor(EpInfo *ep, u_int16 i) {
	u_int16 n;
	//u_int16 request[4] = {0x8006, 0x0003, 0x0000, 0x1200};
	u_int16 request[4] = {0x8006, 0x0003, 0x0000, 0x8000};
	request[1] |= (i<<8);
	UsbSetupReadOperation(ep, request, buffer, 100);
	n = buffer[0]>>9; //String length
	for (i=1; i<n; i++) {				
		printf("%c",buffer[i]>>8);
	}
	printf(" ");
}

// Configure mass storage device (USB stick)
int configure_device(UsbDeviceInfo *usbdev) {
  u_int16 i,cnt,len;
  EpInfo *ep = &usbdev->ep[0];

  sofwait = 0;
  memset(usbdev, 0,sizeof(UsbDeviceInfo));
  memset(ep, 0, sizeof(EpInfo));

  // Initialize control endpoint data structure
  ep->size        = 8;
  ep->num         = 0;
  ep->dev_toggle  = 1;
  ep->host_toggle = 0;
  ep->dev         = usbdev;
  usbdev->addr    = 0;
  usbdev->ep[EP_OUT].dev = usbdev;
  usbdev->ep[EP_IN].dev  = usbdev;
  

  // Wait for some time
  for (i=0; i < 100;) {
    if (PERIP(USB_STATUS)&SOF_INT) {
      if (PERIP(USB_STATUS)&TX_INT) {
	i++;
	PERIP(USB_STATUS) = SOF_INT|TX_INT;
      }
    }
  }



  // 1. Get device descriptor + MaxPacketSize
  UsbSetupReadOperation(ep, "\p\x80\x06\x00\x01\x00\x00\x08\x00", buffer, 8);
  ep->size = getbyte(7,buffer);

  ResetUsbBus();
  // 1.1 Get device descriptor + MaxPacketSize
  UsbSetupReadOperation(ep, "\p\x80\x06\x00\x01\x00\x00\x12\x00", buffer, 18);
  
  printf("\nVID_%04x PID_%04x ",SwapWord(buffer[4]),SwapWord(buffer[5]));
  PrintStringDescriptor(ep,1);
  PrintStringDescriptor(ep,2);
  printf("\n");

  // 2. Get size of configuration descriptor
  UsbSetupReadOperation(ep,"\p\x80\x06\x00\x02\x00\x00\x09\x00", buffer, 9);
  i = buffer[1]>>8; //Configuration descriptor size, will fail if descriptor > 255 bytes.
  if(i>127)
  {
  		return 0; //MT Patch because we got a phone who crash when he read 128 bytes and he should read 39
	}	
 #if debugUSB
  printf("Read config\n");
  #endif
  // 3. Read whole configuration descriptor
  UsbSetupReadOperation(ep,"\p\x80\x06\x00\x02\x00\x00\xff\x00", buffer, i);


  debug_printf1("Parse Conf packet\n");
  parse_conf_packet(usbdev,i,buffer);
  debug_printf1("Done\n");

  // 4. Set Address
  UsbWriteToEndpoint(ep, "\p\x00\x05\x01\x00\x00\x00\x00\x00", 8, 1);
  UsbReadFromEndpoint(ep, buffer, 0x64);
  usbdev->addr = 1;
  CheckSOF(1);

  // 5. Set Configuration
  UsbWriteToEndpoint(ep, "\p\x00\x09\x01\x00\x00\x00\x00\x00", 8, 1);
  UsbReadFromEndpoint(ep, buffer, 0x64);
  CheckSOF(1);
  return 1;
}

EpInfo  *OpenEndpoint(UsbDeviceInfo *d, u_int16 n) {
  EpInfo *epinfo = &d->ep[0];
  //printf("Opening endpoint %p %p\n",d,epinfo);
  //while (1) ;

  return (epinfo+n);

  // Clear endpoint, 
  // EpInfo: datatoggle, direction, size, {bulk|interrupt|iso}, *info
}


ioresult OpenUsbDevice(UsbDeviceInfo *usbinfo) {
	u_int16 cnt,j,jcnt,i;
	
	// Detect device, set address, set configuration.
	// Check DP state
	// Reset bus
	// Get Device Descriptor(8 bytes) -> MaxPacketSize
	// Get Device Descriptor(all 18 bytes)
	
	// Get Configuration Descriptor(9 bytes) -> wTotalSize
	// Get Configuration Descriptor(wTotalSize)
  	
	// Get EP info
	// Set Address(addr)
	// Set Configuration(addr,1)
  	
	// Detect device, set address, set configuration.
	// Find endpoints, populate epinfo
	
	//printf("OpenUsbDevice\n");
  	
	if (init_usb_bus() != S_OK) {
		return S_ERROR;
	}
	
	enable_autosofs(1);
	Delay(100);
  	
	configure_device(usbinfo);
	  	
	// Wait some time device to set configuration
	Delay(100);
	
	return S_OK;
}
	
// Enable hw sof transmitter
void enable_autosofs(u_int16 onoff) {
  if (onoff) {
    PERIP(USB_UTMIW) |= (1<<12);
  } else {
    PERIP(USB_UTMIW) &= ~(1<<12);
  }
}

int ResetUsbBus() {
  u_int16 j;

  // Wait 1ms
  enable_autosofs(0);

  PERIP(USB_STATUS) |= SOF_INT;  
  while (!(PERIP(USB_STATUS)&SOF_INT)) ;
  PERIP(USB_STATUS) |= SOF_INT;  
  
  // Set Reset state
  PERIP(USB_UTMIW) = 0x8000;

  // wait_for_10ms();
  for (j=0; j < 10; j++) {
    while (! (PERIP(USB_STATUS)&SOF_INT)) ;
    PERIP(USB_STATUS) |= SOF_INT;  
  }
  
  PERIP(USB_UTMIW)  = 0x800C|(1<<12);
  Delay(10);//MTC
  enable_autosofs(1);
  Delay(10);//MTC
  CheckSOF(1);
  Delay(50);//MTC
}


/*
 * USB line state commands
 */

//
// Wait for usb_int for one SOF or Timeout
//
u_int16 WaitForUsbInt(u_int16 mask) {
  u_int16 ret;
  do {
    ret = PERIP(USB_STATUS);    
  } while ((ret&mask) == 0);
  PERIP(USB_STATUS) = ret&mask;
  return ret;
}


void send_cmd(u_int16 addr, u_int16 ep, u_int16 pid, u_int16 flags) {  
  u_int16 old;

  // remember old sendmem data word for restore at end
  old = USEY(USB_SEND_MEM); 

  // Disable interrupts

  PERIP(USB_STATUS)    = RX_INT|TIME_OUT|TX_INT|TOGGLE_ERR;
  PERIP(USB_EP_OUT0)   = 0x8000;  
  USEY(USB_SEND_MEM)   = (addr<<8)|ep;

  CheckSOF(0);
  PERIP(USB_HOST)      = (pid<<12)|SEND_PACKET|flags;
  // Enable interrupts

#if 0
  if (PERIP(USB_STATUS) & SOF_INT) {
    printf("send_cmd sof_set !!!\n");
    while (1) ;
  }
  if (GET_SOF_CNT() < 2000) {
    printf("Too low counter %d\n",GET_SOF_CNT());
  }
#endif

  WaitForUsbInt(TX_INT);
  USEY(USB_SEND_MEM) = old;
}


ioresult UsbWriteToEndpoint(register EpInfo *ep, register int *buf, 
			    register u_int16 bytes, register u_int16 isSetup) 
{
  
  u_int16 nack,cnt,errs,timeout;
  u_int16 sendBytes,sendWords;
  ioresult res = S_ERROR;
  timeout = nack = cnt = errs = 0;
  
  Forbid();
  do {
    u_int16 x,t;
    cnt++;
    PERIP(USB_WRPTR) = 0;
    PERIP(USB_RDPTR) = 0;
    
    if (isSetup == 0) {
      send_cmd(ep->dev->addr, ep->num, PID_OUT, 0);
    } else {
      send_cmd(ep->dev->addr, ep->num, PID_SETUP, 0);
      ep->host_toggle = 0;
      ep->dev_toggle  = 1;
    }
    
    if (bytes > ep->size) {
      sendBytes = ep->size;
      sendWords = ep->size>>1;
      memcpyXY((__mem_y*)USB_SEND_MEM,buf,sendWords);

      //buf += sendWords;
      //bytes -= sendBytes;
    } else {
      sendBytes = bytes;
      sendWords = (bytes+1)>>1;
      memcpyXY((__mem_y*)USB_SEND_MEM,buf,sendWords);
      //bytes -= sendBytes;
    }
    
    PERIP(USB_EP_OUT0)   = 0x8000|sendBytes;  
    
    if (ep->host_toggle) {
      PERIP(USB_HOST)      = (PID_DATA1<<12)|SEND_PACKET;
    } else {
      PERIP(USB_HOST)      = (PID_DATA0<<12)|SEND_PACKET;
    }
    
    // Wait for response to data packet
    x  = WaitForUsbInt(RX_INT|TIME_OUT|SOF_INT);
    
    // Diagnostic
    if (x & SOF_INT) {
      t = 0;
      //printf("SOF AT WRONG PLACE %d\n",GET_SOF_CNT());
      // WaitForUsbInt(TX_INT);      
    } else if (x & RX_INT) {
      t  = PERIP(USB_STATUS)&0x0F;
    } else if (x & TIME_OUT) {
      t=0;
      timeout++;
    } else {
      t = 0;
    }
    
    // Wait for timeout or RX_INT
    switch (t) {

    case PID_NACK:
#if 1
      if (nack > 5) {
	Delay(10);
	CheckSOF(1);
      }
#endif
      nack++;
      break;
      
    case PID_STALL:
      //printf("STALL\n");
      res = E_STALL;
      goto finally;
      break;

    case PID_ACK:
      ep->host_toggle = ep->host_toggle?0:1;      

      buf   += sendWords;
      bytes -= sendBytes;

      if (bytes == 0) {
	res = S_OK;
	goto finally;
      }

    default:
      break;
    }
    
    // If no success -> return
    if (timeout > 5) {
      debug_printf2("TIMEOUT %d > 5\n",timeout);
      goto finally;
    }
    if (nack > MAX_USB_NACKS) {
      debug_printf2("NACKS %d > 1000\n",nack);
      goto finally;
    }
    if (cnt > MAX_USB_TRIES) {
      debug_printf1("CNT > 1000\n");
      res = E_AGAIN;
      goto finally;
    }
    if (errs > 1000) {
      debug_printf1("errs > 100\n");
      goto finally;
    }
  } while (1);

 finally:
  Permit();
  return res;
}

// ioresult UsbReadFromEndpoint(devAddr, ep, &buf, bytes);

ioresult UsbReadFromEndpoint(register EpInfo *ep, register int *buf, register u_int16 bytes)
{
  s_int16 errs,timeout;
  s_int16 t,x,amount;
  s_int32 cnt,nack;//MTC should be u_int16 cnt,nack; because 
  int *p;
  ioresult res = S_ERROR;

  p       = buf;
  timeout = 0;
  errs    = 0;
  amount  = 0;
  cnt     = 0;
  nack    = 0;
  
  //printf("Reading %d bytes from endpoint%p(%d) %d\n",bytes,ep,ep->num,*toggle);

  Forbid();
  while (1) {
    cnt++;
    PERIP(USB_WRPTR) = 0;
    PERIP(USB_RDPTR) = 0;

    send_cmd(ep->dev->addr, ep->num, PID_IN, 0);

    x  = WaitForUsbInt(RX_INT|TIME_OUT|SOF_INT);

    if (x & SOF_INT) {
      t = 0;
      // debug_printf("SOF AT WRONG PLACE %d\n",GET_SOF_CNT());
      // WaitForUsbInt(TX_INT);
      
    } else if (x & RX_INT) {
      t  = PERIP(USB_STATUS)&0x0F;

    } else if (x & TIME_OUT) {
      timeout++;
      t = 0;
    } else {
      t = 0;
    }

    // Wait for timeout or RX_INT
    switch (t) {

    case PID_NACK:

#if 0
      // prevent IN-NAK flood, Yield()?
      if (nack > 5) {
	CheckSOF(1);
      }
#endif      
      nack++;
      break;
      
    case PID_STALL:
      debug_printf1("STALL\n");
      res = E_STALL;
      goto finally;
      break;
      
    case PID_DATA0:
    case PID_DATA1:
      if (t == PID_DATA0 && ep->dev_toggle != 0) {
	debug_printf1("Wrong data\n");
	errs++;
      } else if (t == PID_DATA1 && ep->dev_toggle != 1) {
	debug_printf1("Wrong data\n");
	errs++;
      } else {
	u_int16 numbytes = (USEY(PERIP(USB_RDPTR)+USB_RECV_MEM)&0x03FF);
	u_int16 lenW = (numbytes+1)>>1;
	
	ep->dev_toggle = ep->dev_toggle?0:1;
	amount += numbytes;

#if 1
	if (amount > bytes) {
	  // Check if read is too long
	  lenW = (amount-bytes+1)>>1;
	  debug_printf3("Truncated  %d %d\n",amount,bytes);
	}
	
	if (numbytes > ep->size) {
	  // Too many bytes from device
	  debug_printf3("Too big packet %d %d\n",numbytes,ep->size);
	  goto finally;
	}
#endif
	memcpyYX(p,(__mem_y *)(USB_RECV_MEM+1),lenW);
	p += lenW;

	if (amount > bytes) {
	  debug_printf3("Crapped buffer with amount %d > %d.\n",amount,bytes);
	}

	if (amount >= bytes || numbytes < ep->size) {
	  res = amount;
	  goto finally;
	}
      }
      break;

    default:
      break;
    }

    
    if (timeout > 5) {
      // Device seems to be mute
      debug_printf2("TIMEOUT %d > 5\n",timeout);
      goto finally;
    }
    
    if (nack > MAX_USB_NACKS) {//MTC if nack is s_int16 this never happen because 50000 is too big for a signed 16 bit
      // Too many nacks
      debug_printf2("NACKS %d > 50000\n",nack);
      goto finally;
    }
    
    if (cnt > MAX_USB_TRIES) {
      // Too many tries
      debug_printf1("CNT > 50001\n");
      res = E_AGAIN;
      goto finally;
    }
  }

 finally:
  Permit();
  return res;
}
