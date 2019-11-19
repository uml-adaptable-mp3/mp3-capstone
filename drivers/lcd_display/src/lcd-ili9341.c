/// \file lcd-ili9341.c Driver for A-Tops AT028A9341Z-P TFT module with ILI9341 controller
/// \author Panu-Kristian Poiksalo, VLSI Solution Oy
/**
	- Uses a Byte-wide Bus device driver to access the TFT LCD display,
	  but sets RD, WR, A0 and CS pins by direct register writes in hard macros for speed.
	- This device driver is very fast.
	- Define the actual pin connections in your PCB in the LcdInit function and macros in the file.

	LCD NOTES
	- By default, TFT displays in VS1005 systems are kept in 16 bits per pixel, RGB565 color mode.
*/

#include <stdlib.h>
#include <string.h>
#include <vstypes.h>
#include "vsos.h"
#include "vs1005h.h"
#include "romfont1005e.h"
#include "vo_gpio.h"
#include "devByteBus.h"
#include "ili9341.h"
#include "rgb565.h"
#include "lcd.h"
#include "vsnand.h"
#include "sethandler.h"
#include <hwLocks.h>

//#include "forbid_stdout.h"
#include <stdio.h>

#if 1
#define PORTRAIT_MODE
#endif

DEVICE devLcdHw;
#define __LCD_PIN_RD 0x09
#define __LCD_PIN_WR 0x0a
#define __LCD_PIN_A0 0x0e
#define __LCD_PIN_CS 0x1f

void LcdDelay(u_int32 n){
  while (n--) {
    n = ~n;
    n = ~n;
  }
}

// Hard macros to manipulate the interface pins.
#define A0_SET_HIGH(){PERIP(GPIO0_SET_MASK) = (1 << 0xe);}
#define A0_SET_LOW(){PERIP(GPIO0_CLEAR_MASK) = (1 << 0xe);}
#define CS_SET_HIGH(){PERIP(GPIO1_SET_MASK) = (1 << 0xf);}
#define CS_SET_LOW(){PERIP(GPIO1_CLEAR_MASK) = (1 << 0xf);}
#define RD_SET_HIGH(){PERIP(GPIO0_SET_MASK) = (1 << 0x9);}
#define RD_SET_LOW(){PERIP(GPIO0_CLEAR_MASK) = (1 << 0x9);}
#define WR_SET_HIGH(){PERIP(GPIO0_SET_MASK) = (1 << 0xa);}
#define WR_SET_LOW(){PERIP(GPIO0_CLEAR_MASK) = (1 << 0xa);}
#define WRITE_BYTE(a){PERIP(GPIO0_CLEAR_MASK)=0x00ff; PERIP(GPIO0_SET_MASK)=((a)&0xff); PERIP(GPIO0_CLEAR_MASK)=0x0400; WR_SET_LOW();WR_SET_LOW();PERIP(GPIO0_SET_MASK)=0x0600;}
#define READ_BYTE(a){RD_SET_LOW();RD_SET_LOW();RD_SET_LOW();RD_SET_LOW();RD_SET_LOW();RD_SET_LOW();RD_SET_LOW();RD_SET_LOW();a=PERIP(GPIO0_IDATA)&0xff;RD_SET_HIGH();}

extern lcdInfo lcd0;
//u_int16 myTest = 1;

#ifdef PORTRAIT_MODE

#define __LCD_LOGICAL_WIDTH 240
#define __LCD_LOGICAL_HEIGHT 320

#define PG0  8 // 8 ei vaikutusta
#define PG1  8 // 3 ei vaikutusta
#define PG2  8 // 4 ei vaikutusta
#define PG3  8 // 5 ei vaikutusta
#define PG4  8 // 1 ei vaikutusta
#define PG5  8 // 9 pient� hienos��t�� kirkkaammassa p��ss�
#define PG6  8 // 8 isompi luku kirkastaa puoliv�lin kohdalta
#define PG7  4 // 6 isompi luku kirkastaa hieman puoliv�lin yl�puolelta
#define PG8  0 // 8 ei vaikutusta
#define PG9  6 // 5-->7 isompi luku kirkastaa koko keskialuetta
#define PG10 0 // 11 ei vaikutusta
#define PG11 0 // 2 ei vaikutusta
#define PG12 0 // 0 ei vaikutusta
#define PG13 0 // 0 isompi luku kirkastaa aivan tummimpia s�vyj�
#define PG14 0 // 0 tummin vihre�n s�vy muuttuu t�st� (turha)
#define PG15 0 // 0 ei vaikutusta

#define NG0  8 // 8 ei vaikutusta
#define NG1  5 // 6 s��t�� kirkkaimman v�rin valkoisuutta ja punaisuutta
#define NG2  6 // 8 isompi arvo tummentaa kirkkaimpia s�vyj�
#define NG3  0 // 7
#define NG4  6 // 7 isompi arvo tummentaa melkein kirkkaimpia s�vyj�
#define NG5  5 // 5 ei vaikutusta?
#define NG6  7 // 7 j�lleen suht laaja-alainen kirkkaiden s�vyjen yleistaso
#define NG7  8 // 10 ei vaikutusta
#define NG8  8 // 8 ei vaikutusta
#define NG9  10  // 8 pieni arvo kirkastaa tummimpia s�vyj�
#define NG10 0 // 4-->0
#define NG11 8 // 12 (8) ei vaikutusta
#define NG12 8 // 15
#define NG13 8 // 15
#define NG14 8 // 15
#define NG15 8 // 15

const s_int16 ILI9341_init[] = {
	3,  INTERFACE_CONTROL,			0x01, 0x01, 0x00,
	3,  0xEF,    						0x03, 0x80, 0x02,
	3,  POWER_CONTROL_B,   			0x00, 0xF2, 0xA0,
	4,  POWER_ON_SEQUENCE_CONTROL,	0x64, 0x03, 0x12, 0x81,
	5,  POWER_CONTROL_A,   			0x39, 0x2C, 0x00, 0x34, 0x02,
	2,  DRIVER_TIMING_CONTROL_B,	0x00, 0x00,
	3,  DRIVER_TIMING_CONTROL_A,	0x85, 0x10, 0x7A,
	1,  POWER_CONTROL_1,				0x21,    //VRH[5:0]
	1,  POWER_CONTROL_2,				0x11,    //SAP[2:0];BT[3:0]
	2,  VCOM_CONTROL_1,				0x3F, 0x3C,
	1,  VCOM_CONTROL_2,				0xC6,       // 0xD2
	1,  PIXEL_FORMAT_SET,			0x55,
	1,  MEMORY_ACCESS_CONTROL,		0x08, // 0xA8 = 90deg. rotate, 0x08 = normal
	2,  FRAME_RATE_CONTROL,			0x00, 0x1B, // 70Hz framerate
	2,  DISPLAY_FUNCTION_CONTROL,	0x0A, 0xA2,
	15, POSITIVE_GAMMA_CORRECTION,	PG0,PG1<<2,PG2<<2,PG3,PG4<<1,PG5,PG6<<3,PG7|(PG8<<3),PG9<<3,PG10,PG11<<1,PG12,PG13<<2,PG14<<2,PG15,
	15, NEGATIVE_GAMMA_CORRECTION,	NG0,NG1<<2,NG2<<2,NG3,NG4<<1,NG5,NG6<<3,NG7|(NG8<<3),NG9<<3,NG10,NG11<<1,NG12,NG13<<2,NG14<<2,NG15,
	0, WRITE_MEMORY,
	0, SLEEP_OUT,  // exit sleep
	//0xFF, 120,   // delay 120msec
	0, DISPLAY_ON,  // display ON
	//6, 0x33, 0,32,0,128,((320-32-128)>>8)&0xFF,(320-32-128)&0xFF,  (Scrolling)
	//2, 0x37, 0,64,
	-1
};

#else // !PORTRAIT_MODE

#define __LCD_LOGICAL_WIDTH 320
#define __LCD_LOGICAL_HEIGHT 240

const s_int16 ILI9341_init[] = {
	3,  INTERFACE_CONTROL,          0x01, 0x01, 0x00,
	3,  0xEF,    0x03, 0x80, 0x02,
	3,  POWER_CONTROL_B,   0x00, 0xF2, 0xA0,
	4,  POWER_ON_SEQUENCE_CONTROL, 0x64, 0x03, 0x12, 0x81,
	5,  POWER_CONTROL_A,   0x39, 0x2C, 0x00, 0x34, 0x02,
	2,  DRIVER_TIMING_CONTROL_B, 0x00, 0x00,
	3,  DRIVER_TIMING_CONTROL_A,    0x85, 0x10, 0x7A,
	1,  POWER_CONTROL_1,   0x21,    //VRH[5:0]
	1,  POWER_CONTROL_2,   0x11,    //SAP[2:0];BT[3:0]
	2,  VCOM_CONTROL_1,    0x3F, 0x3C,
	1,  VCOM_CONTROL_2,    0xC6,       // 0xD2
	1,  PIXEL_FORMAT_SET,    0x55,
	1,  MEMORY_ACCESS_CONTROL,  0xA8, //BGR=A0
//	2,  FRAME_RATE_CONTROL,   0x00, 0x1B,
	2,  FRAME_RATE_CONTROL,   0x00, 0x00,
	2,  DISPLAY_FUNCTION_CONTROL, 0x0A, 0xA2,
	1,  ENABLE_3G,     0x00,    // 3Gamma function disable
	1,  GAMMA_SET,     0x01,    // Gamma curve selected
	15, POSITIVE_GAMMA_CORRECTION, 0x0f, 0x24, 0x21, 0x0F, 0x13, 0x0A, 0x52, 0xC9, 0x3B, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00,
	15, NEGATIVE_GAMMA_CORRECTION, 0x00, 0x1B, 0x1E, 0x00, 0x0C, 0x04, 0x2F, 0x36, 0x44, 0x0a, 0x1F, 0x0F, 0x3F, 0x3F, 0x0F,
	0, WRITE_MEMORY,
	0, SLEEP_OUT,  // exit sleep
	//0xFF, 120,   // delay 120msec
	0, DISPLAY_ON,  // display ON
	-1
};

#endif

#define Output(p,v,b,n) Write(p,b,v,n)
#define Input(p,v,b,n) Read(p,b,v,n)

void TFTWriteRegister(u_int16 reg, u_int16 data) {
	//Start freme needs to be abstract, because the previous operation might have been overlapped; the ioctl waits and clears the overlapped condition.
	devLcdHw.Ioctl(&devLcdHw, IOCTL_START_FRAME, 0);
	//GpioSetPin(__LCD_PIN_A0, 0); //LCD set command mode
	PERIP(GPIO0_CLEAR_MASK) = (1 << 14);
	//Output the register number
	devLcdHw.Output(&devLcdHw, 0, &reg, 2);
	//GpioSetPin(__LCD_PIN_A0, 1); //LCD set data mode
	PERIP(GPIO0_SET_MASK) = (1 << 14);
	//Output the data, because of the ioctl_start_frame, the device is now in non-overlapped mode.
	devLcdHw.Output(&devLcdHw, 0, &data, 2);
	//devLcdHw.Ioctl(&devLcdHw, IOCTL_END_FRAME, 0);
	PERIP(GPIO1_SET_MASK) = (1 << 0);
}

#if 0
// A faster implementation of TFTWriteRegister
// This function seems to work at at least 80 MHz.
void TFTWriteRegister2(u_int16 reg, u_int16 data) {
	PERIP(GPIO0_MODE) &= ~0x07ff; //Set GPIO0 pins [10:0] as GPIO pins
	PERIP(GPIO0_DDR) |= 0x06ff; //Set GPIO0 pins [10:9,7:0] as outputs
	WR_SET_HIGH();
	RD_SET_HIGH();
	A0_SET_LOW();
	CS_SET_LOW();
	A0_SET_LOW();
	WRITE_BYTE(0);
	WRITE_BYTE(reg);
	A0_SET_HIGH();
	WRITE_BYTE(data >> 8);
	WRITE_BYTE(data);
	CS_SET_HIGH();
	PERIP(GPIO0_MODE) |= 0x07ff; //Set GPIO0 pins [10:0] as peripherals
}
#endif

// This function seems to work at at least 80 MHz.
void TFTWriteVector(s_int16 *vec) {
	s_int16 reg;
	s_int16 parm;

	PERIP(GPIO0_MODE) &= ~0x07ff; //Set GPIO0 pins [10:0] as GPIO pins
	PERIP(GPIO0_DDR) |= 0x06ff; //Set GPIO0 pins [10:9,7:0] as outputs
	WR_SET_HIGH();
	RD_SET_HIGH();
	while (*vec != -1) {
		parm = *vec++;
		reg = *vec++;
		//printf("\nReg:%04x",reg);
		CS_SET_LOW();
		A0_SET_LOW();
		WRITE_BYTE(reg);
		while (parm--) {
			A0_SET_HIGH();
			//printf(" %02x",*vec);
			WRITE_BYTE(*vec++);
		}
	}
	CS_SET_HIGH();
	PERIP(GPIO0_MODE) |= 0x07ff; //Set GPIO0 pins [10:0] as peripherals
	//SysReport("End");
}


u_int16 MyLcdFilledRectangle (u_int16 x1, u_int16 y1, u_int16 x2, u_int16 y2, u_int16 *texture, u_int16 color) {
	s_int16 cmd[] = {
		4, SET_COLUMN_ADDRESS, 0, 0, 0, 0,
		4, SET_PAGE_ADDRESS, 0, 0, 0, 0,
		//4, WRITE_MEMORY, 0x55, 0xaa, 0x37, 0xf6,
		-1};

	u_int16 buff[2];
	u_int32 n = (((u_int32)x2-x1)+1);


	n *= (((u_int32)y2-y1)+1);
	n *= 2; //pixels to bytes

	{
		s_int16 *k = &cmd[2];
		*k++ = (x1>>8); *k++ = (x1&0xff);
		*k++ = (x2>>8); *k++ = (x2&0xff);
		k += 2;
		*k++ = (y1>>8); *k++ = (y1&0xff);
		*k++ = (y2>>8); *k++ = (y2&0xff);
	}
	ObtainHwLocksBIP(HLB_NONE, HLIO_0_14|HLIO_0_15, HLP_NONE);
	TFTWriteVector(cmd);

	//NandWaitIdle();
	GpioSetPin(__LCD_PIN_A0, 0); //LCD set command mode
	devLcdHw.Ioctl(&devLcdHw, IOCTL_START_FRAME, 0);
	devLcdHw.Output(&devLcdHw, WRITE_MEMORY, 0, 1);
	//NandWaitIdle();
	GpioSetPin(__LCD_PIN_A0, 1); //LCD set data mode

	if (texture) {
		devLcdHw.Output(&devLcdHw,0,texture,(u_int16)n);
	} else {
		while(n) {
			if (n>16384) {
				n -= 16384;
				devLcdHw.Output(&devLcdHw,color,0,16384);
			} else {
				devLcdHw.Output(&devLcdHw,color,0,(u_int16)n);
				n=0;
			}
		}
	}
	devLcdHw.Ioctl(&devLcdHw, IOCTL_END_FRAME, 0);
	ReleaseHwLocksBIP(HLB_NONE, HLIO_0_14|HLIO_0_15, HLP_NONE);
}

u_int16 MyLcdTextOutXY (u_int16 x1, u_int16 y1, char *s) {
	static u_int16 buff[7*8];
	while (*s) { //Foreach character *s do:
		__mem_y u_int16 *p = &latin1[(*s)*3]; //Get a pointer to ROM font table
		u_int16 x,y;
		memset(buff,lcd0.backgroundColor,sizeof(buff)); //fill the texture with bk color
		for (x=0; x<3; x++) { //light some texture pixels with text color based on font
			for (y=0; y<8; y++) {
				if (p[x]&(1 << (y+8))) buff[y*7+x*2] = lcd0.textColor;
				if (p[x]&(1 << (y))) buff[y*7+x*2+1] = lcd0.textColor;
			}
		}
		LcdFilledRectangle(x1,y1,x1+6,y1+7,buff,0); //Draw a textured rectangle
		x1 += 7;
		s++;
	}
}



u_int16 LcdInit (u_int16 display_mode) {
	static const devByteBusHwInfo hwInfo = {
		(__mem_y byteBusRegisters*)NF_CF, // register base;
		0xffffU, // CS pin 0xffffU: use io channel number;
		6, // io channel number (74HC138 address decoder select pattern)
		1, //speed (divider)
	};
	u_int16 i;

	if (display_mode == 0) { //Power-Up reset
		DevByteBusCreate (&devLcdHw, &hwInfo, 0);
		// Execute all register writes in the initialization vector
		TFTWriteVector(ILI9341_init);
		memset(&lcd0,0,sizeof(lcd0));
	}

	if (display_mode == 1) { //Power-Up reset
		memset(&lcd0,0,sizeof(lcd0));
	}

	lcd0.width = __LCD_LOGICAL_WIDTH;
	lcd0.height = __LCD_LOGICAL_HEIGHT;
	lcd0.clipx1 = 0;
	lcd0.clipx2 = __LCD_LOGICAL_WIDTH - 1;
	lcd0.clipy1 = 0;
	lcd0.clipy2 = __LCD_LOGICAL_HEIGHT - 1;


	lcd0.defaultTextColor = lcd0.textColor = __RGB565RGB(180,180,180);
	lcd0.defaultBackgroundColor = lcd0.backgroundColor = __RGB565RGB(0,0,128);
	lcd0.highlightColor = __RGB565RGB(255,255,255);
	lcd0.shadowColor = __RGB565RGB(80,80,80);
	lcd0.buttonTextColor = __RGB565RGB(0,0,0);
	lcd0.buttonFaceColor = __RGB565RGB(180,180,180);

	SetHandler(LcdFilledRectangle, MyLcdFilledRectangle);
	SetHandler(LcdTextOutXY, MyLcdTextOutXY);

	// Clear the screen
	LcdFilledRectangle(0,0,lcd0.width-1,lcd0.height-1,0,lcd0.backgroundColor);
}
