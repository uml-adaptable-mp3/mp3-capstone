
#include <vs1005h.h>

#include "usbinit.h"
#include <clockspeed.h>
#include <string.h>
extern u_int16 usbuf[];

const __mem_y u_int16 usbHwInitTab[] = { //Regisgter writes to initialize USB hardware
	USB_CF, 	USB_CF_USBENA | USB_CF_NOHIGHSPEED | USB_CF_RST,
 	USB_CF, 	USB_CF_USBENA | USB_CF_NOHIGHSPEED | USB_CF_DTOGERR,
	USB_EP_ST0, USB_EP_ST_OTYP_BULK | USB_EP_ST_OENA | USB_EP_ST_ITYP_BULK | USB_EP_ST_IENA,
	USB_EP_ST2, USB_EP_ST_ITYP_BULK | USB_EP_ST_IENA,
	USB_EP_ST3, USB_EP_ST_OTYP_BULK | USB_EP_ST_OENA,
	USB_ST, 	0xffff,
	0
};

void ResetUsbHardware() {	
	PERIP(ANA_CF2) |= ANA_CF2_REF_ENA | ANA_CF2_UTM_ENA | ANA_CF2_2G_ENA;
	PERIP(ANA_CF3) |= ANA_CF3_480_ENA | ANA_CF3_UTMBIAS;
	memset(&usbsender,0,sizeof(usbsender));
	usbsender.diskDataPtr = &usbuf[0];
	//usbsender.writePackets = 0;
	//usbsender.incomingPackets = 0;
	{  //Execute set of Y register writes from tabarray
		__mem_y u_int16 *p = usbHwInitTab;
		while (*p) {
			u_int16 r = *p++;
			u_int16 v = *p++;
			PERIP(r) = v;
		}
	}
}

void InitUsbHardware() {
	SetClockSpeed(SET_CLOCK_USB); // 60 MHz from VCO, usb compatible, uses more power
}
