/// \file stdbuttons.c Std touchscreen buttons handler for VSOS
/// \author Panu-Kristian Poiksalo, VLSI Solution Oy 2012

#include <vstypes.h>
#include "stdbuttons.h"
#include <vo_stdio.h>
#include <string.h>
#include <timers.h>
#include <clockspeed.h>
 
StdButton *lastButtonPressed = NULL;
u_int16 virtualWidth = 240;
u_int16 virtualHeight = 320;



void StdButtonDefaultRender(register const StdButton *button, register u_int16 op, register u_int16 x, register u_int16 y) {
	if (op == BTN_OP_UNSPECIFIED) {
		if (button->flags & BTN_LOWERED) {
		}
		if (!(button->flags & BTN_NO_CAPTION)) {
			printf("% 5d ",button->result);
			if (button->flags & BTN_CHECKABLE) {
				printf("[%c] ",(button->flags & BTN_CHECKED) ? 'X' : ' ');
			}
			printf(button->caption);
			printf("\n");
		}
	}

}  

auto void RenderStdButton(register const StdButton *button) {
	button->render(button,0,0,0);
} 

auto void CreateStdButton(register StdButton *button, register u_int16 result, register u_int16 flags, register s_int16 x, 
	register s_int16 y, register u_int16 w, register u_int16 h, register const char *caption){
	button->flags = flags;
	button->result = result;
	button->caption = caption;
	button->render = StdButtonDefaultRender;
	//printf("Create button %S\n",caption);
}


#include "iochannel.h" 
auto s_int16 GetStdButtonPress(register StdButton *buttons) {
	//RenderStdButtons(buttons);
	return 0;	
}

auto void SetClippingRectangleToButton(register StdButton *button){
}

