/// Functions for starting the VSOS on VS1005G Developer Board.

#include <vsos.h>     // Headers for using the VSOS Kernel

#include <vstypes.h>
#include <string.h>
#include "vo_stdio.h"
#include "vsos_vs1005g.h" // Functions for starting the OS on a VS1005F Developer Board
#include "power.h"
#include "iochannel.h" // Definitions for generating chip selects on the PCB
#include "iochannel_devboard.h" // Definitions of chip selects on the VS1005 developer board
#include "devSdSd.h" // Device Driver for SD card in SPI mode
#include "devSwSpi.h" // Device Driver for making an SPI bus from GPIO pins
#include "vo_gpio.h"  // Headers for setting and reading GPIO pins
#include "vo_fat.h"   // Filesystem Driver for FAT16/FAT32 for the VSOS
#include "lcd.h"
#include "devConsole.h"
#include "vsostasks.h"
#include <exec.h>
#include <sysmemory.h>
#include "lowlevels.h"
#include "devSpiFlash.h"
#include "devHwSpi.h"

#define USE_INTERNAL_FLASH /* uncomment this line if you want to use INTERNAL flash for S: */

void ListDevices(void) {
	//static u_int16 hoi;
	u_int16 i;
	printf("\nInstalled system devices:\n"); //Decorative listout of all devices
	for (i=0; i<26; i++) {
		if (vo_pdevices[i]) {
			printf("%c: %s",'A'+i, vo_pdevices[i]->Identify(vo_pdevices[i],0,0));
			if (vo_pdevices[i]->fs) {
				printf(", handled by %s",vo_pdevices[i]->fs->Identify(vo_pdevices[i]->fs,0,0));
			}
			printf(".\n");
		}
	}
}


// This function starts the operating system on a VS1005 developer board
void StartOS3(void) {
		
#if 1
	vo_printf("VSOS %0.2f build " __DATE__ " " __TIME__ "\nVLSI Solution Oy 2012-2018 - www.vlsi.fi\n\n", (float)osVersion/100);
	SysReport("Starting the kernel..");
#endif
	vo_kernel_init();
#if 1
	SysReport("Starting Devices... ");
#endif
	

	// Reset SD card on the developer board
	// ResetSdCard(); // Already done in InitBoard()

#ifndef USE_INTERNAL_FLASH	
	// configure EXTERNAL SPI FLASH as system disk S
	{
		static DEVICE spiFlash; // this SPI flash device (only one flash per design is supported)
		static DEVICE spi; // the SPI bus used to connect to this SPI flash
		static const devHwSpiHwInfo hwInfo = {
			(__y spiRegisters*)SPI0_CF, // register base; spiRegisters *regs;
			CSPIN_USE_IOCHANNEL, // CS pin
			3, // io channel number (74HC138 address decoder select pattern)
			6144, // Maximum clock in kHz
		};
		SysReport("External SPI Flash");
		DevHwSpiCreate (&spi, &hwInfo, 0);
		DevSpiFlashCreate(&spiFlash, &spi, 2048); //SPI flash size 2 megabytes (2048 kilobytes)
		vo_pdevices['S'-'A'] = &spiFlash;
	}
#else
	// configure INTERNAL SPI FLASH as system disk S
	{
		static DEVICE spiFlash; // this SPI flash device (only one flash per design is supported)
		static DEVICE spi; // the SPI bus used to connect to this SPI flash
		static const devHwSpiHwInfo hwInfo = {
			(__y spiRegisters*)SPI0_CF, // register base; spiRegisters *regs;
			CSPIN_USE_INTERNAL, // ( CS pin ) -> select INTERNAL flash
			0,
			24576, // Maximum clock in kHz
		};	
		SysReport("Internal Flash");
		DevHwSpiCreate (&spi, &hwInfo, 0);
		DevSpiFlashCreate(&spiFlash, &spi, 1024); //SPI flash size 1 megabyte (1024 kilobytes)
		vo_pdevices['S'-'A'] = &spiFlash;	
	}	
#endif
	ListDevices();
}




// Patch for AllocMemY() hook in VS1005g.
__y void *AllocMemYPatch(size_t size, size_t align) {
  u_int16 oldForbidCount;
  void __y *retVal;
  Forbid();
  // ROM AllocMemY routine sometimes breaks forbidCount, hence this patch
  oldForbidCount = forbidCount;
  // Call ROM function (which may break forbidCount)
  retVal = __AllocMemY(size, align);
  // Restore correct forbidCount
  forbidCount = oldForbidCount;
  // Make sure everythng related to Permit() is also done
  Permit();
  return retVal;
}

void intOSgfix(void);
void ApplyGFixes(void) {
	int i;
	u_int32 intOSfixVal = 0x2a00000e + ((u_int32)intOSgfix<<6);	
#if 0
	for (i=32;i<64; i++) {
		if (i != 32+13) { /* For all except INT_RX */
			//printf("%02d %08lx %04x ", i-32, ReadIMem((void *)i), (u_int16)(ReadIMem((void *)i) >> 6));
			if (ReadIMem((void *)i) == 0x2a2026ce) {
				//printf("Mod vector %2d to %08lx", i, intOSfixVal);
				WriteIMem((void *)i, intOSfixVal);
			} else {
				printf("No change to %d\n", i-32);
			}
			//printf("\n");
		} else {
			printf("No change to %d\n", i-32);
		}
	}
#else
	for (i=0; i<32; i++) {
		if (i>1 && i!=13 && i!=14) {
			WriteIMem((void *)(i+32), intOSfixVal);
		} 
	}
#endif
	WriteIMem((void *)(AllocMemY), 0x2a000000+((u_int32)((u_int16)AllocMemYPatch) << 6));
}
