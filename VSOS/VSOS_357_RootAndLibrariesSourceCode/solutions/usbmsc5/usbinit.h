#ifndef USBINIT_H
#define USBINIT_H

#define USB_EP_ST_OTYP_BULK (0<<14)
#define USB_EP_ST_OTYP_INT  (1<<14)
#define USB_EP_ST_OTYP_ISO  (3<<14)
#define USB_EP_ST_ITYP_BULK (0<<6)
#define USB_EP_ST_ITYP_INT  (1<<6)
#define USB_EP_ST_ITYP_ISO  (3<<6)

extern struct UsbSender {
	u_int16 ep;
	u_int16 bytes;
	__mem_y u_int16 *buf;
	//u_int16 incomingPackets;
	u_int16 expectedDiskPackets;
	u_int16 *diskDataPtr;
	//__mem_y u_int16 *writePtrY;
	u_int32 lba;
	u_int16 writeSectors;
	u_int16 readSectors;
	u_int16 sendCSW;
} usbsender;


void ResetUsbHardware();
void InitUsbHardware();

#endif
