/// \file devHwSpi.c VsOS Device Driver for VS1005 Hardware SPI Port
/// \author Panu-Kristian Poiksalo, VLSI Solution oy

/// \todo support FIFO in receive mode

#include <stdlib.h>
#include <string.h>
#include <hwLocks.h>
#include "vsos.h"
#include "devHwSpi.h"
#include "vo_fat.h"	
#include "vs1005h.h"
#include "vo_gpio.h"
#include "iochannel.h"
#include "forbid_stdout.h"



const DEVICE devHwSpiDefaults = {	
	//	vo_flags   flags; ///< VSOS Flags
	0,
	//	char*    (*Identify)(register __i0 void *self, char *buf, u_int16 bufsize); ///< Return some kind of decorative name for this object.
	DevHwSpiIdentify,
	//	ioresult (*Create)(register __i0 DEVICE *self, const void *name, u_int16 extraInfo); ///< Start device, populate descriptor, find and start filesystem.
	DevHwSpiCreate,
	//	ioresult (*Delete)(register __i0 DEVICE *self); ///< Flush, Stop, Clean and Free the descriptor of the device
	DevHwSpiDelete,
	//	ioresult (*Ioctl)(register __i0 DEVICE *self, s_int16 request, char *argp); ///< Reset, Start, Stop, Restart, Flush, Check Media etc
	DevHwSpiIoctl,
	// Stream operations
	//	u_int16  (*Read) (register __i0 DEVICE *self, void *buf, u_int16 destinationIndex, u_int16 bytes); ///< Read bytes to *buf, at byte index
	//DevHwSpiRead,
	(void *)CommonOkResultFunction,
	//	u_int16  (*Write)(register __i0 DEVICE *self, void *buf, u_int16 sourceIndex, u_int16 bytes); ///< Write bytes from *buf at byte index
	DevHwSpiWrite,
	// Block device operations
	//	ioresult (*BlockRead)(register __i0 DEVICE *self, u_int32 firstBlock, u_int16 blocks, u_int16 *data); ///< Read block at LBA
	(ioresult(*)(register __i0 DEVICE*,u_int32,u_int16,u_int16*)) CommonErrorResultFunction,
	//	ioresult (*BlockWrite)(register __i0 DEVICE *self, u_int32 firstBlock, u_int16 blocks, u_int16 *data); ///< Write block at LBA
	(ioresult(*)(register __i0 DEVICE*,u_int32,u_int16,u_int16*)) CommonErrorResultFunction,
	//	FILESYSTEM *fs; ///< pointer to the filesystem driver for this device
	//	u_int16  deviceInstance; ///< identifier to detect change of SD card etc
	//	u_int16  hardwareInfo[6]; ///< Device driver's info of this hardware
	//	u_int16  deviceInfo[16]; ///< Filesystem driver's info of this device		
};



static char* deviceName = "Hardware SPI Port";
char* DevHwSpiIdentify(register __i0 void *obj, char *buf, u_int16 bufsize){
	return deviceName;
}


//SPI config value 0x0baf=fifo,master,8bit,idle_hi
//#define __HWSPI_8BIT_MASTER 0x3af
//#define __HWSPI_16BIT_MASTER 0x3bf
#define __HWSPI_8BIT_MASTER 0x1af
#define __HWSPI_16BIT_MASTER 0x1bf
#define __HWSPI_WAIT_UNTIL_TX_IDLE() {	while (hw->regs->status & SPI_ST_TXRUNNING); } 
#define __HWSPI_WAIT_UNTIL_TX_FIFO_NOT_FULL() {	while ((hw->regs->status & SPI_ST_TXFIFOFULL)){/*printf("ST=%04x ",hw->regs->status);*/} }


ioresult DevHwSpiCreate (register __i0 DEVICE *dev, const void *name, u_int16 extraInfo) {
	devHwSpiHwInfo* hw=(devHwSpiHwInfo*)dev->hardwareInfo;	
	memcpy(dev, &devHwSpiDefaults, sizeof(*dev));
	dev->deviceInstance = __nextDeviceInstance++;	
	
	// Initialize the hardware info
	memcpy(hw, name, sizeof(devHwSpiHwInfo));

	// Init here
	// printf("IO Channel:%d\n",hw->ioChannel);
	// printf("Config:%04x\n",hw->regs->config);
	// hw->regs->config = 0x0baf; 
	hw->regs->config = __HWSPI_16BIT_MASTER + SPI_CF_SRESET + SPI_CF_TXFIFO_ENA;
	// printf("Config:%04x\n",hw->regs->config);
	hw->regs->clkconfig = (hw->clockDivider << 2); //divider-1 (0=div1); CLKPOL=0, CLKPHASE=0.
	
	// Perip pins config for known hardware SPI peripherals SPI0 and SPI1
	if ((u_int16)(hw->regs) == SPI0_CF) {
		PERIP(GPIO1_MODE) |= (1<<1)|(1<<2)|(1<<3); //Set GPIO0 pins 1,2,3 as peripheral pins.
	}
	if ((u_int16)(hw->regs) == SPI1_CF) {
		//printf(" HERE\n");
		PERIP(GPIO1_MODE) |= (1<<5)|(1<<6)|(1<<7); //Set GPIO0 pins 1,2,3 as peripheral pins.
	}
	if (hw->csPin == 0xffffU) { //use IO channel select
		SELECT_IO_CHANNEL_IDLE();
	} else {
		GpioSetPin(hw->csPin, 1); //cs pin high
	}

	dev->flags = __MASK_PRESENT | __MASK_OPEN | __MASK_READABLE | __MASK_WRITABLE;
	return S_OK;
}


// Shift n bits in and out using a hardware SPI port
u_int16 HwSpiTransfer (u_int16 outdata, u_int16 nbits, void *pins) {
	u_int16 indata=0;
	return indata; // Return the result byte to caller
}
	
ioresult DevHwSpiDelete(register __i0 DEVICE *dev) {
	devHwSpiHwInfo* hw=(devHwSpiHwInfo*)dev->hardwareInfo;
	dev->flags = 0;
	return S_OK;
}


u_int16 DevHwSpiRead(register __i0 DEVICE *dev, void *buf, u_int16 destinationIndex, u_int16 bytes) {
	devHwSpiHwInfo* hw=(devHwSpiHwInfo*)dev->hardwareInfo;
	u_int16 *wbuf = buf;
	u_int16 w[2];
	__HWSPI_WAIT_UNTIL_TX_IDLE();
	//printf(" HwSpiInput:%dBytes ",bytes);	
	if (!buf) { // No buffer, return single byte read value
		hw->regs->config = __HWSPI_8BIT_MASTER;
		hw->regs->data = 0xff; //send 0xff
		__HWSPI_WAIT_UNTIL_TX_IDLE();
		return hw->regs->data & 0xff;
	}
	//printf("di=%d.",destinationIndex);
	while (bytes) {
		if (bytes == 1) {
			__HWSPI_WAIT_UNTIL_TX_IDLE();
			//printf(" HwSpiInput:%dBytes ",bytes);
			hw->regs->config = __HWSPI_8BIT_MASTER;
			hw->regs->data = 0xffff; //send 0xff
			__HWSPI_WAIT_UNTIL_TX_IDLE();					
			w[0] = hw->regs->data & 0xff;			
			MemCopyPackedBigEndian(buf, destinationIndex, w, 1, 1);
			bytes--;
		} else {
			if (hw->regs->config != __HWSPI_16BIT_MASTER) {
				/*Must wait until end of transmission to switch from 8 bit to 16 bit mode*/
				hw->regs->config = __HWSPI_16BIT_MASTER;
				__HWSPI_WAIT_UNTIL_TX_IDLE();
			}
			__HWSPI_WAIT_UNTIL_TX_FIFO_NOT_FULL();
			hw->regs->data = 0xffff;
			if (destinationIndex) {
				w[0] = hw->regs->data;
				MemCopyPackedBigEndian(wbuf++, destinationIndex, w, 0, 2);
			} else {
				*wbuf++ = hw->regs->data;
			}
			bytes-=2;
		}			
	}
	return S_OK;
}

u_int16 DevHwSpiWrite(register __i0 DEVICE *dev, void *buf, u_int16 sourceIndex, u_int16 bytes) {
	devHwSpiHwInfo* hw=(devHwSpiHwInfo*)dev->hardwareInfo;
	//printf(" HwSpiOutput:%dBytes ",bytes);	
	if (!buf) { //Send bytes copies of byte sourceIndex
		/*Must wait until end of transmission to switch to 8 bit mode*/
		if (hw->regs->config != __HWSPI_8BIT_MASTER) {
			/*Must wait until end of transmission to switch from 8 bit to 16 bit mode*/
			__HWSPI_WAIT_UNTIL_TX_IDLE();
			hw->regs->config = __HWSPI_8BIT_MASTER;
		}
		while(bytes--) {
			__HWSPI_WAIT_UNTIL_TX_FIFO_NOT_FULL();
			hw->regs->data = sourceIndex; //send byte from "sourceIndex" parameter		
		}
		return S_OK;
	}
	while (bytes) {
		u_int16 w=0;
		if (bytes == 1) {
			MemCopyPackedBigEndian(&w, 1, buf, sourceIndex++, 1);
			/*Must wait until end of transmission to switch to 8 bit mode*/
			__HWSPI_WAIT_UNTIL_TX_IDLE();
			hw->regs->config = __HWSPI_8BIT_MASTER;
			hw->regs->data = w;
			bytes--;
		} else {
			MemCopyPackedBigEndian(&w, 0, buf, sourceIndex, 2);
			sourceIndex+=2;
			bytes-=2;
			if (hw->regs->config != __HWSPI_16BIT_MASTER) {
				/*Must wait until end of transmission to switch from 8 bit to 16 bit mode*/
				__HWSPI_WAIT_UNTIL_TX_IDLE();
				hw->regs->config = __HWSPI_16BIT_MASTER;
			}
			__HWSPI_WAIT_UNTIL_TX_FIFO_NOT_FULL();
			hw->regs->data = w;
		}			
	}	
	return S_OK;
}

ioresult DevHwSpiIoctl(register __i0 DEVICE *dev, s_int16 request, char *argp) {
	devHwSpiHwInfo* hw=(devHwSpiHwInfo*)dev->hardwareInfo;
	//printf(" HwSpiIoctl:%d ",request);
	switch (request) {
		case IOCTL_START_FRAME:
			ObtainHwLocksBIP(HLB_NONE, HLIO_CS_SELECT, HLP_NONE);
			hw->hasHwResources = 1;
			__HWSPI_WAIT_UNTIL_TX_IDLE();
			if (hw->csPin == 0xffffU) { //use IO channel select
				SELECT_IO_CHANNEL(hw->ioChannel);
			} else {
				GpioSetPin(hw->csPin, 0); //cs pin low
			}	
			break;
			
		case IOCTL_END_FRAME:
			__HWSPI_WAIT_UNTIL_TX_IDLE();
			if (hw->csPin == 0xffffU) { //use IO channel select
				SELECT_IO_CHANNEL_IDLE();
			} else {
				GpioSetPin(hw->csPin, 1); //cs pin high
			}
			if (hw->hasHwResources) {
				ReleaseHwLocksBIP(HLB_NONE, HLIO_CS_SELECT, HLP_NONE);
				hw->hasHwResources = 0;
			}
			break;
		
		case IOCTL_WAIT_UNTIL_TX_IDLE:
			__HWSPI_WAIT_UNTIL_TX_IDLE();
			break;


		default:
			return S_ERROR;
			break;
	}
	return S_OK;
}
	
