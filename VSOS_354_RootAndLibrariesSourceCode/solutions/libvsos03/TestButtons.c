#include <stdbuttons.h>
#include <vsos.h>
#include <timers.h>

void TestButtons(void) {
	u_int16 i;
	static StdButton buttons[20]={0};
	StdButton *btn = buttons;
	StdButton *textarea;	
	
	console.Ioctl(&console, IOCTL_START_FRAME, "Button Test");
	
	//for (i=0; i<255; i++) printf("%c",i);
	//DelayMicroSec(10000000);


	SetVirtualResolution(2,5);		
	i=0;
	CreateStdButton(btn++, i+1, BTN_CHECKABLE, BTN_SLOT, i, 1, 1, "Antti", NULL); i++;
	CreateStdButton(btn++, i+1, BTN_CHECKABLE | BTN_CHECKED | BTN_HIGHLIGHTED, BTN_SLOT, i, 1, 1, "Banaani", NULL); i++;
	CreateStdButton(btn++, i+1, BTN_INVISIBLE, BTN_SLOT, i, 1, 1, "Panaani", NULL); i++;
	CreateStdButton(btn++, i+1, BTN_COLORMOD1, BTN_SLOT, i, 1, 1, "Cristina", NULL); i++;
	CreateStdButton(btn++, i+1, BTN_COLORMOD2, BTN_SLOT, i, 1, 1, "Daavid", NULL); i++;
	CreateStdButton(textarea = btn++, i+1, BTN_TEXT | BTN_INVISIBLE, BTN_SLOT, i, 1, 1, "Epsilon", NULL); i++;
	CreateStdButton(btn++, -1, 0, BTN_SLOT, i, 1, 2, "The Big One", NULL);i++;
	CreateStdButton(btn++, i+1, 0, 1, 3, 1, 1, "Hoplaa", NULL); i++;

	LcdSetClippingRectangleToButton(&buttons[2]);
	SetVirtualResolution(3,1);
	CreateStdButton(btn++,10,BTN_CHECKABLE | BTN_CHECKED,BTN_SLOT,0,1,1,"Sub1",NULL);
	CreateStdButton(btn++,10,BTN_CHECKABLE,BTN_SLOT,1,1,1,"Sub2",NULL);
	CreateStdButton(btn++,10,BTN_CHECKABLE | BTN_LOWERED,BTN_SLOT,2,1,1,"Sub3",NULL);


	
	RenderStdButtons(buttons);	
	LcdSetClippingRectangleToButton(textarea);		
	i=1;	
	//StartTextInvert();
	while(i){		
		Delay(1000);
		printf("Hoplaa:%d. ",i++);
		buttons[10].flags = BTN_CHECKABLE;
		RenderStdButton(&buttons[10]);
		Delay(1000);
		printf("Hoplaa:%d. ",i++);
		buttons[10].flags = BTN_CHECKABLE | BTN_LOWERED;
		RenderStdButton(&buttons[10]);

	}
	//CreateStdButton(textarea=btn++, -1, BTN_TEXT | BTN_LOWERED | BTN_NO_FACE | BTN_NO_BEVEL, BTN_SLOT, i, 1, 2, "Text Area");i++;
	//CreateStdButton(textarea=btn++, -1, BTN_TEXT | BTN_LOWERED | BTN_NO_FACE, BTN_SLOT, i, 1, 2, "Text Area");i++;
	//CreateStdButton(textarea=btn++, -1, BTN_INVISIBLE, BTN_SLOT, i, 1, 2, NULL);i++;

}
