#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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


s_int16 UsbWriteToEndpoint(EpInfo *ep, int *buf, 
			    u_int16 bytes, u_int16 isSetup) ;

s_int16 UsbReadFromEndpoint(EpInfo *ep, int *buf, 
			     u_int16 bytes);


UsbDeviceInfo devsc;

UsbDeviceInfo OpenUsbDevice();
EpInfo  *OpenEndpoint(UsbDeviceInfo *d, u_int16 n);

// Internal Data buffer for usb/scsi transfers
u_int16 buffer[64];
u_int16 sofwait;

s_int16 scsi_request(u_int16 opcode,UsbDeviceInfo *dev,u_int16 *buf,u_int16 len,u_int16 lun,
		     u_int32 lba);

u_int16 SwapWord(register __a1 u_int16 d);

struct usb_device_descriptor devUsb = {   
  0, // u_int16 flags; //< present, block/char
  DevUsbIdentify, //char*    (*Identify)(void *obj, char *buf, u_int16 bufsize);   
  DevUsbCreate, //ioresult (*Create)(DEVICE *dev, char *name, struct filesystem_descriptor *fs);
  (ioresult(*)(DEVICE*))CommonOkResultFunction, //ioresult (*Delete)(DEVICE *dev);
  DevUsbIoctl, //ioresult (*Ioctl)(DEVICE *dev, s_int16 request, char *argp); //Start, Stop, Flush, Check Media
  // Stream operations
  0,
  0,
  // Block device operations
  DevUsbBlockRead, //ioresult (*BlockRead)(DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data);
  DevUsbBlockWrite, //ioresult (*BlockWrite)(DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data);
  // Stream operations
  //DevUsbInput, //ioresult (*Input)(DEVICE *dev, u_int16 port, void *buf, u_int16 bytes);
  //DevUsbOutput, //ioresult (*Output)(DEVICE *dev, u_int16 port, void *buf, u_int16 bytes);
  //FILESYSTEM *fs;
  //DIRECTORY *root; //Zero if the device cannot hold directories
  //u_int16  deviceInstance;
  //u_int16  hardwareInfo[6]; ///< Device driver's info of this hardware
  //u_int16  deviceInfo[10]; // For filesystem use, size TBD   
};



// ====================================================================
// ====================================================================
// ====================================================================

// Supports one mass storage device
#define IOCTL_INITBUS 10
#define IOCTL_STOPBUS 11
#define IOCTL_LSBUS   12

ioresult DevUsbCreate (register __i0 DEVICE *dev, void *name,
			u_int16 extraInfo) {
  if ((void*)dev != (void*)&devUsb) {
    return SysError ("SD must be devUsb");
  }
  // printf("DevUsbCreate kutsuttu\n");
  dev->deviceInstance = __nextDeviceInstance++;
  return dev->Ioctl(dev, IOCTL_INITBUS, 0);
}


const char *DevUsbIdentify(register __i0 void *obj, char *buf, u_int16 bufsize){
  return "Usb Identify";
}

const char *UsbIdentify(void){
  return "Usb Identify";
}

ioresult DevUsbBlockWrite(register __i0 DEVICE *dev,  u_int32 firstBlock,
			   u_int16 blocks, u_int16 *data){
  //devSdSdHwInfo* hw = (devUsbHwInfo*)dev->hardwareInfo;

}

int init_usb_bus();

ioresult DevUsbIoctl(register __i0 DEVICE *dev,
		      s_int16 request, char *argp) {
  devUsbHwInfo *hw=(devUsbHwInfo*)dev->hardwareInfo;
  u_int16 i, n, cmd;
  // printf("DevUsbIoCtl (%d) kutsuttu\n",request);
  switch (request) {
  case IOCTL_RESTART:
    // printf("SD IOCTL RESTART\n");
    // printf("rca ennen: %x\n",hw->rca);
    UsbIdentify();
    // hw->rca = 1234;
    // printf("rca jalkeen: %x\n",hw->rca);
    dev->flags = __MASK_PRESENT | __MASK_OPEN |
      __MASK_SEEKABLE | __MASK_READABLE | __MASK_WRITABLE;
    // Find a filesystem, which understands this device
    dev->fs = StartFileSystem(dev, "0");
    break;

  case IOCTL_INITBUS:
    init_usb_bus();    
    printf("Init Scsi\n");
    init_scsi();
    printf("INITBUS\n");
    while (1) ;
    return S_ERROR;
    break;
    
  case IOCTL_STOPBUS:
    printf("STOPBUS\n");
    return S_ERROR;
    break;

  case IOCTL_LSBUS:
    // return type,name    
    return S_ERROR;
    break;
  default:
    return S_ERROR;
    break;
  }         
  return S_OK;
}


s_int16 read_ep(u_int16 addr,u_int16 epinx,u_int16 *buffer,u_int16 length);
void  send_cmd(u_int16 addr, u_int16 ep, u_int16 pid, u_int16 flags);
int ResetUsbBus();
void  set_usb_reset();
u_int16 sendsofs(u_int16 cnt);
s_int16 wait_for_ack();
void sendsof();
void send_zerodata(u_int16 addr,u_int16 ep, u_int16 pid);
void WaitSendDelay(u_int16 i);
int configure_device(UsbDeviceInfo *usbdev);
s_int16 usb_bulk_only_reset_recovery();
s_int16 clear_endpoint(u_int16 addr,u_int16 ep);

#define UTM_RES_ADDR     0xFED1
void send_getdesc_dev(u_int16 addr,u_int16 len);
void send_getdesc_conf(u_int16 addr, u_int16 len);
void enable_autosofs(u_int16);
s_int16 get_descriptor(u_int16 addr, u_int16 type, u_int16 length);
u_int16 WaitForUsbInt(u_int16 mask);


#define GET_SOF_CNT() (PERIP(USB_UTMIR)&0x3FFF)
#define CHECKSOFMASK (TX_INT|SOF_INT)

/*
 * Bus allocation routine:
 *   If SOF cnt < 850 there is no room for
 *   slow FS device with endpoint size 64
*/
void CheckSOF(u_int16 flag) {
  u_int16 i;

  // Wait for Sof
  if (flag) {
    PERIP(USB_STATUS) = CHECKSOFMASK;        
    WaitForUsbInt(SOF_INT);
    WaitForUsbInt(TX_INT);
    while (GET_SOF_CNT() > 11964) ;
    sofwait++;
   
  }  else if (GET_SOF_CNT() < 850 || (PERIP(USB_STATUS)&SOF_INT) == SOF_INT) {
    PERIP(USB_STATUS) = CHECKSOFMASK;        
    sofwait++;
    while ((PERIP(USB_STATUS)&CHECKSOFMASK) != CHECKSOFMASK) ;        
    PERIP(USB_STATUS) = CHECKSOFMASK;

    while (GET_SOF_CNT() > 11964) ;
  }
}


int init_usb_bus() {
  volatile u_int16 i,j;
  u_int16 cnt,jcnt;

  USEY(ANA_CF3) = 0xBFFF;   // 0xFED3
  USEY(ANA_CF2) = 0xFFFF;   // v1,vco: 0xF518;   // 0xFED2
  USEY(ANA_CF0) = 0x3810;
  USEY(ANA_CF1) = 0x01FF;
  USEY(ANA_CF3) = 0x8008|0x4000;
  USEY(ANA_CF0) = 0x3810;
  USEY(ANA_CF1) = 0x0100;
  USEY(ANA_CF1) = 0x0148|(1<<12);

  USEY(ANA_CF2) = 0xF518;   // v1,vco: 0xF518;   // 0xFED2

  // bits 10 - 7 current drive
  //  USEY(UTM_RES_ADDR) = 0x4002|(1<<5)|(1<<6);
  USEY(UTM_RES_ADDR) = 0xC003;
  USEY(FM_CF)  = 0x0041;
  
  USEY(FMCCF_HI) = 0xFF88; // 12.288MHz oscillator
  USEY(FMCCF_LO) = 0x0000;

  USEY(CLK_CF) = 0x00C4;
  USEY(CLK_CF) = 0x0084;
  USEY(CLK_CF) = 0x0084;
  USEY(CLK_CF) = 0x0084;
  USEY(CLK_CF) = 0x00C4;
  USEY(CLK_CF) = 0x00C4;
  USEY(CLK_CF) = 0x00C4;
  USEY(CLK_CF) = 0x00C4;
  USEY(CLK_CF) = 0x00C4;
  USEY(CLK_CF) = 0x00C4;
  USEY(CLK_CF) = 0x00C4;
  USEY(CLK_CF) = 0x00E4;


  PERIP(USB_CONFIG) = 0x8000;       
  PERIP(USB_CONFIG) = 0x0000;       
  PERIP(USB_CONFIG) = 0x0000;       
  PERIP(USB_RDPTR)  = 0x0000;

  PERIP(USB_CONFIG) = (1<<10)|(1<<9)|(1<<7);  
  PERIP(USB_CONFIG) = 0x8000;       
  PERIP(USB_CONFIG) = 0x0000;       
  PERIP(USB_CONFIG) = (1<<10)|(1<<9)|(1<<7);  
  PERIP(USB_UTMIW)  = (1<<15)|(1<<3)|(1<<2);

  // Poweron BUS
  SetPower(PCL_USB_A|PCL_USB_5V,1);

  // Wait for device powerup
  for (cnt=0; cnt < 100; cnt++) {
    BusyWait1();
  }
  
  //  PERIP(USB_CONFIG) = (1<<11)|(1<<10)|(1<<9)|(1<<7);
  // Is there device attached
  for (cnt=0,jcnt=0; cnt < 10000 && jcnt < 10; ++cnt) {
    j = GET_LINES();
    if (j&LINE_J) { //D+ pull up detected
      ++jcnt;
    } else {
      jcnt = 0;
    }
    for (i=0; i < 1000; i++) ;
  }

  if (jcnt < 9) {
    printf("NO STICK\n");
    return 0;
  }
  // Reset device
  ResetUsbBus();

  // Turn automatic sof transmitting on
  enable_autosofs(1);
}


u_int16 WaitForUsbInt(u_int16 mask);

// Return byte from byte offset
u_int16 getbyte(u_int16 off,u_int16 *p) {
  u_int16 woff = (off>>1);
  if (off&1) {
    return p[woff]&0x00FF;
  }
  return p[woff]>>8;
}

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

// Decriptor types
#define T_DEV_DESCRIPTOR   1
#define T_CONF_DESCRIPTOR  2
#define T_IFACE_DESCRIPTOR 4
#define T_EP_DESCRIPTOR    5
#define T_STATUS_DESC      6

s_int16 parse_descriptor(UsbDeviceInfo *usbdev,u_int16 off,u_int16 *pkt) {
  u_int16 l = getbyte(off+0,pkt);   // pituus
  u_int16 t = getbyte(off+1,pkt);   // tyyppi

  switch (t) {
   
  case T_DEV_DESCRIPTOR:
    printf("Device descriptor %d, Length %d\n",t,l);
    printf("EP0SIZE %d\n",getbyte(off+7,pkt));
    usbdev->ep[EP_0].size = getbyte(off+7,pkt);
    break;

  case T_CONF_DESCRIPTOR:
    printf("Configuration descriptor %d, Length %d\n",t,l);
    printf("Interface Number %d\n",getbyte(off+4,pkt));
    printf("Interface Configuration Value %d\n",getbyte(off+5,pkt));
    printf("Max Power (getbyte) %d mA\n",2*getbyte(off+8,pkt));
    break;
    
  case T_IFACE_DESCRIPTOR:
    printf("Interface descriptor %d, Length %d\n",t,l);
    printf("InterfaceNumber %d\n",getbyte(off+2,pkt));
    printf("bAlternateSetting %d\n",getbyte(off+3,pkt));
    printf("bNumEndpoints %d\n",getbyte(off+4,pkt));

    // interface is ok if 
    // - numendpoints == 2;
    // - interfaceclass == MASS_STORAGE (0x08)
    // - interface subclass == SCSI_TRANSPARENT_COMMAND_SET (0x06)
    // - interface protocol == BULK_ONLY_TRANSPORT (0x50)

    #define MASS_STORAGE_CLASS 0x08
    #define SCSI_CMD_SET 0x06
    #define BULK_ONLY 0x50

    if (getbyte(off+4,pkt) != 2 || getbyte(off+5,pkt) != MASS_STORAGE_CLASS ||
	getbyte(off+6,pkt) != SCSI_CMD_SET || getbyte(off+7,pkt) != BULK_ONLY) {
      usbdev->iface = -1;
    } else {
      usbdev->iface = getbyte(off+2,pkt);
    }
    break;

  case T_EP_DESCRIPTOR:
    printf("Endpoint descriptor %d, Length %d\n",t,l);
    printf("EndPoint number %X\n",getbyte(off+2,pkt));
    printf("TransferType %X\n",getbyte(off+3,pkt));
    printf("MaxPacketSize %X\n",getbyte(off+4,pkt));    
    if (getbyte(off+2,pkt) & 0x80) {
      usbdev->ep[EP_IN].size = getbyte(off+4,pkt);
      usbdev->ep[EP_IN].num = getbyte(off+2,pkt)&0x7F;
    } else {
      usbdev->ep[EP_OUT].size = getbyte(off+4,pkt);
      usbdev->ep[EP_OUT].num = getbyte(off+2,pkt);
    }
    break;
  default:
    return -1;
  }
  return l;
}

void parse_conf_packet(UsbDeviceInfo *usbdev,u_int16 len,u_int16 *pkt) {
  u_int16 i;
  u_int16 off = 0;
  u_int16 l,l1,tmp,tmp1,tmp2;
 
  off = 0;
  while (off < len) {
    u_int16 x = parse_descriptor(usbdev,off,pkt);
    if (x > 0) {
      off += x;
    }
  }
  return;
}

#define CONF_TRY_LIMIT 5
#define DESC_TRY_LIMIT 5

void send_set_std_desc(u_int16 addr, u_int16 bmRequestType,
		       u_int16 bRequest, u_int16 wValue,
		       u_int16 wIndex)
{

  PERIP(USB_STATUS)    = TX_INT|TIME_OUT|RX_INT;
  USEY(USB_SEND_MEM+1) = SwapWord(wValue);
  USEY(USB_SEND_MEM+2) = SwapWord(wIndex);
  USEY(USB_SEND_MEM+3) = 0; // Number of bytes is there is data stage

  send_cmd(addr, 0 ,PID_SETUP,0);
  
  USEY(USB_SEND_MEM)   = (bmRequestType << 8) | (bRequest&0xff);
  PERIP(USB_STATUS)    = TX_INT|TIME_OUT|RX_INT;
  
  PERIP(USB_EP_OUT0)   = 0x8008;
  PERIP(USB_HOST)      = (PID_DATA0<<12)|SEND_PACKET;
    
  WaitForUsbInt(TX_INT);
}

void send_get_std_desc(u_int16 addr, u_int16 bmRequestType,
		       u_int16 bRequest, u_int16 wValue,
		       u_int16 wIndex,u_int16 wLength)
{

  PERIP(USB_STATUS)    = TX_INT|TIME_OUT|RX_INT;
  USEY(USB_SEND_MEM+1) = SwapWord(wValue);
  USEY(USB_SEND_MEM+2) = SwapWord(wIndex);
  USEY(USB_SEND_MEM+3) = SwapWord(wLength);

  send_cmd(addr, 0 ,PID_SETUP,0);
  
  USEY(USB_SEND_MEM)   = (bmRequestType << 8) | (bRequest&0xff);
  PERIP(USB_STATUS)    = TX_INT|TIME_OUT|RX_INT;
  
  PERIP(USB_EP_OUT0)   = 0x8008;
  PERIP(USB_HOST)      = (PID_DATA0<<12)|SEND_PACKET;
    
  WaitForUsbInt(TX_INT);
}

#if 1

s_int16 set_descriptor(u_int16 addr, u_int16 dst, u_int16 type, u_int16 value) {
  u_int16 cnt = 0;
  u_int16 i=0;
  u_int16 chunk = 0;

  // This is always with EP0
  devsc.ep[0].dev_toggle = 1;
  devsc.ep[0].host_toggle = 0;
  
  for (i=0; i == 0; ) {
    send_set_std_desc(addr,dst,type,value,0);
    i = wait_for_ack();
    if (i < 0) {
      printf("XNO ACK %d\n",i);
    }
    if (i == PID_STALL)
      return -1;
    
    cnt++;
    
    if (cnt > DESC_TRY_LIMIT) {
      return -1;
    }
  }

  chunk = devsc.ep[EP_0].size;  
  {
    u_int16 b =0;
    u_int16 off=0;
    cnt = 0;    
    b = read_ep(addr,0,buffer+off,chunk);
    return b;
  }
}
#endif

void send_clear_endpoint(u_int16 addr,u_int16 ep) {  

  USEY(USB_SEND_MEM)   = 0x0201;
  USEY(USB_SEND_MEM+1) = 0x0000;
  USEY(USB_SEND_MEM+2) = 0x0000|(ep<<8);  
  USEY(USB_SEND_MEM+3) = 0x0000;

  send_cmd(addr, 0,PID_SETUP,0);

  PERIP(USB_EP_OUT0)   = 0x8008;
  PERIP(USB_HOST) = (PID_DATA0<<12)|SEND_PACKET;

  WaitForUsbInt(TX_INT);
}


// Clear enpoint 
s_int16 clear_endpoint(u_int16 addr,u_int16 ep) {
  
  u_int16 i;
  s_int16 err;

  err = 0;

  for (i=0; i < 3; i++) {
    send_clear_endpoint(devsc.addr,ep);

    if (wait_for_ack() < 0) {
      printf("NO ACK FROM CLEAR_ENDPOINT\n");
      continue;
    }

    err = read_ep(devsc.addr,devsc.ep[EP_0].num,buffer,devsc.ep[EP_0].size); 
    if (err >= 0)
      break;
  }
  return err;
}


s_int16 get_descriptor(u_int16 addr, u_int16 type, u_int16 length) {
  
  u_int16 cnt = 0;
  u_int16 i=0;
  u_int16 chunk = 0;

  // This is always with EP0
  devsc.ep[0].dev_toggle = 1;
  devsc.ep[0].host_toggle = 0;
  
#define RCPT_DEVICE    0
#define RCPT_INTERFACE 1
#define RCPT_ENDPOINT  2
#define GET_MAX_LUN 0xFE
#define SET_BULKMASSRESET 0xFF
#define GET_DESCRIPTOR 0x06
#define GET_STATUS  0

  //  send_get_std_desc(addr,bmRequestType,bRequest,wValue,wIndex,wLength)

  switch (type) {
  case T_DEV_DESCRIPTOR:
    send_get_std_desc(addr,0x80|RCPT_DEVICE, GET_DESCRIPTOR, 0x100, 0,length);
    break;
  case T_CONF_DESCRIPTOR:
    send_get_std_desc(addr,0x80|RCPT_DEVICE, GET_DESCRIPTOR, 0x200, 0,length);
    break;
  case GET_MAX_LUN:
    send_get_std_desc(addr,0xA0|RCPT_INTERFACE,GET_MAX_LUN,0,0,1);
    break;
  case T_STATUS_DESC:
    send_get_std_desc(addr,0x80|RCPT_ENDPOINT, GET_STATUS,0,0x80|devsc.ep[EP_IN].num,2);
    break;
  }
  //   a1 fe 00 00 00 00 01 00

  i = wait_for_ack();
  
  if (i < 0) {
    printf("11NO ACK %d\n",i);
  }

  if (i == PID_STALL)
    return E_STALL;
  
  cnt++;
  
  if (cnt > DESC_TRY_LIMIT) {
    return -1;
  }

  chunk = devsc.ep[EP_0].size;
  
  {    
    u_int16 b =0;
    u_int16 off=0;
    cnt = 0;

    while (b < length) {
      b = read_ep(addr,0,buffer+off,chunk);
      if (b > 0 && b <= chunk) {
 	// it is done;
	printf("TAALLA DONE %d\n",b);
	break;
      } else if (b > 0) {
	off += b;	
      } else {
	// Error
      }
      printf("b %d,len %d, chunk %d\n",b,length,chunk);
    }
    // Bytes read = off+b
    
    send_zerodata(addr,0,PID_DATA1);
    i = wait_for_ack();
    if (i < 0) {
      printf("NO ACK %d\n",i);
      return -1;
    }
    if (i == PID_STALL)
      return -1;
    
    printf("Returning %d bytes\n",b);
    return b;
  }
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
  usbdev->ep[EP_OUT].dev=usbdev;
  usbdev->ep[EP_IN].dev=usbdev;
  
  printf("ADDR=%d %d %p %p\n",ep->dev->addr, usbdev->addr,ep->dev,usbdev);

  // Wait for some time
  for (i=0; i < 100;) {
    if (PERIP(USB_STATUS)&SOF_INT) {
      if (PERIP(USB_STATUS)&TX_INT) {
	i++;
	PERIP(USB_STATUS) = SOF_INT|TX_INT;
      }
    }
  }
  
  printf("ADDR=%d %d %p %p\n",ep->dev->addr, usbdev->addr,ep->dev,usbdev);

  // 1. Get device descriptor + MaxPacketSize
  UsbWriteToEndpoint(ep, "\p\x80\x06\x00\x01\x00\x00\x18\x00", 8, 1);
  UsbReadFromEndpoint(ep, &buffer, 8);
  UsbWriteToEndpoint(ep, NULL, 0, 0);

  ep->size = getbyte(7,buffer);

  
  // 2. Get size of configuration descriptor
  UsbWriteToEndpoint(ep, "\p\x80\x06\x00\x02\x00\x00\x09\x00", 8, 1);
  UsbReadFromEndpoint(ep, &buffer, 9);
  UsbWriteToEndpoint(ep, NULL, 0, 0);
  printf("Conf desc Total Length %d,ep0 size %d\n",SwapWord(buffer[1]),
	 ep->size);


  // 3. Read whole configuration descriptor
  UsbWriteToEndpoint(ep, "\p\x80\x06\x00\x02\x00\x00\xff\x00", 8, 1);
  len = UsbReadFromEndpoint(ep, &buffer, 0x64);
  UsbWriteToEndpoint(ep, NULL, 0, 0);

  printf("ADDR0=%d %d %p %p\n",ep->dev->addr, usbdev->addr,ep->dev,usbdev);

  parse_conf_packet(usbdev,len,buffer);

  // 4. Set Address
  UsbWriteToEndpoint(ep, "\p\x00\x05\x01\x00\x00\x00\x00\x00", 8, 1);
  len = UsbReadFromEndpoint(ep, &buffer, 0x64);

  usbdev->addr = 1;


  // 5. Set Configuration
  UsbWriteToEndpoint(ep, "\p\x00\x09\x01\x00\x00\x00\x00\x00", 8, 1);
  len = UsbReadFromEndpoint(ep, &buffer, 0x64);
 
  printf("ADDR=%d %d %p %p\n",ep->dev->addr, usbdev->addr,ep->dev,usbdev);
  return 1;

#if 0
#if 0
  {
    u_int16 n;

    UsbDeviceInfo d = OpenUsbDevice();
    // Detect device, set address, set configuration.
    // Find endpoints, populate epinfo

    EpInfo ep = OpenEndpoint(&d, n); //Clear endpoint, 
    // EpInfo: datatoggle, direction, size, {bulk|interrupt|iso}, *info


    UsbWriteToEndpoint(&ep, "\p\x80\x06\x00\x01\x00\x00\x18\x00", 8, 1);
    UsbWriteToEndpoint(&ep, "\p\x80\x06\x00\x01\x00\x00\x18\x00", 8, 1);
    UsbReadFromEndpoint(&ep, &buffer, 8);
    UsbWriteToEndpoint(&ep, "\p\x80\x06\x00\x01\x00\x00\x08\x00", 0, 0);
  }
  while (1) ;
#endif
  
#if 0
  // Do endpoint 0 size
  if (get_descriptor(0,T_DEV_DESCRIPTOR,8) != 8) {
    return -1;
  }
  parse_descriptor(0 ,buffer);

  for (i=0; i<4; i++) {
    u_int16 j;
    printf("%02d :",i*8);
    for (j=0; j<8; j++) {
      printf(" %02X %02X",buffer[i*8+j]>>8,buffer[i*8+j]&0xff);
    }
    printf("\n");
  }

  cnt = 0;

  usbdev->addr = 1;
  set_descriptor(0,0,0x05,usbdev->addr);

  while (1) {
    u_int16 b;
    i=0;

    b = get_descriptor(usbdev->addr,T_DEV_DESCRIPTOR,18);
    if (b <= 0) {
      cnt++;
    } else {
      parse_descriptor(b,buffer);
    }      
   
    if (b > 0) {
      b = get_descriptor(usbdev->addr,T_CONF_DESCRIPTOR,64);
      if (b < 0) {
	cnt++;
      } else {
	parse_conf_packet(b,buffer);
	break;
      }
    }
    
    if (cnt > CONF_TRY_LIMIT) {
      printf("Number of configuration tries: %d\n",cnt);
      return -1;
    }
  }
#endif    
  //  set_descriptor(0,SET_ADDR,);
  init_scsi();
  while (1) ;
  scsi_request(SCSI_TEST_UNIT_READY,&usbdev,buffer,0,0,0);
  scsi_request(SCSI_REQUEST_SENSE,&usbdev,buffer,usbdev->ep[EP_IN].size,0,0);


  printf("STATUS %d\n",get_descriptor(usbdev->addr,T_STATUS_DESC,64));
  scsi_request(SCSI_REQUEST_SENSE,&usbdev,buffer,usbdev->ep[EP_IN].size,0,0);

  //request_sense(usbdev->addr,usbdev->ep[EP_IN].num,usbdev->ep[EP_OUT].num);

  get_descriptor(usbdev->addr,GET_MAX_LUN,64);
  //request_sense(usbdev->addr,usbdev->ep[EP_IN].num,usbdev->ep[EP_OUT].num);
  printf("Max Lun = %X\n",buffer[0]>>8);

  // Get device capacity
  scsi_request(SCSI_READ_CAPACITY_10,&usbdev,buffer,8,0,0);
  {
    u_int32 capasity,blocksize;
    printf("Capasity  = %ld\n",((u_int32)buffer[0]<<16)|buffer[1]);
    printf("BlockSize = %ld\n",((u_int32)buffer[2]<<16)|buffer[3]);
  }


  //send_inquiry_page(0,36);
  //  test_unit_ready(usbdev->addr,usbdev->ep[EP_IN].num,usbdev->ep[EP_OUT].num);
  scsi_request(SCSI_REQUEST_SENSE,&usbdev,buffer,64,0,0);
  scsi_request(SCSI_INQUIRY,&usbdev,buffer,36,0,128);

  
  scsi_request(SCSI_READ_10,&usbdev,buffer,512,0,0x76543210);
  scsi_request(SCSI_REQUEST_SENSE,&usbdev,buffer,18,0,0);

  { 
    u_int16 i;
    s_int16 err;
    for (i=0; i < 10000; i++) {
      err = scsi_request(SCSI_READ_10,&usbdev,buffer,512,0,i);
      if (err != 512) {
	printf("ERR %d\n",err);
	scsi_request(SCSI_REQUEST_SENSE,&usbdev,buffer,13,0,0);
	while (1) ;
      }
    }
  }
  printf("SOFWAIT = %d\n",sofwait);


  while (1) ;

  return 1;
#endif
}


EpInfo  *OpenEndpoint(UsbDeviceInfo *d, u_int16 n) {
  EpInfo *epinfo = &d->ep[0];
  return (epinfo+n);

  // Clear endpoint, 
  // EpInfo: datatoggle, direction, size, {bulk|interrupt|iso}, *info


}
// One 
UsbDeviceInfo OpenUsbDevice() {
  static UsbDeviceInfo usbinfo;
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

  printf("OpenUsbDevice\n");
  init_usb_bus();
  ResetUsbBus();
  enable_autosofs(1);
  configure_device(&usbinfo);
  printf("Configured\n");
  dump_devsc(&usbinfo);

  return usbinfo;
}



// umass reset recovery 

s_int16 usb_bulk_only_reset_recovery() {
  s_int16 err;
  err = set_descriptor(devsc.addr,0x21,0xff,0);
  if (err < 0) {
    return err;
  }

  err = clear_endpoint(devsc.addr,0x80|devsc.ep[EP_IN].num);
  if (err < 0) {
    return err;
  }
  
  err = clear_endpoint(devsc.addr,devsc.ep[EP_OUT].num);
  return err;
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
  for (j=0; j < 50; j++) {
    BusyWait10();
  }


  PERIP(USB_STATUS) |= SOF_INT;  
  while (!(PERIP(USB_STATUS)&SOF_INT)) ;
  PERIP(USB_STATUS) |= SOF_INT;  

  // Set Reset state
  PERIP(USB_UTMIW) &= 0xFFF0;

  // wait_for_10ms();
  for (j=0; j < 10; j++) {
    while (! (PERIP(USB_STATUS)&SOF_INT)) ;
    PERIP(USB_STATUS) |= SOF_INT;  
  }
  
  PERIP(USB_UTMIW)  = (1<<5)|(1<<0)|2;
  PERIP(USB_CONFIG) = (1<<15)|(1<<10)|(1<<9)|(1<<7);
  PERIP(USB_CONFIG) = (1<<10)|(1<<9)|(1<<7);
  PERIP(USB_UTMIW) = 0x800C;
}


/*
 * USB line state commands
 */

void  set_usb_reset() {
  PERIP(USB_UTMIW) &= 0xFFF0;
}

void WaitSendDelay(u_int16 i) {
  volatile u_int16 d;
  for (d=0; d < i; d++) {
    USEX(0);
  }
}

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

void send_setaddr(u_int16 addr) {  

  PERIP(USB_STATUS)    = TX_INT|TIME_OUT|RX_INT;
  USEY(USB_SEND_MEM)   = 0x0005;
  USEY(USB_SEND_MEM+1) = addr<<8;
  USEY(USB_SEND_MEM+2) = 0x0000;
  USEY(USB_SEND_MEM+3) = 0x0000;

  send_cmd(0,0,PID_SETUP,0);

  PERIP(USB_EP_OUT0)   = 0x8008;
  PERIP(USB_HOST) = (PID_DATA0<<12)|SEND_PACKET;
  
  while ((PERIP(USB_STATUS) & TX_INT) == 0) ;
  PERIP(USB_STATUS) = TX_INT;  
}


void sendsof() {  
  send_cmd(0,0,PID_SOF,0); 
}

u_int16 sendsofs(u_int16 cnt) {
  u_int16 i;

  for (i=0; i < cnt; i++) {
    while ((PERIP(USB_STATUS)&SOF_INT) == 0) ;
    PERIP(USB_STATUS) = SOF_INT;  
    sendsof();
  }
  return 0;
}



void send_cmd(u_int16 addr, u_int16 ep, u_int16 pid, u_int16 flags) {  
  u_int16 old;

  // remember old sendmem data word for restore at end
  old = USEY(USB_SEND_MEM); 

  if (pid == PID_SETUP) {
    devsc.ep[EP_0].dev_toggle = 1;
    devsc.ep[EP_0].host_toggle = 0;
  }

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

void send_zerodata(u_int16 addr,u_int16 ep, u_int16 pid) {
  send_cmd(addr,ep,PID_OUT,0);
  PERIP(USB_EP_OUT0)   = 0x8000;  
  USEY(USB_SEND_MEM)   = 0x0000;
  PERIP(USB_STATUS)    = TIME_OUT|TX_INT;    
  PERIP(USB_HOST)      = (pid<<12)|SEND_PACKET;
  WaitForUsbInt(TX_INT);
}


void send_getdesc_conf(u_int16 addr, u_int16 len) {  


  PERIP(USB_STATUS)    = TX_INT|TIME_OUT|RX_INT;
  USEY(USB_SEND_MEM)   = 0x8006;
  USEY(USB_SEND_MEM+1) = 0x0002;
  USEY(USB_SEND_MEM+2) = 0x0000;
  USEY(USB_SEND_MEM+3) = len<<8;  
  

  send_cmd(addr,0,PID_SETUP,0);

  PERIP(USB_STATUS)    = TX_INT|TIME_OUT|RX_INT;
  PERIP(USB_EP_OUT0)   = 0x8008;
  PERIP(USB_HOST) = (PID_DATA0<<12)|SEND_PACKET;
  WaitForUsbInt(TX_INT);
}


void send_getdesc_dev(u_int16 addr,u_int16 len) {  
  
  PERIP(USB_STATUS)    = TX_INT|TIME_OUT|RX_INT;
  USEY(USB_SEND_MEM)   = 0x8006;
  USEY(USB_SEND_MEM+1) = 0x0001;
  USEY(USB_SEND_MEM+2) = 0x0000;
  USEY(USB_SEND_MEM+3) = 0x0000|(len<<8);  

  send_cmd(addr, 0 ,PID_SETUP,0);

  PERIP(USB_STATUS)    = TX_INT|TIME_OUT|RX_INT;
  
  PERIP(USB_EP_OUT0)   = 0x8008;
  PERIP(USB_HOST)      = (PID_DATA0<<12)|SEND_PACKET;

  WaitForUsbInt(TX_INT);
}



#if 0
u_int16 wait_for_ack() {
  
  // Either reply or timeout is expected
  volatile u_int16 w;
  
  // Wait end of timeout
  PERIP(USB_STATUS) = TIME_OUT;

  for (w=0; w < 1000; w++) {
    if (PERIP(USB_STATUS)&SOF_INT) {
      printf("SOF DURING ACK WAIT\n");
    }
    
    if ((PERIP(USB_STATUS)&RX_INT) && 
	(PERIP(USB_STATUS)&0x0F) == PID_ACK) {
      PERIP(USB_STATUS) = RX_INT|TIME_OUT|TX_INT;
      return 1;
    }
  }
  printf("NO ACK %X\n",PERIP(USB_STATUS)&0x0F);
  return 0;
}
#endif

s_int16 wait_for_ack() {
  u_int16 w;

  // Wait end of timeout
  PERIP(USB_STATUS) = TIME_OUT;
  
  // Wait end of timeout
  for (w=0; w < 100; w++) {
    if (PERIP(USB_STATUS)&SOF_INT) {
      printf("SOF DURING ACK WAIT\n");
    }

    if (PERIP(USB_STATUS)&RX_INT) {
      u_int16 r;
      PERIP(USB_STATUS) = RX_INT;
      r = PERIP(USB_STATUS)&0x0F;
      if (r == PID_ACK) {
	PERIP(USB_STATUS) = RX_INT|TIME_OUT|TX_INT;
	return 1;
      } else if (r == PID_NACK) {
	PERIP(USB_STATUS) = RX_INT|TIME_OUT|TX_INT;
	return -1;
      } else if (r == PID_STALL) {
	PERIP(USB_STATUS) = RX_INT|TIME_OUT|TX_INT;
	return E_STALL;
      }
    }
    if (PERIP(USB_STATUS)&TIME_OUT) {
      PERIP(USB_STATUS) = TIME_OUT;
      printf(" Waiting ack TimeOut\n");
      return 0;
    }
  }
  printf("XXX %04X\n",PERIP(USB_STATUS));
  return 0;
}

// getDeviceDescriptor
// send_usb_packet(0,0,PID_SETUP,GetDevDesc,8)

void send_usb_packet(u_int16 addr, u_int16 epinx, u_int16 cmd, u_int16 *d, u_int16 len) {  
  EpInfo *ep = &devsc.ep[epinx];
  u_int16 i;
  
  if (epinx > 2) {
    printf("ENDPOINT TOO BIG %d\n",epinx);
    return ;
  }

  // Copy data to output buffer
  for (i=0; i < ((len>>1)+1); i++) {
    USEY(USB_SEND_MEM+1) = d[i];
  }
  
  if (cmd == PID_SETUP) {
    // Host toggle = 0
    // Dev toggle = 1;
    ep->host_toggle = 0;
    ep->dev_toggle  = 1;
  }
  
  // Send command portion of message
  send_cmd(addr, ep->num, cmd, 0);
  
  // Correct sendbuf[0] at USB_SEND_MEM 
  // (send_cmd overwrites memory at USB_SEND_MEM)
  USEY(USB_SEND_MEM)   = d[0];
  PERIP(USB_STATUS)    = TX_INT|TIME_OUT|RX_INT;
  PERIP(USB_EP_OUT0)   = 0x8000|len;

  if (ep->host_toggle) {
    PERIP(USB_HOST)      = (PID_DATA1<<12)|SEND_PACKET;
    ep->host_toggle = 0;
  } else {
    PERIP(USB_HOST)      = (PID_DATA0<<12)|SEND_PACKET;
    ep->host_toggle = 1;
  }
  
  WaitForUsbInt(TX_INT);
}


#if 0
// Scsi layer (Not block read/write)
writeChar(dev,data,len) -> writeUSB(addr,epout,data,len);
readChar(dev,data,len) -> readUSB(addr,epin,data,len);
void send_usb_data(u_int16 addr, u_int16 ep, u_int16 *d, u_int16 len) {
}
#endif


ioresult DevUsbBlockRead(register __i0 DEVICE *dev, u_int32 firstBlock,
			  u_int16 blocks, u_int16 *data){
  devUsbHwInfo* hw=(devUsbHwInfo*)dev->hardwareInfo;
  
#if 0
  u_int16 numeps;
  u_int16 i;
  u_int16 ptr = data;

  hardwareInfo[0] = addr;
  
  // block is 256 words

  addr = hw[0];
  numeps = 512/epinSize;

  for (i=0; i < numeps; i++) {
    cnt = read_ep(addr,epIn,ptr,epinSize);        
    ptr += cnt;    
  }
#endif
}



// If read_ep  fails -> possible actions
// 1. Reset endpoint
// 2. Reset device

s_int16 write_ep(u_int16 addr,u_int16 epinx,u_int16 *buffer,u_int16 length) {
  EpInfo *ep = &devsc.ep[epinx];
  u_int16 *toggle;
  u_int16 epsize;

  epsize = ep->size;
  toggle = &ep->dev_toggle;

  do {
    u_int16 t,x;

    // Get max epsize data
    send_cmd(addr, ep->num, PID_OUT, 0);

    PERIP(USB_STATUS) = 0;
    
    x  = WaitForUsbInt(RX_INT|TIME_OUT|SOF_INT);
    if (x & SOF_INT) {
      t = 0;
    } else {
      t  = PERIP(USB_STATUS)&0x0F;
    }
  } while (1);
}

ioresult UsbWriteToEndpoint(EpInfo *ep, int *buf, 
			    u_int16 bytes, u_int16 isSetup) 
{
  
  u_int16 nack,cnt,errs;
  u_int16 p = ep->dev;
  nack = cnt = errs = 0;
  
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
      ep->dev_toggle = 1;
    }
    
    memcpyXY((__mem_y*)USB_SEND_MEM,buf,(bytes+1)>>1);
    
    PERIP(USB_EP_OUT0)   = 0x8000|bytes;  
    
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
      printf("SOF AT WRONG PLACE %d\n",GET_SOF_CNT());
      // WaitForUsbInt(TX_INT);      
    } else if (x & RX_INT) {
      t  = PERIP(USB_STATUS)&0x0F;
    } else {
      t = 0;
    }
    
    // Wait for timeout or RX_INT
    switch (t) {

    case PID_NACK:
#if 1
      if (nack > 5) {
	CheckSOF(1);
      }
#endif
      nack++;
      break;
      
    case PID_STALL:
      printf("STALL\n");
      return E_STALL;
      break;
    case PID_ACK:
      ep->host_toggle = ep->host_toggle?0:1;
      return bytes;
    default:
      break;
    }
    
    // If no success -> return
    if (nack > 1500) {
      printf("NACKS %d > 1000\n",nack);
      return E_FATAL;
    }
    if (cnt > 5000) {
      printf("CNT > 1000\n");
      return E_AGAIN;
    }
    if (errs > 1000) {
      printf("errs > 100\n");
      return E_FATAL;
    }
  } while (1);

  return E_FATAL;
}

// ioresult UsbReadFromEndpoint(devAddr, ep, &buf, bytes);

ioresult UsbReadFromEndpoint(EpInfo *ep, int *buf, u_int16 bytes)
{
  EpInfo *epinfo;
  u_int16 ok,cnt,nack,i;
  u_int16 errs = 0;
  u_int16 *toggle;
  u_int16 epsize;
  u_int16 t,x,amount;
  int *p;

  toggle = &ep->dev_toggle;
  epsize = ep->size;
  p = buf;
  amount = 0;
  cnt  = 0;
  nack = 0;
  ok   = 0;

  printf("Reading %d bytes from endpoint %d %d\n",bytes,ep->num,*toggle);

  do {
    cnt++;
    PERIP(USB_WRPTR) = 0;
    PERIP(USB_RDPTR) = 0;

    // Get max epsize data
    
    send_cmd(ep->dev->addr, ep->num, PID_IN, 0);

    x  = WaitForUsbInt(RX_INT|TIME_OUT|SOF_INT);

    if (x & SOF_INT) {
      t = 0;
      printf("SOF AT WRONG PLACE %d\n",GET_SOF_CNT());
      // WaitForUsbInt(TX_INT);
      
    } else if (x & RX_INT) {
      t  = PERIP(USB_STATUS)&0x0F;
    } else {
      t = 0;
    }
    
    // Wait for timeout or RX_INT
    switch (t) {

    case PID_NACK:
#if 1
      if (nack > 5) {
	CheckSOF(1);
      }
#endif
      
      nack++;
      break;

    case PID_STALL:
      printf("STALL\n");
      return E_STALL;
      break;

    case PID_DATA0:
      if (*toggle != 0) {
	// wrong_dev toggle
	// device may have missed our ack 
	// Discard data -> Send new in
	errs++;
	printf("Wrong data\n");
      } else {
	u_int16 numbytes = (USEY(PERIP(USB_RDPTR)+USB_RECV_MEM)&0x03FF);
	u_int16 lenW = (numbytes+1)>>1;
	*toggle = 1;

	amount += numbytes;

	for (i=0; i < lenW && bytes >= 0; i++) {
	  *p++ = USEY(i+USB_RECV_MEM+1);
	}
	if (amount >= bytes || numbytes < epsize) {
	  printf("Return %d %d %d %d\n",bytes,numbytes,epsize,amount);
	  return amount;
	}
      }
      break;

    case PID_DATA1:
      if (*toggle != 1) {
	errs++;
	// Discard data
	printf("Wrong data\n");
      } else {
	u_int16 numbytes = (USEY(PERIP(USB_RDPTR)+USB_RECV_MEM)&0x03FF);
	u_int16 lenW = (numbytes+1)>>1;
	//read_data();
	*toggle = 0;
	amount += numbytes;

	for (i=0; i < lenW && bytes >= 0; i++) {
	  *p++ = USEY(i+USB_RECV_MEM+1);
	}
	if (amount >= bytes || numbytes < epsize) {
	  printf("Return %d %d %d %d\n",bytes,numbytes,epsize,amount);
	  return amount;
	}
      }
    default:
      break;
    }

    // If no success -> return
    if (nack > 1500) {
      printf("NACKS %d > 1000\n",nack);
      return E_FATAL;
    }
    if (cnt > 5000) {
      printf("CNT > 1000\n");
      return E_AGAIN;
    }
    if (errs > 1000) {
      printf("errs > 100\n");
      return E_FATAL;
    }
  } while (ok == 0);

  return E_FATAL;
}



ioresult read_ep(u_int16 addr,u_int16 epinx,u_int16 *buf,u_int16 length) {
  EpInfo *ep = &devsc.ep[epinx];
  u_int16 ok,cnt,nack,i;
  u_int16 errs = 0;
  u_int16 *toggle;
  u_int16 epsize;
  u_int16 t,x;

  epsize = ep->size;
  toggle = &ep->dev_toggle;

  cnt  = 0;
  nack = 0;
  ok   = 0;

  do {
    cnt++;
    PERIP(USB_WRPTR) = 0;
    PERIP(USB_RDPTR) = 0;

    // Get max epsize data
    
    send_cmd(addr, ep->num, PID_IN, 0);

    x  = WaitForUsbInt(RX_INT|TIME_OUT|SOF_INT);

    if (x & SOF_INT) {
      t = 0;
      printf("SOF AT WRONG PLACE %d\n",GET_SOF_CNT());
      // WaitForUsbInt(TX_INT);
      
    } else if (x & RX_INT) {
      t  = PERIP(USB_STATUS)&0x0F;
    } else {
      t = 0;
    }
    
    // Wait for timeout or RX_INT
    switch (t) {

    case PID_NACK:
#if 1
      if (nack > 5) {
	CheckSOF(1);
      }
#endif
      
      nack++;
      break;

    case PID_STALL:
      printf("STALL\n");
      return E_STALL;
      break;

    case PID_DATA0:
      if (*toggle != 0) {
	// wrong_dev toggle
	// device may have missed our ack 
	// Discard data -> Send new in
	errs++;
	printf("Wrong data\n");
      } else {
	u_int16 numbytes = (USEY(PERIP(USB_RDPTR)+USB_RECV_MEM)&0x03FF);
	u_int16 lenW = (numbytes+1)>>1;
	*toggle = 1;
	ok = 1;
	for (i=0; i < lenW; i++) {
	  buf[i] =  USEY(i+USB_RECV_MEM+1);
	}
	return numbytes;
      }
      break;

    case PID_DATA1:
      if (*toggle != 1) {
	errs++;
	// Discard data
	printf("Wrong data\n");
      } else {
	u_int16 numbytes = (USEY(PERIP(USB_RDPTR)+USB_RECV_MEM)&0x03FF);
	u_int16 lenW = (numbytes+1)>>1;
	//read_data();
	*toggle = 0;
	ok = 1;
	for (i=0; i < lenW; i++) {
	  buf[i] =  USEY(i+USB_RECV_MEM+1);
	}
	return numbytes;
      }
    default:
      break;
    }

    // If no success -> return
    if (nack > 1500) {
      printf("NACKS %d > 1000\n",nack);
      return E_FATAL;
    }
    if (cnt > 5000) {
      printf("CNT > 1000\n");
      return E_AGAIN;
    }
    if (errs > 1000) {
      printf("errs > 100\n");
      return E_FATAL;
    }
  } while (ok == 0);

  return E_FATAL;
}
