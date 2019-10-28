/// \file main.c ILI9341 2.88 inch TFT LCD display device driver for VSOS3
/// \author Panu-Kristian Poiksalo, VLSI Solution Oy

/* For free support for VSIDE, please visit www.vsdsp-forum.com */

/// This driver implements LcdFilledRectangle and LcdTextOutXY 
/// for ILI9341 displays. Compile it to DL3 file, save the DL3 file to S:SYS
/// and add its name (without the .DL3) to S:CONFIG.TXT to use the display.

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <lcd.h>
#include <vstypes.h>
#include <timers.h>
#include <iochannel.h>
#include <string.h>
#include <kernel.h>

#define LcdFilledRectangle MyLcdFilledRectangle
#define LcdTextOutXY MyLcdTextOutXY

u_int16 MyLcdTextOutXY (u_int16 x1, u_int16 y1, char *s);
u_int16 MyLcdFilledRectangle (u_int16 x1, u_int16 y1, u_int16 x2, u_int16 y2, u_int16 *texture, u_int16 color);


void ConsoleFrame(const char *caption) {
	u_int16 w = lcd0.width-1;
	u_int16 h = lcd0.height-1;
	
	lcd0.clipx1 = lcd0.clipy1 = 0;
	lcd0.clipx2 = lcd0.width - 1;
	lcd0.clipy2 = lcd0.height - 1;
	LcdFilledRectangle(0,0,lcd0.clipx2,lcd0.clipy2,0,lcd0.backgroundColor);
	LcdFilledRectangle(1,0,w-1,8,0,lcd0.defaultTextColor);
	LcdFilledRectangle(0,1,w,7,0,lcd0.defaultTextColor);
	lcd0.textColor = lcd0.defaultBackgroundColor;
	lcd0.backgroundColor = lcd0.defaultTextColor;
	LcdTextOutXY(5,1,caption);
	lcd0.textColor = lcd0.shadowColor;
	LcdTextOutXY(w-10,0,"x");
	lcd0.textColor = lcd0.defaultTextColor;		
	lcd0.backgroundColor = lcd0.defaultBackgroundColor;
	lcd0.clipy1 = 12;
	lcd0.x = lcd0.clipx1;
	lcd0.y = lcd0.clipy1;
}

void LcdPrint(char *s) {
	LcdTextOutXY(3,lcd0.y,s);
	lcd0.y += 10;
}

ioresult main(char *parameters) {
	u_int16 lasty = lcd0.y;
	
	if (lcd0.width == 0) {
		static char title[50];
		LcdInit(0);
		sprintf(title,"VS1005 Developer Board, VSOS %1.2f",(float)osVersion/100);
		ConsoleFrame(title);
		LcdPrint("Running in console mode.");
		LcdPrint("UART: 115200 bps, N81.");
	} else {
		LcdInit(0);
		lcd0.y = lasty;
	}
	LcdPrint(parameters);
	return S_OK;
}

