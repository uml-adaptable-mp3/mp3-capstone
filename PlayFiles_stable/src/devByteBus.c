/// \file devByteBus.c VsOS Device Driver for Byte-width Bus (used for LCD display and nand flash)
/// \author Panu-Kristian Poiksalo, VLSI Solution oy

#include <stdlib.h>
#include <string.h>
#include "vsos.h"
#include "devByteBus.h"	
#include "vs1005h.h"
#include "vo_gpio.h"
#include "iochannel.h"
#if 0
#include "forbid_stdout.h"
#else
#include <stdio.h>
#endif
#include "hwLocks.h"

/// The start address word of 1.5kW perip mem for the device
#define HLB_BYTEBUS HLB_5
#define __BYTEBUS_PERIP_MEM_BASE (0x0100*HLB_BYTEBUS)


const DEVICE devByteBusDefaults = {	
	0, //u_int16   flags; //< present, block/char
	DevByteBusIdentify, //char*    (*Identify)(void *obj, char *buf, u_int16 bufsize);	 
	DevByteBusCreate, //ioresult (*Create)(DEVICE *dev, char *name, struct filesystem_descriptor *fs);
	DevByteBusDelete, //ioresult (*Delete)(DEVICE *dev);
	DevByteBusIoctl, //ioresult (*Ioctl)(DEVICE *dev, s_int16 request, char *argp); //Start, Stop, Flush, Check Media
	// Stream operations	
	DevByteBusRead, //u_int16  DevByteBusRead(register __i0 DEVICE *f, void *buf, u_int16 destinationIndex, u_int16 bytes); ///< Read bytes to *buf, at byte index
	DevByteBusWrite, //u_int16  DevByteBusWrite(register __i0 DEVICE *f, void *buf, u_int16 sourceIndex, u_int16 bytes); ///< Write bytes from *buf at byte index
	// Block device operations
	0, //DevByteBusBlockRead, //ioresult (*BlockRead)(DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data);
	0, //DevByteBusBlockWrite, //ioresult (*BlockWrite)(DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data);
	//FILESYSTEM *fs;
	//DIRECTORY *root; //Zero if the device cannot hold directories
	//u_int16  deviceInstance;
	//u_int16  deviceInfo[10]; // For filesystem use, size TBD	
};

char* DevByteBusIdentify(register __i0 void *obj, char *buf, u_int16 bufsize){
	return "Bytewide Bus";
}

#define __NF_ALE_PIN 0x0f
#define __NF_CLE_PIN 0x0e

ioresult DevByteBusCreate (register __i0 DEVICE *dev, void *name, u_int16 extraInfo) {
	devByteBusHwInfo* hw=(devByteBusHwInfo*)dev->hardwareInfo;	
	memcpy(dev, &devByteBusDefaults, sizeof(*dev));
	dev->deviceInstance = __nextDeviceInstance++;	

	// Initialize the hardware info
	memcpy(hw, name, sizeof(devByteBusHwInfo));
	hw->regs->conf = hw->clockDivider;
		
	// Perip pins config for known hardware SPI peripherals SPI0 and SPI1
	PERIP(GPIO0_MODE) |= 0x07ff; //Set GPIO0 pins [10:0] as peripherals
	//PERIP(GPIO0_MODE) &= ~0x07ff; //Set GPIO0 pins [10:0] as GPIO pins
	//PERIP(GPIO0_DDR) |= 0x06ff; //Set GPIO0 pins [10:9,7:0] as outputs
	
	GpioSetPin(__NF_ALE_PIN, 0); //Set ALE pin as output, pull low
	GpioSetPin(__NF_CLE_PIN, 0); //Set CLE pin as output, pull low
		
	if (hw->csPin == 0xffffU) { //use IO channel select
		INITIALIZE_IO_CHANNEL_IDLE(); //switch GPIO pins to output, pull high
	} else {
		GpioSetPin(hw->csPin, 1); //cs pin high
	}

	dev->flags = __MASK_PRESENT | __MASK_OPEN | __MASK_READABLE | __MASK_WRITABLE;
	return S_OK;
}

	
ioresult DevByteBusDelete(register __i0 DEVICE *dev) {
	devByteBusHwInfo* hw=(devByteBusHwInfo*)dev->hardwareInfo;
	dev->flags = 0;
	return S_OK;
}

#define __BB_WAIT_IDLE() {while(PERIP(NF_CTRL) & NF_CTRL_ENA);}




// Notice: last byte goes to LSB of word. Whole words are big-endian.

/// \todo Now requires that destinationIndex = 0 for block transfers.
u_int16  DevByteBusRead(register __i0 DEVICE *dev, void *buf, u_int16 destinationIndex, u_int16 bytes){ ///< Read bytes to *buf, at byte index
	devByteBusHwInfo* hw=(devByteBusHwInfo*)dev->hardwareInfo;
	//printf(" ByteBusInput:%dBytes ",bytes);	
	if (!buf) { // No buffer, return single byte read value
		//return ByteBusTransfer(0xffffU, 8, &(hw->pins));
	}

	while(bytes) {
		u_int16 j = (bytes > 512) ? 512 : bytes; //max 512 bytes per transfer
		PERIP(NF_LEN) = j; //length in bytes
		PERIP(NF_PTR) = __BYTEBUS_PERIP_MEM_BASE;
		PERIP(NF_CTRL) = NF_CTRL_READSEL | NF_CTRL_USEPERIP | NF_CTRL_ENA;
		bytes -= j;
		j = (j+1) / 2; //j is now amount of words to transfer		
		__BB_WAIT_IDLE();

		PERIP(XP_CF) &= ~XP_CF_WRBUF_ENA;
		PERIP(XP_CF) |= XP_CF_RDBUF_ENA;
		PERIP(XP_ADDR) = __BYTEBUS_PERIP_MEM_BASE;
#pragma msg 92 off //Dummy read triggers a compiler warning	
		PERIP(XP_IDATA); //Dummy read
		PERIP(XP_IDATA); //Dummy read
#pragma msg 92 on
		{
			u_int16 *wbuf = (u_int16*)buf;
			u_int16 i;
			for (i=0; i<j; i++) {
				*wbuf++ = PERIP(XP_IDATA);
			}
			buf = wbuf;
		}
	}												
	PERIP(XP_CF) &= ~XP_CF_WRBUF_ENA;
	return S_OK;
}

/*
			PERIP(GPIO1_SET_MASK) = (1 << 6); //Feature LED on
			PERIP(GPIO1_CLEAR_MASK) = (1 << 6); //Feature LED off
*/


/// \todo Now requires that sourceIndex = 0 for block transfers.
u_int16  DevByteBusWrite(register __i0 DEVICE *dev, void *buf, u_int16 sourceIndex, u_int16 bytes){ ///< Write bytes from *buf at byte index
// Notice: last byte is written from LSB of word. Whole words are big-endian.
	//devByteBusHwInfo* hw=(devByteBusHwInfo*)dev->hardwareInfo;		
  //  printf("b=%3d\n", bytes);
	while(bytes) {
		if (bytes > 2) {
			u_int16 j = (bytes > 512) ? 512 : bytes; //max 512 bytes per transfer			
			u_int16 i;			
			j &= ~1;
			PERIP(NF_LEN) = j; //length in bytes
			bytes -= j;
			j >>= 1; //j is now amount of words to transfer
			PERIP(XP_CF) &= ~XP_CF_RDBUF_ENA;
			PERIP(XP_CF) |= XP_CF_WRBUF_ENA;
			PERIP(XP_ADDR) = __BYTEBUS_PERIP_MEM_BASE;
			if (buf) {
				u_int16 *wbuf = (u_int16*)buf;
				for (i=0; i<j; i++) {
					PERIP(XP_ODATA) = *wbuf++;
				}
				buf = wbuf;
			} else {
				for (i=0; i<j; i++) {
					PERIP(XP_ODATA) = sourceIndex;
				}			
			}
			PERIP(NF_PTR) = __BYTEBUS_PERIP_MEM_BASE;
			PERIP(NF_CTRL) = NF_CTRL_USEPERIP | NF_CTRL_ENA;
			//This would be nice, but without it the compiler uses the loop hw for the xfer to fifomem
			//(if this is not commented out, the compiler spills wbuf which devastates the performance)
#if 0
			if (__F_OVERLAPPED(dev) && (bytes==0)) {
				return S_OK; //In overlapped mode, return without waiting for the transfer to complete
			}
#endif
			__BB_WAIT_IDLE();											
		} else if (bytes > 1) {
			bytes-=2;
			PERIP(XP_ODATA) = buf ? *(u_int16 *)buf>>8 : sourceIndex>>8;
			PERIP(NF_CTRL) = NF_CTRL_ENA;
			__BB_WAIT_IDLE();
			PERIP(XP_ODATA) = buf ? *(u_int16 *)buf : sourceIndex;
			PERIP(NF_CTRL) = NF_CTRL_ENA;
			{
				u_int16 *wbuf = buf;
				wbuf++;
				buf = wbuf;
			}
		} else {
			bytes--;
			PERIP(XP_ODATA) = buf ? *(u_int16 *)buf : sourceIndex;
			PERIP(NF_CTRL) = NF_CTRL_ENA;
		}
	}
	__BB_WAIT_IDLE(); //Removal of this also causes slowdown
	return S_OK;
}


ioresult DevByteBusIoctl(register __i0 DEVICE *dev, s_int16 request, char *argp) {
	devByteBusHwInfo* hw=(devByteBusHwInfo*)dev->hardwareInfo;
	//printf(" ByteBusIoctl:%d ",request);

	if (__F_OVERLAPPED(dev)){
		__BB_WAIT_IDLE();
		dev->flags &= ~(__MASK_OVERLAPPED);
	}
	switch (request) {
		case IOCTL_START_FRAME:
			if (hw->csPin == 0xffffU) { //use IO channel select
				ObtainHwLocksBIP(HLB_BYTEBUS, HLIO_NF|HLIO_CS_SELECT, HLP_NAND|HLP_XPERIP_IF);
				SELECT_IO_CHANNEL(hw->ioChannel);
			} else {
				ObtainHwLocksBIP(HLB_BYTEBUS, HLIO_NF, HLP_NAND|HLP_XPERIP_IF);
				GpioSetPin(hw->csPin, 0); //cs pin high
			}	
			break;
			
		case IOCTL_END_FRAME:
			if (hw->csPin == 0xffffU) { //use IO channel select
				SELECT_IO_CHANNEL_IDLE();
				ReleaseHwLocksBIP(HLB_BYTEBUS, HLIO_NF|HLIO_CS_SELECT, HLP_NAND|HLP_XPERIP_IF);
			} else {
				ReleaseHwLocksBIP(HLB_BYTEBUS, HLIO_NF, HLP_NAND|HLP_XPERIP_IF);
				GpioSetPin(hw->csPin, 1); //cs pin high
			}
		
		case IOCTL_WAIT_UNTIL_TX_IDLE:
			break;
		
		case IOCTL_TEST:
			GpioSetPin(0x16, 1);
			while (1) {
				while(PERIP(NF_CTRL) & NF_CTRL_ENA)
				  ; // Do nothing
				//PERIP(GPIO1_CLEAR_MASK) = (1 << 6); //Feature LED off
				PERIP(XP_ODATA) = 0x55aa;
				PERIP(NF_CTRL) = NF_CTRL_ENA;
				//PERIP(GPIO1_SET_MASK) = (1 << 6); //Feature LED on
			}
			break;
			
		default:
			return S_ERROR;
			break;
	}
	return S_OK;
}
	

