/// \file devSwSpi.c VsOS driver for Software SPI Port.
/// You can use this as an example for getting started with writing device drivers.
/// \author Panu-Kristian Poiksalo, VLSI Solution OY

#include <stdlib.h>
#include <string.h>
#include <vs1005g.h>
#include <vsos.h>
#include <devSwSpi.h>
#include <vo_gpio.h>
#include <forbid_stdout.h>
#include <hwLocks.h>

static char* deviceName = "Software SPI Port";

char* DevSwSpiIdentify(register __i0 void *obj, char *buf, u_int16 bufsize){
	return deviceName;
}

// Shift n bits in and out using a software SPI port

#if 0
u_int16 SwSpiTransfer (u_int16 outdata, u_int16 nbits, SpiPins *pins) {
	u_int16 b=0;
	u_int16 indata=0;	
	for (b=0; b<nbits; b++) {
		if (outdata & (1 << (nbits-1))) { // Leftmost bit first
			GpioSetPin(pins->mosi, 1);
		} else {
			GpioSetPin(pins->mosi, 0);
		}
		outdata <<= 1; // Shift outdata bits left
		GpioSetPin(pins->sclk, 1);		
		indata <<= 1; // Shift indata bits left
		indata |= GpioReadPin(pins->miso);
		GpioSetPin(pins->sclk, 0);		
	}
	return indata; // Return the result byte to caller
}
#else
/*
  Faster though less easily readable version of SwSpiTransfer().
  If you want to understand what GPIO operations actually happen,
  look 2 lines upwards.
*/
u_int16 SwSpiTransfer(u_int16 outdata, u_int16 nbits, SpiPins *pins) {
  register u_int16 b=0;
  register u_int16 indata=0;
  u_int16 __y *reg[3];
  u_int16 sclkMask = 1<<(pins->sclk & 0xf);

  reg[0] = GPIO0_BIT_CONF + 32*(pins->mosi >> 4);
  reg[1] = GPIO0_BIT_CONF + 32*(pins->miso >> 4);
  reg[2] = GPIO0_SET_MASK + 32*(pins->sclk >> 4);

  PERIP(reg[0]) = 0xf0U | (pins->mosi & 0xf);	/* Data 15 to pin mosi */
  PERIP(reg[1]) = (PERIP(reg[1]) & 0x00FF) |
    ((0x0     | (pins->miso & 0xf)) << 8);	/* Data  0 to pin miso */

  reg[0] += 1;	// BIT_CONF -> BIT_ENG0
  reg[1] += 2;	// BIT_CONF -> BIT_ENG1

  outdata <<= (16-nbits);
  for (b=0; b<nbits; b++) {
    PERIP(reg[0]) = outdata; 	    // outdata bit 15 to MOSI
    outdata <<= 1; // Shift outdata 1 bit left
    PERIP(reg[2]) = sclkMask;       // clock high (SET_MASK)
    indata <<= 1; // Shift indata 1 bit left
    indata |= (PERIP(reg[1]));
    PERIP(reg[2]+1) = sclkMask;     // clock low (CLEAR_MASK)
  }

  return indata; // Return the result byte to caller
}
#endif


ioresult DevSwSpiCreate(register __i0 DEVICE *dev, void *name, u_int16 extraInfo) {
	devSwSpiHwInfo *hw=(devSwSpiHwInfo*)dev->hardwareInfo;

	memcpy(dev, CommonErrorResultFunction, sizeof(*dev));
	dev->Identify = DevSwSpiIdentify;
	dev->Create = DevSwSpiCreate;
	dev->Ioctl = DevSwSpiIoctl;
	dev->Read = DevSwSpiRead;
	dev->Write = DevSwSpiWrite;		
	dev->deviceInstance = __nextDeviceInstance++;	
	dev->fs = 0; //No filesystem
	hw->pins = *(SpiPins *)name;
	ObtainHwLocksBIP(HLB_NONE, PinToGpioHwLockMask(hw->pins.xcs), HLP_NONE);

	{
		// This device never has a file system, so we can use
		// deviceInfo for our own purposes; in this case to save
		// I/O this SPI device requires, which is in
		// dev->deviceInfo[0] and dev->deviceInfo[1].
		u_int32 *maskP = (u_int32 *)(&dev->deviceInfo[0]);
		u_int32 ioMask;
		ioMask = PinToGpioHwLockMask(hw->pins.miso);
		ioMask |= PinToGpioHwLockMask(hw->pins.mosi);
		ioMask |= PinToGpioHwLockMask(hw->pins.sclk);
		*maskP = ioMask;
	}

	return dev->Ioctl(dev, IOCTL_RESTART, 0);	
}


u_int16 DevSwSpiRead(register __i0 DEVICE *dev, void *buf, u_int16 destinationIndex, u_int16 bytes) {
	devSwSpiHwInfo* hw=(devSwSpiHwInfo*)dev->hardwareInfo;
	u_int16 bytesToTransfer = bytes;
	if (!buf) { // if no buffer is specified, return a single byte value read
		return SwSpiTransfer(0xffffU, 8, &(hw->pins));
	}
	while (bytesToTransfer--) {
		u_int16 b = SwSpiTransfer(0xffffU, 8, &(hw->pins));
		MemCopyPackedBigEndian(buf, destinationIndex++, &b, 1, 1);
	}
	return bytes;
}

u_int16 DevSwSpiWrite(register __i0 DEVICE *self, void *buf, u_int16 sourceIndex, u_int16 bytes) {
	devSwSpiHwInfo* hw=(devSwSpiHwInfo*)self->hardwareInfo;
	u_int16 bytesToTransfer = bytes;
	if (!buf) { //if no buffer is specified, transfer "bytes" copies of byte "sourceIndex"
		while(bytesToTransfer--) {
			SwSpiTransfer(sourceIndex,8,&(hw->pins)); //send byte from "sourceIndex" parameter
		}
		return S_OK;
	}
	while (bytesToTransfer--) {
		u_int16 b;
		MemCopyPackedBigEndian(&b, 1, buf, sourceIndex++, 1);
		SwSpiTransfer(b, 8, &(hw->pins));
	}	
	return bytes;
}

ioresult DevSwSpiIoctl(register __i0 DEVICE *dev, s_int16 request, char *argp) {
	devSwSpiHwInfo *hw=(devSwSpiHwInfo*)dev->hardwareInfo;
	switch (request) {
		case IOCTL_START_FRAME:	
			{
				u_int32 *maskP = (u_int32 *)(&dev->deviceInfo[0]);
				ObtainHwLocksBIP(HLB_NONE, *maskP, HLP_NONE);
			}
			GpioSetPin(hw->pins.mosi, 0); // Set as output idle low pin
			GpioSetPin(hw->pins.sclk, 0); // Set as output idle low pin
			GpioSetAsInput(hw->pins.miso);	// Set as input pin
			GpioSetPin(hw->pins.xcs,0);
			break;
		
		case IOCTL_END_FRAME:
			GpioSetPin(hw->pins.xcs,1);
			{
			  	u_int32 *maskP = (u_int32 *)(&dev->deviceInfo[0]);
				ReleaseHwLocksBIP(HLB_NONE, *maskP, HLP_NONE);
			}
			break;
	
		case IOCTL_RESTART: 					
			GpioSetPin(hw->pins.xcs, 1); // Set as output idle high pin
#if 0
			GpioSetPin(hw->pins.mosi, 0); // Set as output idle low pin
			GpioSetPin(hw->pins.sclk, 0); // Set as output idle low pin
			GpioSetAsInput(hw->pins.miso);	// Set as input pin
#endif
			dev->flags = __MASK_PRESENT | __MASK_OPEN | __MASK_CHARACTER_DEVICE | __MASK_READABLE | __MASK_WRITABLE;
			break;

		default:
			return S_ERROR;
			break;
	}		
	return S_OK;
}
