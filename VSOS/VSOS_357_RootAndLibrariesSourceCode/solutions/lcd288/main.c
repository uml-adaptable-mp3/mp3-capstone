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
#include <kernel.h>

DLLENTRY(init)
int init(void) {
	LcdInit(0);
	LcdTextOutXY(1,1,"ILI9341");
	return S_OK;
}

void fini(void) {
	LcdInit(0);
}
