#ifndef _LCD_ILI9341_H_
#define _LCD_ILI9341_H_

/**
 * Low-level device driver for an LCD display.
 */

#include <vstypes.h>

#define SLEEP_ENTER                0x10
#define SLEEP_OUT                  0x11
#define GAMMA_SET                  0x26
#define DISPLAY_OFF                0x28
#define DISPLAY_ON                 0x29
#define SET_COLUMN_ADDRESS         0x2A
#define SET_PAGE_ADDRESS           0x2B
#define WRITE_MEMORY               0x2C
#define READ_MEMORY                0x2E
#define MEMORY_ACCESS_CONTROL      0x36
#define PIXEL_FORMAT_SET           0x3A
#define FRAME_RATE_CONTROL         0xB1
#define DISPLAY_FUNCTION_CONTROL   0xB6
#define POWER_CONTROL_1            0xC0
#define POWER_CONTROL_2            0xC1
#define VCOM_CONTROL_1             0xC5
#define VCOM_CONTROL_2             0xC7
#define POWER_CONTROL_A            0xCB
#define POWER_CONTROL_B            0xCF
#define POSITIVE_GAMMA_CORRECTION  0xE0
#define NEGATIVE_GAMMA_CORRECTION  0xE1
#define DRIVER_TIMING_CONTROL_A    0xE8
#define DRIVER_TIMING_CONTROL_B    0xEA
#define POWER_ON_SEQUENCE_CONTROL  0xED
#define UNDOCUMENTED_0xEF          0xEF
#define ENABLE_3G                  0xF2
#define INTERFACE_CONTROL          0xF6

// #define YELLOW __RGB565RGB(255, 255, 0)
// #define RED    __RGB565RGB(255, 0, 0)
// #define WHITE  lcd0.defaultTextColor
// #define GREEN  __RGB565RGB(0, 128, 0)


void TFTWriteRegister(u_int16 reg, u_int16 data);
void TFTWriteVector(s_int16 *vec);
u_int16 MyLcdFilledRectangle (u_int16 x1, u_int16 y1, u_int16 x2, u_int16 y2, u_int16 *texture, u_int16 color);
u_int16 MyLcdTextOutXY (u_int16 x1, u_int16 y1, char *s);
u_int16 LcdInit (u_int16 display_mode);

#endif  // _LCD_ILI9341_H_
