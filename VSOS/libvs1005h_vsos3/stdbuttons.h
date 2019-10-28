/// \file stdbuttons.h Std touchscreen buttons handler for VSOS
/// \author Panu-Kristian Poiksalo, VLSI Solution Oy 2012

/// Some functons declared in this h file come from the libvsos02.a library.

#ifndef StdBUTTONS_H
#define StdBUTTONS_H

#include <vstypes.h>

// Button state: 0 is "normal" button, other bits are variations
#define BTN_END 0
#define BTN_END_OF_LIST 0
#define BTN_NORMAL 0
#define BTN_PRESSED (1<<0)
#define BTN_NO_BACKGROUND (1<<1)
#define BTN_NO_BEVEL (1<<2)
#define BTN_LOWERED (1<<3)
#define BTN_NO_FACE (1<<4)
#define BTN_HIGHLIGHTED (1<<5)
#define BTN_NO_CAPTION (1<<6)
#define BTN_TEXT (1<<7)
#define BTN_DISABLED (1<<8)
#define BTN_CHECKABLE (1<<9)
#define BTN_CHECKED (1<<10)
#define BTN_COLORMOD1 (1<<11)
#define BTN_COLORMOD2 (1<<12)
#define BTN_SLOT -1

#define BTN_INVISIBLE (BTN_NO_BACKGROUND | BTN_NO_BEVEL | BTN_NO_FACE | BTN_NO_CAPTION | BTN_DISABLED)



/// Abstraction of a rectangular area on the (LCD) screen
typedef struct stdButtonStruct {
	u_int16 flags;
	s_int16 result,x1,y1,x2,y2;
	char *caption;
	void *extraRenderInfo;
	void (*render)(register struct stdButtonStruct *button, register u_int16 op, register u_int16 x, register u_int16 y);
} StdButton;
extern StdButton *lastButtonPressed;

auto void RaisedBevel(u_int16 __a1 x1, __a0 y1, __b1 x2,__b0 y2);
auto void LoweredBevel(u_int16 __a1 x1, __a0 y1, __b1 x2,__b0 y2);

#define NormalBevel RaisedBevel
#define SelectedBevel LoweredBevel

/// Redraw all buttons on the buttons struct array.
auto void RenderStdButtons(register StdButton *buttons);
/// Redraw one button. 
auto void RenderStdButton(register StdButton *buttons);
/// Set virtual resolution. Call this before using CreateStdButton.
auto void SetVirtualResolution(register u_int16 screenWidth, register u_int16 screenHeight);
/// Calculate a position for a new button within the lcd0 clipping rectangle
/// using virtual resolution set by a call to SetVirtualResolution
/// Populate the button structure based on flags, result and virtual resolution info.
/// The created button shall fit inside the virtual rectangle, complete with 
/// borders so that buttons right next to each other render beautifully on the screen.
/// If x equals -1, y is evaluated as an index to the pixels of virtual resolution,
/// which is useful for creating an array of buttons each with virtual height and width of 1.
/// Sets default renderer.
auto void CreateStdButton(register StdButton *button,  register u_int16 result, register u_int16 flags,
						register s_int16 x, register s_int16 y, register u_int16 w, register u_int16 h, register const char *caption);
/// Do a single step of whatever is necessary to track button states. 
/// Don't use too much time to do it, and return 0 if there is no buttonpress to report.
/// If a button is pressed, return its result value.
/// For a touch screen, this function is relatively straightforward,
/// for a non-touchscreen approach, this function may update the screen if needed.
auto s_int16 GetStdButtonPress(register StdButton buttons[]);
/// Set the screen's clipping rectangle to the coordinates of the button.
/// The clipping rectangle can be slightly different if the button has no bevel or no background.
auto void SetClippingRectangleToButton(register StdButton *button);


/// The default render method. Not visible to apps, called by RenderStdButton.
/// If you write your own render method, use a prototype which is compatible with this one.
void StdButtonDefaultRender(register const StdButton *button, register u_int16 op, register u_int16 x, register u_int16 y);

#define BTN_OP_UNSPECIFIED 0
#define BTN_OP_PRESSING 1

auto void InitStdButtons(void);

/// Prototype for testing
void TestButtons(void);

#endif
