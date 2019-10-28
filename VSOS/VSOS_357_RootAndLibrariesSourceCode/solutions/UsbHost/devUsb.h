
#ifndef DevUsb_H
#define DevUsb_H

#define LINE_J 0x01
#define LINE_K 0x02

#define USB_CFF_RESET  (1<<15)
#define USB_CFF_NOHIGHSPEED (1<<11)
#define USB_CFF_MASTER (1<<9)
#define USB_CFF_ENABLE (1<<7)

#define SEND_PACKET    (1<<9)
#define TIME_OUT       (1<<9)

//
// LOWLEVEL USB
//

struct  _usb_device_info_;

typedef struct {
   u_int16 dev_toggle,    // next expected data toggle from device
     host_toggle,         // next data toggle to device
     num,                 // Endpoint number
     size;                // Endpoint size
  struct _usb_device_info_ *dev;
} EpInfo;

struct _usb_device_info_ {
  u_int16 addr;
  u_int16 iface;
  u_int16 maxlun;
  EpInfo  ep[3];          // Hard coded endpoint numers 0 is CONTROL, 1 is IN, 2 is OUT
} ;

typedef struct _usb_device_info_ UsbDeviceInfo ;


ioresult UsbWriteToEndpoint(register EpInfo *ep, register int *buf, 
			   register u_int16 bytes, register u_int16 isSetup) ;

s_int16 UsbReadFromEndpoint(register EpInfo *ep, register int *buf, 
			     register u_int16 bytes);

//ioresult OpenUsbDevice(UsbDeviceInfo *);
EpInfo  *OpenEndpoint(UsbDeviceInfo *d, u_int16 n);



#endif
