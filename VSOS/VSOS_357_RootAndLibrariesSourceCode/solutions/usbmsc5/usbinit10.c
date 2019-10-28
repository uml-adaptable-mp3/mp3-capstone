
#include <vs1010b.h>

#include "usbinit.h"

const __mem_y u_int16 usbHwInitTab10[] = { //Regisgter writes to initialize USB hardware
	USB_CF, 	USB_CF_USBENA | USB_CF_NOHIGHSPEED | USB_CF_RST,
 	USB_CF, 	USB_CF_USBENA | USB_CF_NOHIGHSPEED | USB_CF_DTOGERR,
	USB_EP_ST0, USB_EP_ST_OTYP_BULK | USB_EP_ST_OENA | USB_EP_ST_ITYP_BULK | USB_EP_ST_IENA,
	USB_EP_ST2, USB_EP_ST_ITYP_BULK | USB_EP_ST_IENA,
	USB_EP_ST3, USB_EP_ST_OTYP_BULK | USB_EP_ST_OENA,
	USB_ST, 	0xffff,
	0
};


void ResetUsbHardware() {
	PERIP(ANA_CF3) |= ANA_CF3_UTMBIAS | ANA_CF3_480_ENA | ANA_CF3_UTM_ENA;
	DelayL(10);
	//printf("\nBRST\n");	
	PERIP(ANA_CF3) &= ~ANA_CF3_480_ENA;
	
	
	usbsender.writePackets = 0;
	{  //Execute set of Y register writes from tabarray
		__mem_y u_int16 *p = usbHwInitTab;
		while (*p) {
			u_int16 r = *p++;
			u_int16 v = *p++;
			PERIP(r) = v;
		}
	}
}

void SetCoreClockVCO(register u_int32 vco_sdm_value) {
	PERIP(ANA_CF0) = ANA_CF0_HIGH_REF | ANA_CF0_REF_ENA;
	DelayL(10000); // SRD, Some Random Delay

	PERIP(VCO_CCF_HI) = vco_sdm_value >> 16;
	PERIP(VCO_CCF_LO) = vco_sdm_value;
	PERIP(ANA_CF1) |= ANA_CF1_VCO_ENA;
	PERIP(ANA_CF3) = ANA_CF3_SDM_ENA;
	
	{
		u_int16 d = 6;	
		while(1) {
			u_int16 ana_cf3 = PERIP(ANA_CF3) & ~0xf;
			ana_cf3 |= (d & 0xf);
			PERIP(ANA_CF3) = ana_cf3;
			DelayL(10000); // SRD
			PERIP(ANA_CF3) |= ANA_CF3_LCKCHK;
			PERIP(ANA_CF3) &= ~ANA_CF3_LCKCHK;
			DelayL(10000); // SRD
			if (PERIP(ANA_CF3) & ANA_CF3_LCKST) {
				break; //VCO locked, exit.
			}
			d++;
		}
	}	
	PERIP(CLK_CF) &= ~(CLK_CF_FORCEPLL | 0xf); //Switch to 1X
	PERIP(CLK_CF) |= CLK_CF_USBCLK | 0x1; //Select USB as core clock source
	PERIP(CLK_CF) |= (CLK_CF_FORCEPLL); // Switch on PLL 
	
}



void InitUsbHardware() {
	SetCoreClockVCO(0xFF87FFFF);
	Disable();	InitSpiVideo(14); Enable();
	Disable(); DelayL(10000); Enable(); PERIP(SPI1_DATA) = 0;
	printf("Relocked (%04x)\n",PERIP(ANA_CF3));	
}
