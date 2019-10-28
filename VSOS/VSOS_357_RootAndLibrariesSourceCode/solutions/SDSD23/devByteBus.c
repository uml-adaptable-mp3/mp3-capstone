/// \file devByteBus.c VsOS Device Driver for Byte-width Bus (used for LCD display and nand flash)
/// \author Panu-Kristian Poiksalo, VLSI Solution oy

#if 0
#include "forbid_stdout.h"
#else
#include <vo_stdio.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <vsos.h>
#include "devByteBus.h"	
#include <vs1005h.h>
#include <vo_gpio.h>
#include <iochannel.h>
#include <hwLocks.h>
#include "fifoRdWr.h"

/// The start address word of 1.5kW perip mem for the device
#define HLB_BYTEBUS HLB_5
#define __BYTEBUS_PERIP_MEM_BASE (0x0100*HLB_BYTEBUS)


const DEVICE devByteBusDefaults = {	
  0, //u_int16   flags; //< present, block/char
  DevByteBusIdentify, //char *    (*Identify)(void *obj, char *buf, u_int16 bufsize);	 
  DevByteBusCreate, //ioresult (*Create)(DEVICE *dev, char *name, struct filesystem_descriptor *fs);
  DevByteBusDelete, //ioresult (*Delete)(DEVICE *dev);
  DevByteBusIoctl, //ioresult (*Ioctl)(DEVICE *dev, s_int16 request, char *argp); //Start, Stop, Flush, Check Media
  // Stream operations	
  DevByteBusRead, //u_int16  DevByteBusRead(register __i0 DEVICE *f, void *buf, u_int16 destinationIndex, u_int16 bytes); ///< Read bytes to *buf, at byte index
  DevByteBusWrite, //u_int16  DevByteBusWrite(register __i0 DEVICE *f, void *buf, u_int16 sourceIndex, u_int16 bytes); ///< Write bytes from *buf at byte index
  // Block device operations
  0, //DevByteBusBlockRead, //ioresult (*BlockRead)(DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data);
  0, //DevByteBusBlockWrite, //ioresult (*BlockWrite)(DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data);
  //FILESYSTEM *fs; // pointer to the filesystem driver for this device
  //u_int16  deviceInstance; // identifier to detect change of SD card etc
  //u_int16  hardwareInfo[6]; // Device driver's info of this hardware
  //u_int16  deviceInfo[16]; // Filesystem driver's info of this device
};

char *DevByteBusIdentify(register __i0 void *obj, char *buf, u_int16 bufsize){
  return "Bytewide Bus";
}

#define __NF_ALE_PIN 0x0f
#define __NF_CLE_PIN 0x0e

ioresult DevByteBusCreate(register __i0 DEVICE *dev, void *name, u_int16 extraInfo) {
  devByteBusHwInfo *hw=(devByteBusHwInfo *)dev->hardwareInfo;	
  memcpy(dev, &devByteBusDefaults, sizeof(*dev));
  dev->deviceInstance = __nextDeviceInstance++;	

  // Initialize the hardware info
  memcpy(hw, name, sizeof(devByteBusHwInfo));
  
  // Perip pins config for known hardware SPI peripherals SPI0 and SPI1
  PERIP(GPIO0_MODE) |= 0x07ff; //Set GPIO0 pins [10:0] as peripherals
  //PERIP(GPIO0_MODE) &= ~0x07ff; //Set GPIO0 pins [10:0] as GPIO pins
  //PERIP(GPIO0_DDR) |= 0x06ff; //Set GPIO0 pins [10:9,7:0] as outputs
	
#if 0
  /* Removed 2017-10-23 because they interefered with
     VS1005 Dev. board Extension 1 */
  GpioSetPin(__NF_ALE_PIN, 0); //Set ALE pin as output, pull low
  GpioSetPin(__NF_CLE_PIN, 0); //Set CLE pin as output, pull low
#endif
		
  if (hw->csPin == 0xffffU) { //use IO channel select
    INITIALIZE_IO_CHANNEL_IDLE(); //switch GPIO pins to output, pull high
  } else {
    GpioSetPin(hw->csPin, 1); //cs pin high
  }

  dev->flags = __MASK_PRESENT | __MASK_OPEN | __MASK_READABLE | __MASK_WRITABLE;
  return S_OK;
}

	
ioresult DevByteBusDelete(register __i0 DEVICE *dev) {
  devByteBusHwInfo *hw=(devByteBusHwInfo *)dev->hardwareInfo;
  dev->flags = 0;
  return S_OK;
}

#define __BB_WAIT_IDLE() {while(PERIP(NF_CTRL) & NF_CTRL_ENA);}




/// \todo Now requires that destinationIndex = 0 for block transfers.
u_int16 DevByteBusRead(register __i0 DEVICE *dev, void *buf, u_int16 destinationIndex, u_int16 bytes){ ///< Read bytes to *buf, at byte index
  u_int16 *iBuf = buf;
  devByteBusHwInfo *hw=(devByteBusHwInfo *)dev->hardwareInfo;

  while (bytes) {
    u_int16 j = (bytes > 512) ? 512 : bytes; //max 512 bytes per transfer
    PERIP(NF_LEN) = j; //length in bytes
    PERIP(NF_PTR) = __BYTEBUS_PERIP_MEM_BASE;
    PERIP(NF_CTRL) = NF_CTRL_READSEL | NF_CTRL_USEPERIP | NF_CTRL_ENA;
    bytes -= j;
    j = (j+1) / 2; //j is now amount of words to transfer		
    __BB_WAIT_IDLE();

    xp_fiford(iBuf, __BYTEBUS_PERIP_MEM_BASE, j);
    iBuf += j;
  }												
  return S_OK;
}

/// \todo Now requires that sourceIndex = 0 for block transfers.
u_int16  DevByteBusWrite(register __i0 DEVICE *dev, void *buf, u_int16 sourceIndex, u_int16 bytes){ ///< Write bytes from *buf
  u_int16 *iBuf = buf;
  while (bytes) {
    u_int16 j = (bytes > 512) ? 512 : bytes; //max 512 bytes per transfer
    PERIP(NF_LEN) = j; //length in bytes
    bytes -= j;
    j = (j+1)/2; //j is now amount of words to transfer
    if (iBuf) {
      xp_fifowr(iBuf, __BYTEBUS_PERIP_MEM_BASE, j);
      iBuf += j;
    } else {
      xp_fifowrx(sourceIndex, __BYTEBUS_PERIP_MEM_BASE, j);
    }
    PERIP(NF_PTR) = __BYTEBUS_PERIP_MEM_BASE;
    PERIP(NF_CTRL) = NF_CTRL_USEPERIP | NF_CTRL_ENA;
    __BB_WAIT_IDLE();								
  }
  return S_OK;
}


ioresult DevByteBusIoctl(register __i0 DEVICE *dev, s_int16 request, char *argp) {
  devByteBusHwInfo *hw=(devByteBusHwInfo *)dev->hardwareInfo;
  //printf(" ByteBusIoctl:%d ",request);

  switch (request) {
  case IOCTL_START_FRAME:
    if (hw->csPin == 0xffffU) { //use IO channel select
      ObtainHwLocksBIP(HLB_BYTEBUS, HLIO_NF|HLIO_CS_SELECT, HLP_NAND|HLP_XPERIP_IF);
      SELECT_IO_CHANNEL(hw->ioChannel);
    } else {
      ObtainHwLocksBIP(HLB_BYTEBUS, HLIO_NF, HLP_NAND|HLP_XPERIP_IF);
      GpioSetPin(hw->csPin, 0); //cs pin low
    }
    hw->regs->cf = GetDivider(hw->maxClockKHz, 31);
    break;
    
  case IOCTL_END_FRAME:
    if (hw->csPin == 0xffffU) { //use IO channel select
      SELECT_IO_CHANNEL_IDLE();
      ReleaseHwLocksBIP(HLB_BYTEBUS, HLIO_NF|HLIO_CS_SELECT, HLP_NAND|HLP_XPERIP_IF);
    } else {
      GpioSetPin(hw->csPin, 1); //cs pin high
      ReleaseHwLocksBIP(HLB_BYTEBUS, HLIO_NF, HLP_NAND|HLP_XPERIP_IF);
    }
    break;
    
#if 0
  case IOCTL_WAIT_UNTIL_TX_IDLE:
    break;
    
  case IOCTL_TEST:
    break;
#endif
    
  default:
    return S_ERROR;
    break;
  }
  return S_OK;
}

