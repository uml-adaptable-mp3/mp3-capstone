/// \file simplebuttons.h Simple touchscreen buttons handler for VSOS
/// \author Panu-Kristian Poiksalo, VLSI Solution Oy 2012

#ifndef SIMPLEBUTTONS_H
#define SIMPLEBUTTONS_H

/// Abstraction of a rectangular area on the (LCD) screen
typedef struct simpleButtonStruct {
	u_int16 state,result,x1,y1,x2,y2;
	char *caption;
} SimpleButton;
extern SimpleButton *lastButtonPressed;

auto void RaisedBevel(u_int16 __a1 x1, __a0 y1, __b1 x2,__b0 y2);
auto void LoweredBevel(u_int16 __a1 x1, __a0 y1, __b1 x2,__b0 y2);

#define NormalBevel RaisedBevel
#define SelectedBevel LoweredBevel

void RenderSimpleButtons(SimpleButton *buttons);
u_int16 GetSimpleButtonPress(SimpleButton buttons[]);


#endif