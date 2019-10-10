#include <exec.h>
#include <audio.h>
#include <timers.h>
#include "configureTouch.h"


StdButton cbuttons[3]={0};

void ConfigureTouch(void) {
	volatile struct TouchInfo *touchInfo = pTouchInfo;
	volatile s_int16 x, y;
	s_int16 rawX, rawY;
	u_int32 lastTimeCount = ReadTimeCount();
	s_int16 mustUpdate = 1;
	

	LcdFilledRectangle(0,0,lcd0.width-1,lcd0.height-1,0,COLOR_BLACK);
		
	lcd0.clipx1 = 0; lcd0.clipy1 = 0; lcd0.clipx2 = lcd0.width-1; lcd0.clipy2 = lcd0.height - 1;
	SetVirtualResolution(3,7);
	CreateStdButton(&cbuttons[0], 1, 0, 1, 2, 1, 1, "Ok");
	CreateStdButton(&cbuttons[1], 2, 0, 1, 3, 1, 2, "Cancel");
	//RenderStdButtons(button);
	

	lcd0.clipx1 = lcd0.x = 0;
	lcd0.clipy1 = lcd0.y = 0;
	lcd0.textColor = COLOR_WHITE;
	lcd0.backgroundColor = COLOR_BLACK;

	touchInfo->calibrationMode = 0;


	
	
#if 0
	touchInfo->maxX = calibInfo[0]+1;
	touchInfo->maxY = calibInfo[1]+1;
	touchInfo->minX = calibInfo[0];
	touchInfo->minY = calibInfo[1];
#else
	touchInfo->maxX = -32768;
	touchInfo->maxY = -32768;
	touchInfo->minX = 32767;
	touchInfo->minY = 32767;
#endif
	
	//while (!GetStdButtonPress(button)){
	while(1) {
		static u_int16 seemsCalibrated = 0;
		s_int16 pressure;
		u_int32 currTimeCount;

		if (mustUpdate) {
			mustUpdate = 0;
			LcdFilledRectangle(0,0,3,3,0,COLOR_WHITE);
			LcdFilledRectangle(lcd0.width-4,0,lcd0.width-1,3,0,COLOR_WHITE);
			LcdFilledRectangle(0,lcd0.height-4,3,lcd0.height-1,0,COLOR_WHITE);
			LcdFilledRectangle(lcd0.width-4,lcd0.height-4,lcd0.width-1,lcd0.height-1,0,COLOR_WHITE);

			LcdTextOutXY(8, 16, "TOUCH CALIBRATION");
			LcdTextOutXY(8, 32, "Touch all four corners.");
			LcdTextOutXY(8, 44, "To abort touch calibration, reset now.");
		}
		while ((currTimeCount = ReadTimeCount()) - lastTimeCount < TICKS_PER_SEC/50) {
			Delay(TICKS_PER_SEC/200);
		}
		lastTimeCount = currTimeCount;

		Disable(); // Want to make sure two calls to GetTouchLocation() return same moment in time
		touchInfo->calibrationMode = 1;
		pressure = GetTouchLocation(&rawX, &rawY);
		touchInfo->calibrationMode = 0;
		GetTouchLocation(&x, &y);
		Enable();
		if (pressure >= 20) {
			static s_int16 xl=2, xc=2, xh=2, yl=2, yc=2, yh=2;

			if (!seemsCalibrated) {
				if ((touchInfo->maxX - touchInfo->minX > 200) && (touchInfo->maxY - touchInfo->minX > 200)) {
					seemsCalibrated = 1;
					RenderStdButtons(cbuttons);
				}
			}

			if (rawX < touchInfo->minX) touchInfo->minX = rawX;
			if (rawY < touchInfo->minY) touchInfo->minY = rawY;
			if (rawX > touchInfo->maxX) touchInfo->maxX = rawX;
			if (rawY > touchInfo->maxY) touchInfo->maxY = rawY;
			//lcd0.x = 20; lcd0.y = 200;
			//printf("xy : %3d,%3d   raw: %3d,%3d \n",x,y,calibInfo[0],calibInfo[1]);
			//printf("min: %3d,%3d   max: %3d %3d \n",touchInfo[0],touchInfo[1],touchInfo[2],touchInfo[3]);
#if 0
			LcdFilledRectangle(x,y,x,y,0,COLOR_ORANGE);			
#else
			if (x >= 0 && x <= (s_int16)lcd0.clipx2 && y >= 0 && y <= (s_int16)lcd0.clipy2) {
				LcdFilledRectangle(xc, yl, xc, yh, NULL, lcd0.backgroundColor);
				LcdFilledRectangle(xl, yc, xh, yc, NULL, lcd0.backgroundColor);
				xc = x;
				yc = y;
				xl = xc-40;
				if (xl < 0) xl = 0;
				yl = yc-40;
				if (yl < 0) yl = 0;
				xh = xc+40;
				if (xh >= lcd0.clipx2) xh = lcd0.clipx2-1;
				yh = yc+40;
				if (yh >= lcd0.clipy2) yh = lcd0.clipy2-1;
				LcdFilledRectangle(xc, yl, xc, yh, NULL, COLOR_WHITE);
				LcdFilledRectangle(xl, yc, xh, yc, NULL, COLOR_WHITE);
				mustUpdate = 1;
			}
#endif
		}
		if (seemsCalibrated) {
			switch (GetStdButtonPress(cbuttons)) {
			case 1: //Ok
				{
					FILE *f = fopen("S:VSOS.INI", "w");
					if (f) {
						fprintf(f,"VSOS0304\n");
						fprintf(f,"touchHMinX=%d\ntouchHMinY=%d\n",touchInfo->minX, touchInfo->minY);
						fprintf(f,"touchHMaxX=%d\ntouchHMaxY=%d\n",touchInfo->maxX, touchInfo->maxY);
						fclose(f);
					}

				}
				// Save calibration
				goto finally;				
			case 2: //Cancel
				goto finally;	
			}
		}
	}
	finally:
	touchInfo->calibrationMode = 0;
} 