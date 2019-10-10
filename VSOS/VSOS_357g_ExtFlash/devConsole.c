/// \file devConsole.c VsOS Console device driver for VS1005 Developer Board V1.0
/// \author Panu-Kristian Poiksalo, VLSI Solution oy

/// This console driver holds the touch panel hardware info (calibration data).


#include <stdlib.h>
#include <string.h>
#include "vsos.h"
#include "devConsole.h"	
#include "lcd.h"
#include "lcdfunctions.h"
#include "stdbuttons.h"
//#include "forbid_stdout.h"
#include <stdio.h>
#include <touch.h>
#include <exec.h>


static const char* deviceName = "Console";
char* DevConsoleIdentify(register __i0 void *obj, char *buf, u_int16 bufsize){
	return deviceName;
}

u_int16  ConsoleRead (register __i0 DEVICE *f, void *buf, u_int16 destinationIndex, u_int16 bytes);
u_int16 ConsoleWrite (register __i0 DEVICE *f, void *buf, u_int16 sourceIndex, u_int16 bytes);

extern DEVICE console;
extern u_int16 metal[];

void ConsoleFrame(const char *caption) {
	u_int16 i,j;

	u_int16 w = lcd0.width;
	u_int16 h = lcd0.height;
	simpleLcd = LcdInit(1);	

	// Draw the text background	
	if (simpleLcd) {
		LcdFilledRectangle(0,0,w,h,0,lcd0.backgroundColor);
	} else {
		for (i=0; i<=h; i++) {
			GetTile(0,i,w,1,&lcdBackground,renderBuffer);
			LcdFilledRectangle(0,i,w-1,i,renderBuffer,0);
		}
	}
	
	// Draw the screen title
	{
		StdButton b;
		memset(&b,0,sizeof(b));
		b.x1=1;
		b.x2=lcd0.width-2;
		b.y2=21;
		b.caption = caption;
		StdButtonDefaultRender(&b,0,0,0);
	}

	//lcd0.defaultBackgroundColor = lcd0.backgroundColor = __RGB565RGB(80,80,80);
	lcd0.backgroundColor = lcd0.defaultBackgroundColor;
	lcd0.textColor = lcd0.defaultTextColor;		

	lcd0.clipx1 = 3;
	lcd0.clipx2 = lcd0.width-4;
	lcd0.clipy1 = 22;
	lcd0.clipy2 = lcd0.height-2;
	lcd0.x = lcd0.clipx1;
	lcd0.y = lcd0.clipy1;
}


/// Creates a console device.
ioresult DevConsoleCreate (register __i0 DEVICE *dev, void *hwdev, u_int16 extraInfo) {
	devConsoleHwInfo* hw=(devConsoleHwInfo*)dev->hardwareInfo;	
	memset(dev, 0, sizeof(*dev));
	dev->deviceInstance = __nextDeviceInstance++;	
	dev->Identify = DevConsoleIdentify;
	dev->Create = DevConsoleCreate;	
	dev->Ioctl = DevConsoleIoctl;
	dev->Write = ConsoleWrite;
	dev->Read = ConsoleRead;
	// Initialize the hardware info
	dev->flags = __MASK_PRESENT | __MASK_READABLE | __MASK_WRITABLE | __MASK_CHARACTER_DEVICE;
	return dev->Ioctl(dev, IOCTL_RESTART, 0);
}

ioresult ConsoleOpen (register __i0 VO_FILE *f, const char *name, const char *mode) { ///< Find and open a file
	//printf("Console Open");
	f->flags = __MASK_OPEN | __MASK_READABLE | __MASK_WRITABLE | __MASK_PRESENT;
	return S_OK;
}

u_int16  ConsoleRead (register __i0 DEVICE *dev, void *buf, u_int16 destinationIndex, u_int16 bytes){ ///< Read bytes to *buf, at byte index		
	return S_OK; //this console does not support reading. (Sami's PS/2 Keyboard Driver does)
}


u_int16 ConsoleWrite (register __i0 DEVICE *dev, void *buf, u_int16 sourceIndex, u_int16 bytes){ ///< Write bytes from *buf at byte index
	devConsoleHwInfo* hw=(__F_CHARACTER_DEVICE(dev) ? (devConsoleHwInfo*)dev->hardwareInfo
	: (devConsoleHwInfo*)(((VO_FILE*)dev)->dev)->hardwareInfo);
	char text[2];

	Forbid();

	text[0] = text[1] = 0;
	while(bytes--) {
		MemCopyPackedBigEndian((u_int16*)text, 1, buf, sourceIndex++, 1);
		if (text[0] == '\r') {
			lcd0.x = lcd0.clipx1;
			continue;					
		}
		if (text[0] == '\n') {
			lcd0.x = lcd0.clipx2;
		}
		if (lcd0.x+7 > (lcd0.clipx2)) {
			lcd0.x = lcd0.clipx1;
			lcd0.y += 8;
			if (lcd0.y + 9 >= lcd0.clipy2) {
				lcd0.y = lcd0.clipy1;
			}					
			
			if ((!simpleLcd) && (lcd0.backgroundColor == lcd0.defaultBackgroundColor)) {
				u_int16 i;
				for (i=lcd0.y; i<lcd0.y+10; i++) {
					GetTile(lcd0.x,i,lcd0.clipx2-lcd0.x+1,1,&lcdBackground,renderBuffer);
					LcdFilledRectangle(lcd0.x,i,lcd0.clipx2,i,renderBuffer,0);	
				}
			} else {
				LcdFilledRectangle(lcd0.x, lcd0.y, lcd0.clipx2, lcd0.y+10, 0, lcd0.backgroundColor);
			}
		}
		if(text[0]!='\n') {
			LcdTextOutXY(lcd0.x+1, lcd0.y, text);
			lcd0.x += 7;	
		}			
	}

	Permit();
	
	return S_OK;
}


ioresult DevConsoleIoctl(register __i0 DEVICE *dev, s_int16 request, char *argp) {
	switch (request) {	
		case IOCTL_RESTART:
			ConsoleFrame(deviceName);			
			dev->fs = StartFileSystem(dev, "0"); //Start a file system (typically starts the character device FS)			
			if (dev->fs) {
				dev->flags = __MASK_PRESENT | __MASK_OPEN | __MASK_READABLE | __MASK_WRITABLE | __MASK_CHARACTER_DEVICE;
			}
			break;

		case IOCTL_START_FRAME:
			ConsoleFrame(argp);
			break;

		case IOCTL_OPEN_DEVICE_FILE:
		case IOCTL_CLOSE_DEVICE_FILE:
		case IOCTL_END_FRAME:
		case IOCTL_WAIT_UNTIL_TX_IDLE:
		case IOCTL_TEST:
			break;
			
		default:
			return S_ERROR;
			break;
	}
	return S_OK;
}
	