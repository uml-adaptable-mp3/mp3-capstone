/// \file lcd.h Calling convention for graphical LCD displays
/** 
	LCD NOTES
	- By default, TFT displays in VS1005 systems are kept in 16 bits per pixel, RGB565 color mode.
*/

#ifndef LCD_H
#define LCD_H

#include <vstypes.h>
#include "rgb565.h"
#define COLOR_BTN_FACE __RGB565RGB(220,220,220)
#define COLOR_SHADOW __RGB565RGB(128,128,128)
#define HighlightText() {lcd0.textColor = lcd0.highlightColor;}
#define NormalText() {lcd0.textColor = lcd0.defaultTextColor;}
#define ShadowText() {lcd0.textColor = lcd0.shadowColor;}
#define StartTextInvert() {lcd0.textColor = lcd0.defaultBackgroundColor; lcd0.backgroundColor = lcd0.defaultTextColor;}
#define EndTextInvert() {lcd0.textColor = lcd0.defaultTextColor; lcd0.backgroundColor = lcd0.defaultBackgroundColor;}

/// Info structure of LCD display state
typedef struct lcdInfoStruct {
	u_int16 width; ///< width of visible area in pixels
	u_int16 height; ///< height of visible area in pixels
	u_int16 x; ///< Optional X coordinate of logical cursor in pixels. Origin is left top corner.
	u_int16 y; ///< Optional Y coordinate of logical cursor in pixels. Origin is left top corner.
	u_int16 textColor; ///< Current text color (RGB565). Used by the console, including printf.
	u_int16 backgroundColor; ///< Current background color (RGB565). Used as the background color for new text output.
	u_int16 defaultTextColor; ///< Defaut text color for this screen. Read by the console to set default console colors.
	u_int16 defaultBackgroundColor; ///< Default text background color for this screen. Read by the console.
	u_int16 clipx1,clipy1,clipx2,clipy2; ///< Current clipping rectangle (used for setting console text area)
	u_int16 shadowColor, highlightColor, buttonFaceColor,buttonTextColor;
} lcdInfo;

extern lcdInfo lcd0; ///< Information for the main LCD screen.

typedef struct ColoringStruct {
	u_int16 textColor;
	u_int16 textBackground;
	u_int16 textHighlight;
	u_int16 textShadow;
	u_int16 buttonText;
	u_int16 buttonFace;
	u_int16 buttonHighlight;
	u_int16 buttonShadow;
	u_int16 buttonHighlightFace;
	u_int16 buttonLoweredFace;
} Coloring;

extern Coloring colorScheme[4]; //allocated in stdbuttons.c

/// Start up the LCD display, initializes the LCD info struct and clears the screen to background color.
/// \param display_mode Optional parameter to tweak the display. Set at zero.
u_int16 LcdInit (u_int16 display_mode);
u_int16 InitDisplay (u_int16 display_mode); ///< VSOS api call for LcdInit
/// Creates a filled rectangle either with texture or solid color
/// \param x1 top left corner x coordinate, 0 is left edge of visible area, grows rightwards
/// \param y1 top left corner y coordinate, 0 is top edge of visible area, grows downwards
/// \param x2 bottom right corner x coordinate, must be more than or equal to x1
/// \param y2 bottom right corner y coordinate, must be more than or equal to y1
/// \param texture Pointer to texture bitmap (usually an array of RGB565 pixel values), left-to-right, top-to-bottom. If texture is NULL, solid color is used
/// \param color The RGB565 color of solid filled rectangle
u_int16 LcdFilledRectangle (u_int16 x1, u_int16 y1, u_int16 x2, u_int16 y2, u_int16 *texture, u_int16 color);
/// Write out a line of text using the current style, starting at pixel coordinates x,y
/// \param x top left corner x pixel coordinate of textbox, 0 is left edge of visible area, grows rightwards
/// \param y top left corner y pixel coordinate of textbox, 0 is top edge of visible area, grows downwards
/// \param s pointer to zero-terminated char* string

u_int16 LcdReadRectangle (u_int16 x1, u_int16 y1, u_int16 x2, u_int16 y2, u_int16 *texture);
u_int16 LcdTextOutXY (u_int16 x, u_int16 y, char *s);

#endif