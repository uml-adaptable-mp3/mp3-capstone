/// Support for floating windows over background area, implemented by exclusion rectangle.

#ifndef LCDLAYER_H
#define LCDLAYER_H

#include <lcd.h>
#include <vs1005h.h>

typedef struct LcdWindowInfoStruct {
	u_int16 active;
	u_int16 x1, y1, x2, y2;
} LcdWindowInfo;

extern LcdWindowInfo window;
extern u_int16 (*ScreenFilledRectangle)(u_int16 x1, u_int16 y1, u_int16 x2, u_int16 y2, u_int16 *texture, u_int16 color);
extern u_int32 lfrSave;

void InitLcdLayer(void);
void FiniLcdLayer();
u_int16 LcdClientRectangle (u_int16 x1, u_int16 y1, u_int16 x2, u_int16 y2, u_int16 *texture, u_int16 color);

#endif