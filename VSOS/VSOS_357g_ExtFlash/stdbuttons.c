/// \file stdbuttons.c Std touchscreen buttons handler for VSOS
/// \author Panu-Kristian Poiksalo, VLSI Solution Oy 2012

#include <vstypes.h>
#include "touch.h"
#include "lcd.h"
#include "lcdfunctions.h"
#include "stdbuttons.h"
#include <vo_stdio.h>
#include <string.h>
#include <timers.h>
#include <clockspeed.h>
 
StdButton *lastButtonPressed = NULL;
u_int16 virtualWidth = 240;
u_int16 virtualHeight = 320;
Coloring colorScheme[4] = {
//	{__RGB565RGB(180,180,180),__RGB565RGB(0,0,128),__RGB565RGB(255,255,255),__RGB565RGB(80,80,80),
//	__RGB565RGB(0,0,0),__RGB565RGB(180,180,180),__RGB565RGB(255,255,255),__RGB565RGB(60,60,60)},

	{__RGB565RGB(190,190,190),/*__RGB565RGB(80,80,80)*/__RGB565RGB(60,60,100),__RGB565RGB(255,255,255),__RGB565RGB(40,40,40),
	__RGB565RGB(0,0,0),/*__RGB565RGB(180,180,180)*/0xE6FC,__RGB565RGB(255,255,255),__RGB565RGB(90,90,90),
	__RGB565RGB(120,255,120),__RGB565RGB(131,131,131)},
	
	{COLOR_WHITE, COLOR_FUCHSIA, COLOR_YELLOW, COLOR_BLACK,
	COLOR_BLACK, COLOR_YELLOW, COLOR_YELLOW, COLOR_YELLOW,0},
	
	{COLOR_WHITE, COLOR_NAVY, COLOR_YELLOW, COLOR_BLACK,
	//COLOR_WHITE, COLOR_FUCHSIA, __RGB565RGB(255,128,255), __RGB565RGB(80,0,80)},
	COLOR_WHITE, COLOR_FUCHSIA, COLOR_FUCHSIA, COLOR_FUCHSIA},
	
	{__RGB565RGB(180,180,180),__RGB565RGB(0,0,128),__RGB565RGB(255,255,255),__RGB565RGB(80,80,80),
	__RGB565RGB(0,0,0),__RGB565RGB(180,180,180),__RGB565RGB(180,180,180),__RGB565RGB(180,180,180),
	__RGB565RGB(120,255,120),__RGB565RGB(111,111,111)},

};

Coloring *currentColoring = colorScheme;

const u_int16 topleft[] = {0x0000, 0x0000, 0xFFF, 0xFFFF,0x0000, __RGB565RGB(220,220,220), 0xFFFF, 0xFFFF,0xFFFF, 0xFFFF, 0xF79E, 0xE6FC,0xFFFF, 0xFFFF, 0xE6FC, 0xE6FC};
const u_int16 bottomleft[] = {0xFFFF, 0xFFFF, 0xE6FC, 0xCE39,0x0000, 0xE6FC, 0xB576, 0x9CB3,0x0000, 0x736e, 0x736E, 0x736E,0x0000, 0x0000, 0x0000, 0x0021};
const u_int16 bottomright[] = {	0xB576, 0x9CB3, 0x736E, 0x0021,	0x9CB3, 0x83F0, 0x62EC, 0x0021,	0x736E, 0x62EC, 0x39A7, 0x0000, 0x0021, 0x0021, 0x0000, 0x0000};
const u_int16 topright[] = {0xE6FC, 0x0000, 0x0000, 0x0000, 0xE6FC, 0x9CB3, 0x62EC, 0x0000, 0xE6FC, 0x9CB3, 0x62EC, 0x0000, 0xD67B, 0x9CB3, 0x62EC, 0x0021};
void PutCorner(u_int16 *ptn, u_int16 xx, u_int16 yy) {
	u_int16 *d = renderBuffer;
	u_int16 x,y;
	for (y=0; y<4; y++) {
		u_int16 bk = lcdbk[(y+yy)&0xf];
		for (x=0; x<4; x++) {
			if (*ptn) {
				*d++ = *ptn;
			} else {
				*d++ = bk;
			}
			ptn++;
		}
	}
	LcdFilledRectangle(xx,yy,xx+3,yy+3,renderBuffer,0);
}
	


#if 0 // TEXTURED BUTTON
/*
void PutCorners2(u_int16 __a1 x1, u_int16 __a0 y1, u_int16 __b1 x2,u_int16 __b0 y2) {
	PutCorner(topleft, x1,y1);
	PutCorner(bottomleft, x1,y2-5);
	PutCorner(bottomright, x2-3,y2-5);
	PutCorner(topright, x2-3,y1);
}

auto void DrawBevel(u_int16 __a1 x1, u_int16 __a0 y1, u_int16 __b1 x2,u_int16 __b0 y2, u_int16 __c0 flags) {	
	if (!(flags & BTN_NO_FACE)) {
		u_int16 y;
		u_int16 w = x2 - x1;
		u_int16 h = y2 - y1 - 1;
		if (flags & BTN_LOWERED) {
			LcdFilledRectangle(x1,y1,x2,y1,0,lcdbk[y1&0xf]);
			y1++;
			h--;
		}
		for (y=0; y<h; y++) {
			if (y<2) {
				memset(renderBuffer,0xffff,sizeof(renderBuffer));
			} else if (y>h-5) {
				memset(renderBuffer,bottomleft[(y-(h-4))*4+3],sizeof(renderBuffer));
				renderBuffer[w] = 0;
			} else {
				GetTile(x1,y1+y,w,1,&metalTile,renderBuffer);
				renderBuffer[0] = 0xffff;
				renderBuffer[1] = 0xffff;
				renderBuffer[w] = 0;
				renderBuffer[w-1] = 0x736E;
				renderBuffer[w-2] = 0x9CB3;
			}
			LcdFilledRectangle(x1,y1+y,x2,y1+y,renderBuffer,0);
		}
	}
	if (!(flags & BTN_NO_BEVEL)) {
			PutCorners2(x1,y1,x2,y2);
	}
}
*/
#else

void PutCorners2(u_int16 __a1 x1, u_int16 __a0 y1, u_int16 __b1 x2,u_int16 __b0 y2) {
	PutCorner(topleft, x1,y1);
	PutCorner(bottomleft, x1,y2-3);
	PutCorner(bottomright, x2-3,y2-3);
	PutCorner(topright, x2-3,y1);
}

u_int16 bevelColor[3];
auto void DrawBevel(u_int16 __a1 x1, u_int16 __a0 y1, u_int16 __b1 x2,u_int16 __b0 y2, u_int16 __c0 flags) {
	
	if (flags & BTN_LOWERED) {
		bevelColor[0] = currentColoring->buttonHighlight;
		bevelColor[1] = currentColoring->buttonShadow;
		bevelColor[2] = currentColoring->buttonLoweredFace;
	} else {
		bevelColor[1] = currentColoring->buttonHighlight;
		//bevelColor[0] = currentColoring->buttonShadow;
		bevelColor[0] = COLOR_BLACK;
		bevelColor[2] = currentColoring->buttonFace;
		if (flags & BTN_HIGHLIGHTED) {
			bevelColor[2] = currentColoring->buttonHighlightFace;
		}
	}

	if (!(flags & BTN_NO_FACE)) {
		LcdFilledRectangle(x1,y1,x2,y2,0,bevelColor[2]);
	}
	if (!(flags & BTN_NO_BEVEL)) {
		LcdFilledRectangle(x1,y1,x2,y1,0,bevelColor[1]);
		LcdFilledRectangle(x1,y1,x1,y2,0,bevelColor[1]);
		LcdFilledRectangle(x1,y2,x2,y2,0,bevelColor[0]);
		LcdFilledRectangle(x2,y1,x2,y2,0,bevelColor[0]);
		if (!(flags & BTN_LOWERED)) {
			LcdFilledRectangle(x2-2,y1,x2-2,y2,0,0x9CB3);
			LcdFilledRectangle(x2-1,y1,x2-1,y2,0,0x736E);
			LcdFilledRectangle(x1,y2-2,x2,y2-2,0,0x9CB3);
			LcdFilledRectangle(x1,y2-1,x2,y2-1,0,0x736E);
			PutCorners2(x1,y1,x2,y2);
		}
	}
}

#endif



const u_int16 radiobutton[] = {	
	0xE6FC, 0xE6FC, 0xE6FC, 0xE6FC, 0x8410, 0x8410, 0xE6FC, 0xE6FC, 0xE6FC, 0xE6FC,
	0xE6FC, 0xE6FC, 0x8410, 0x8410, 0x4208, 0x4208, 0x8410, 0x8410, 0xE6FC, 0xE6FC,
	0xE6FC, 0x8410, 0x4208, 0x4208, 0xFFFF, 0xFFFF, 0x4208, 0x4208, 0xFFFF, 0xE6FC,
	0xE6FC, 0x8410, 0x4208, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xD699, 0xFFFF, 0xE6FC,
	0x8410, 0x4208, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xD699, 0xFFFF,
	0x8410, 0x4208, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xD699, 0xFFFF,
	0xE6FC, 0x8410, 0x4208, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xD699, 0xFFFF, 0xFFFF,
	0xE6FC, 0x8410, 0xD699, 0xD699, 0xFFFF, 0xFFFF, 0xD699, 0xD699, 0xFFFF, 0xE6FC,
	0xE6FC, 0xE6FC, 0xFFFF, 0xFFFF, 0xD699, 0xD699, 0xFFFF, 0xFFFF, 0xFFFF, 0xE6FC,
	0xE6FC, 0xE6FC, 0xE6FC, 0xE6FC, 0xFFFF, 0xFFFF, 0xE6FC, 0xE6FC, 0xE6FC, 0xE6FC,
};


void StdButtonDefaultRender(register const StdButton *button, register u_int16 op, register u_int16 x, register u_int16 y) {
	u_int16 useConsoleFont;
	useConsoleFont = 0;
	if ((button->y2 - button->y1) <= 20) useConsoleFont = 1;
	if (button->flags & BTN_TEXT) useConsoleFont = 1;
	if (op == BTN_OP_UNSPECIFIED) {
		currentColoring = &colorScheme[(button->flags >> 11) & 3];
		DrawBevel(button->x1,button->y1,button->x2,button->y2,button->flags);
		if (!(button->flags & BTN_TEXT)){
			lcd0.textColor = currentColoring->buttonText;
			lcd0.backgroundColor = bevelColor[2];
			//lcd0.backgroundColor = currentColoring->buttonFace;//FOR TEXTURED BUTTONS
		}
		lcd0.x = lcd0.clipx1 = button->x1+4;
		lcd0.y = lcd0.clipy1 = button->y1+1;
		lcd0.clipx2 = button->x2-3;
		lcd0.clipy2 = button->y2-1;
		if (button->flags & BTN_LOWERED) {
			lcd0.x++; lcd0.y++;
		}
		if (!(button->flags & BTN_NO_CAPTION)) {
			u_int16 w = TextWidth16(button->caption);
			u_int16 bw = button->x2 - button->x1 - 8;
			if (w>=bw) {
				useConsoleFont = 1;
			}
			if (button->flags & BTN_CHECKABLE) {
				LcdFilledRectangle(button->x1+2,button->y1+3,button->x1+11,button->y1+12,radiobutton,0);
				if (button->flags & BTN_CHECKED) LcdFilledRectangle(button->x1+6,button->y1+6,button->x1+7,button->y1+9,0,0);
				if (button->flags & BTN_CHECKED) LcdFilledRectangle(button->x1+5,button->y1+7,button->x1+8,button->y1+8,0,0);
				lcd0.x+=9;
				lcd0.clipx1=lcd0.x;
			}
			if (useConsoleFont) {
				printf(button->caption);
			} else {
				u_int16 y = (button->y1+button->y2)/2-9;
				if (button->flags & BTN_LOWERED) {
					y++;
				}
				LcdTextOutXY16(button->x1+bw/2-w/2, y, button->caption);
			}
		}
	}
}  

auto void RenderStdButton(register const StdButton *button) {
	__y lcdInfo lcdSave;
	memcpyXY(&lcdSave, &lcd0, sizeof(lcd0));
	button->render(button,0,0,0);
	memcpyYX(&lcd0, &lcdSave, sizeof(lcd0));
} 



auto u_int16 physX(register u_int16 logX) {
	return ((logX*(lcd0.clipx2-lcd0.clipx1+1))/virtualWidth) + lcd0.clipx1;
}

auto u_int16 physY(register u_int16 logY) {
	return ((logY*(lcd0.clipy2-lcd0.clipy1+1))/virtualHeight) + lcd0.clipy1;
}


auto void CreateStdButton(register StdButton *button, register u_int16 result, register u_int16 flags, register s_int16 x, 
	register s_int16 y, register u_int16 w, register u_int16 h, register const char *caption){
	button->flags = flags;
	button->result = result;
	button->caption = caption;
	button->render = StdButtonDefaultRender;
	//button->render = StdButtonMetalRender;
	if (x==-1) {
		x = y % virtualWidth;
		y = y / virtualWidth;		
	}
	button->x1 = physX(x)+1;
	button->y1 = physY(y)+1;
	button->x2 = physX(x+w)-2;
	button->y2 = physY(y+h)-2;
/*
	button->x1 = (x*lcd0.width)/virtualWidth+1;
	button->y1 = (y*(lcd0.height-12))/virtualHeight+1+12;
	button->x2 = (((w*lcd0.width)/virtualWidth) + button->x1)-2;
	button->y2 = (((h*(lcd0.height-12))/virtualHeight) + button->y1)-2+12;	
*/
}

auto void SetVirtualResolution(register u_int16 screenWidth, register u_int16 screenHeight){
	//printf("SetVirtualResolution ");
	virtualWidth = screenWidth;
	virtualHeight = screenHeight; 
}

#include "iochannel.h" 
auto s_int16 GetStdButtonPress(register StdButton *buttons) {
	s_int16 t;
	s_int16 x,y;	
	static u_int16 timeoutCounter=0;
	static s_int16 lastx, lasty = 0;


	Delay(1);


	touchCalibrationMode = 0;
	t = GetTouchLocation(&x, &y);
	
	if (!t) { //Known no-press
		if (lastButtonPressed && (lastButtonPressed->flags & BTN_LOWERED)) {		
			lastButtonPressed->flags &= ~BTN_LOWERED;
			RenderStdButton(lastButtonPressed);
			{ 
				s_int16 result = lastButtonPressed->result;					
				lastButtonPressed = 0;
				return result;
			}
		}
		return 0;
	}
	if (t<0) { //Press in non-known location
		return 0;
	}
	if (lastButtonPressed) {
		
		if (
			((x != lastx) || (y != lasty))
		&& (x<=lastButtonPressed->x2) 
		&& (x>=lastButtonPressed->x1) 
		&& (y<=lastButtonPressed->y2) 
		&& (y>=lastButtonPressed->y1)
		) {
			lastButtonPressed->render(lastButtonPressed, 1, x, y);
			goto finally;
		} else	if (
		   (x<lastButtonPressed->x2+5) 
		&& (x>lastButtonPressed->x1-5) 
		&& (y<lastButtonPressed->y2+5) 
		&& (y>lastButtonPressed->y1-5)
		) {
			goto finally;
		} else {
			lastButtonPressed->flags &= ~BTN_LOWERED;
			RenderStdButton(lastButtonPressed);
			lastButtonPressed = 0;			
		}
	}
	while (buttons->result) {
		while(buttons->flags & BTN_DISABLED) {
			buttons++;
		}
		if ((x>=buttons->x1) && (x<=buttons->x2) && (y>=buttons->y1) && (y<=buttons->y2)) {
			lastButtonPressed = buttons;
			lastButtonPressed->flags |= BTN_LOWERED;
			RenderStdButton(lastButtonPressed);
			goto finally;
		}
		buttons++;
	}
	finally:	
	lastx = x;
	lasty = y;			
	return 0;	
}



auto void SetClippingRectangleToButton(register StdButton *button){
	u_int16 x1 = button->x1;
	u_int16 x2 = button->x2;
	u_int16 y1 = button->y1;
	u_int16 y2 = button->y2;
	Coloring *btnColoring = &colorScheme[(button->flags >> 11) & 3];
	if (!(button->flags & BTN_NO_BEVEL)) {
		// If button has bevel, reduce the clipping rectangle
		x1+=2;
		x2-=2;
		y1+=2;
		y2-=2;
	}
	if (button->flags & BTN_NO_BACKGROUND) {
		// If button has no background, increase the clipping rectangle
		x1-=1;
		x2+=1;
		y1-=1;
		y2+=1;
	}
	lcd0.x = lcd0.clipx1 = x1;
	lcd0.y = lcd0.clipy1 = y1;
	lcd0.clipx2 = x2;
	lcd0.clipy2 = y2;
	lcd0.textColor = btnColoring->buttonText;
	lcd0.backgroundColor = btnColoring->buttonFace;
	if (button->flags & BTN_TEXT) {
		lcd0.textColor = btnColoring->textColor;
		lcd0.backgroundColor = btnColoring->textBackground;
	}
	if (button->flags & BTN_INVISIBLE) {		
		lcd0.backgroundColor = lcd0.defaultBackgroundColor;
	}
	
}

