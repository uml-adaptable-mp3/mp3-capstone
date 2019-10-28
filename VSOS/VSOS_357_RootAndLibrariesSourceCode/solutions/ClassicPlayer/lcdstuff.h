#ifndef LCDSTUFF_H
#define LCDSTUFF_H

#include <vstypes.h>
#include <lcd.h>

extern const __mem_y u_int16 lcdNumData[];
extern u_int16 screenX, screenY;
extern u_int16 screenc0;
extern u_int16 screenc1;
extern u_int16 digits[6];
extern const u_int16 greenText[];
extern const u_int16 greyText[];
extern const u_int16 blackText[];
extern const u_int16 blueOnWhite[];
extern const u_int16 loweredText[];
extern u_int16 raised[3];
extern u_int16 lowered[3];
extern u_int16 lowered_dark[3];
extern u_int16 lowered_text[3];

auto void DrawBevel(u_int16 __a1 x1, u_int16 __a0 y1, u_int16 __b1 x2,u_int16 __b0 y2, u_int16 *bevelColor);
void LcdPutLcdNum(u_int16 n);
void PutDigits();
extern void (*StdRender)(register struct stdButtonStruct *button, register u_int16 op, register u_int16 x, register u_int16 y);
extern u_int16 extraX;
u_int16 LcdTextOutXY16 (u_int16 x1, u_int16 y1, u_int16 x2, u_int16 *klut, char *s);

#define NO_SCREEN 0
#define NORMAL_SCREEN 1
#define BROWSER_SCREEN 2
extern u_int16 currentScreen;

#endif

