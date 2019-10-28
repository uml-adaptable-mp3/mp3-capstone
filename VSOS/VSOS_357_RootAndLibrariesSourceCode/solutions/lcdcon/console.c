/* For free support for VSIDE, please visit www.vsdsp-forum.com */

#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <lcd.h>
#include <vsos.h>


//int register __a0 putchar(int register __a0 c); //from Monitor
void ConsoleFrame(const char *caption) {
	u_int16 w = lcd0.width-1;
	u_int16 h = lcd0.height-1;
	
	lcd0.clipx1 = lcd0.clipy1 = 0;
	lcd0.clipx2 = lcd0.width - 1;
	lcd0.clipy2 = lcd0.height - 1;
	LcdFilledRectangle(0,0,lcd0.clipx2,lcd0.clipy2,0,lcd0.backgroundColor);
	LcdFilledRectangle(1,0,w-1,8,0,lcd0.defaultTextColor);
	LcdFilledRectangle(0,1,w,7,0,lcd0.defaultTextColor);
	lcd0.textColor = lcd0.defaultBackgroundColor;
	lcd0.backgroundColor = lcd0.defaultTextColor;
	LcdTextOutXY(5,1,caption);
	lcd0.textColor = lcd0.shadowColor;
	LcdTextOutXY(w-10,0,"x");
	lcd0.textColor = lcd0.defaultTextColor;		
	lcd0.backgroundColor = lcd0.defaultBackgroundColor;
	lcd0.clipy1 = 12;
	lcd0.x = lcd0.clipx1;
	lcd0.y = lcd0.clipy1;
}

s_int32 ConsoleIoctl (register __i0 VO_FILE *self, s_int16 request, IOCTL_ARGUMENT arg) {
	if (request == IOCTL_START_FRAME) {
		ConsoleFrame((char*)arg);
	} else {
		printf("Console IOCTL %d\n",request);
	}
};

u_int16  ConsoleWrite (register __i0 VO_FILE *self, void *buf, u_int16 sourceIndex, u_int16 bytes) {
	unsigned char ch = (((unsigned char*)buf)[0]); //since console is a console, only characters are written to it. Hence bytes is always 1.
	if (ch == '\r') {
		lcd0.x = lcd0.clipx1;
	}
	if (ch == '\n') {
		lcd0.x = lcd0.clipx2;
	}
	if (lcd0.x+7 > (lcd0.clipx2)) {
		lcd0.x = lcd0.clipx1;
		lcd0.y += 8;
		if (lcd0.y + 9 >= lcd0.clipy2) {
			lcd0.y = lcd0.clipy1;
		}
		{
			LcdFilledRectangle(lcd0.x, lcd0.y, lcd0.clipx2, lcd0.y+10, 0, lcd0.backgroundColor);
		}
	}
	if((ch != '\n') && (ch != '\r')) {
		unsigned char text[2];
		text[0] = ch;
		text[1] = 0;
		LcdTextOutXY(lcd0.x+1, lcd0.y, text);
		lcd0.x += 7;
	}
			
};

