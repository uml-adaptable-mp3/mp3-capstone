#include "visualize.h"
#include "screenlayout.h"
#include <lcd.h>

#define VIY1 (Y_SPECTRUM+3)
#define VIY2 (Y_SPECTRUM+52)
#define VIH (VIY2-VIY1)
#define VIX1 X_SPECTRUM

#define COLOR_BAR __RGB565RGB(0x2f,0xff,0x2f)

const u_int16 color_bar[2] = {__RGB565RGB(0x27,0xff,0x27), __RGB565RGB(0x1f,0xef,0x1f)};


void Visualize() {
	static s_int16 k,q,qq;
	u_int16 i;
	
	if (!model.spectrum.hasData) return;

	if (k) k--;
	if (k) k--;
	if (q) q--;
	if (q) q--;
			
	

	for (i=0; i<60; i++) {
		//s_int16 value = model.spectrum.data[i]+i*model.spectrum.emphasis;
		//if (i>29) value -=30*model.spectrum.emphasis;
		s_int16 value = model.spectrum.data[i];
		
		if (value<90) {
			value = 0;
		} else {
			value -= 90;
		}
		if (i==0) {
			if (value>k) k=value;
			if (qq>q) q=qq;
			LcdFilledRectangle(X_SPECTRUM-2,VIY1,X_SPECTRUM+9,VIY2,NULL,__RGB565RGB(k,0,q));
			LcdFilledRectangle(X_SPECTRUM+130,VIY1,PANEL_RIGHT-2,VIY2,NULL,__RGB565RGB(k,0,q));
		}
		if (i==20) {
			qq=value*2;
		}
		value /= 2;
		
/*
		if ((i==0) || (i==30)) {
			value -= 30;
			if (value<0) value=0;
			value /= 2;
		} else {
			value /= 4;
		}
*/


		if (value>VIH-1) value = VIH-1;
		value = VIY2-value;							
		//LcdFilledRectangle(i*5+14,VIY2-VIH,i*5+17,value,NULL,COLOR_BLACK);
		//LcdFilledRectangle(i*5+14,value+1,i*5+17,VIY2,NULL,COLOR_BAR);
		LcdFilledRectangle(X_SPECTRUM+10+i*2,VIY2-VIH,X_SPECTRUM+i*2+11,value,NULL,__RGB565RGB(k,0,q));
		//LcdFilledRectangle(X_SPECTRUM+10+i*2,value+1,X_SPECTRUM+i*2+11,VIY2,NULL,color_bar[i&1]);
		LcdFilledRectangle(X_SPECTRUM+10+i*2,value+1,X_SPECTRUM+i*2+11,VIY2,NULL,color_bar[i&1]);
		//LcdFilledRectangle(X_SPECTRUM+6+i*2,value+1,X_SPECTRUM+i*2+7,VIY2,NULL,__RGB565RGB(0,0xff,k));
	}
	model.spectrum.hasData = 0;
}