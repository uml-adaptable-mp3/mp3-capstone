/// Support for floating windows over background area, implemented by exclusion rectangle.
/// \author Panu-Kristian Poiksalo, VLSI Solution Oy

#include <vo_stdio.h>
#include <vs1005h.h>
#include <timers.h>
#include "lcdlayer.h"
//#include "mutex.h"
#include "kernelmutex.h"
#include <imem.h>

LcdWindowInfo window;
u_int16 lcdMutex;



u_int16 (*ScreenFilledRectangle)(u_int16 x1, u_int16 y1, u_int16 x2, u_int16 y2, u_int16 *texture, u_int16 color) = NULL;

u_int16 LcdClientRectangle (u_int16 x1, u_int16 y1, u_int16 x2, u_int16 y2, u_int16 *texture, u_int16 color) {
	if ((x2<x1) || (y2<y1)) return;
	ObtainMutex(&lcdMutex);
	if ((window.active==0) || (y1>window.y2) || (y2<window.y1) || (x2<window.x1) || (x1>window.x2)) { // || (y1>window.y2) || (y2<window.y1)
		ScreenFilledRectangle (x1,y1,x2,y2,texture,color);		
		ReleaseMutex(&lcdMutex);
		return 0;
	} else {
		u_int16 x,y,d;
		d=x2-x1+1;
		for (y=y1; y<=y2; y++) {
			if ((y<window.y1) || (y>window.y2) || (x2<window.x1) || (x1>window.x2)) {
				ScreenFilledRectangle(x1,y,x2,y,texture,color);
			} else {
				if (x1<window.x1) {
					ScreenFilledRectangle(x1,y,window.x1-1,y,texture,color);
				}
				if (x2>window.x2) {
					u_int16 *t = texture;
					if (t) {
						t += window.x2-x1+1;
					}
					ScreenFilledRectangle(window.x2+1,y,x2,y,t,color);				
				}
				
			}
			if (texture) texture += d;
		}
	}
	ReleaseMutex(&lcdMutex);
}

u_int32 lfrSave; //The original JMPI opcode to LcdFilledRectangle

/// Installs the layered LCD fillrect function to the OS so all "normal" code like printf uses it
void InitLcdLayer(void) {
	u_int32 v = ReadIMem((u_int16)LcdFilledRectangle);
	lfrSave = v;
	InitMutex(&lcdMutex);
	ScreenFilledRectangle = v>>6; //Get address of function from opcode
	v &= ~((u_int32)0xffffL << 6);
	v |= ((u_int32)LcdClientRectangle)<<6; //Patch new address to opcode
	WriteIMem((u_int16)LcdFilledRectangle, v); //Inject new JMPI opcode
}

/// Restores the original LCD fillrect function to the os
void FiniLcdLayer() {
  if (lfrSave) WriteIMem((u_int16)LcdFilledRectangle, lfrSave);
}

