/// Simple main menu for VS1005 developer boards.
/// \author Panu-Krisitan Poiksalo, VLSI Solution Oy
/// Directory searching utilities by Henrik Herranen, VLSI Solution Oy

/* For free support for VSIDE, please visit www.vsdsp-forum.com */

// Starting point template for creating VSOS applications.
// This will create a <projectname>.AP3 file, which you can copy to 
// your VS1005 Developer Board and run it from there.

// If you rename your application to INIT.AP3, then the kernel will
// load it automatically after booting.

#include <vo_stdio.h>
#include <vsos.h>
#include <sysmemory.h>
#include <string.h>
#include <timers.h>
#include "vo_fatdirops.h"
#include <stdbuttons.h>
#include <lcd.h>
#include "configureTouch.h"
#include <parseFileParam.h>
#include <vs1005h.h>
#include <apploader.h>
#include <imem.h>
#include <kernel.h>

// Result values for some fixed buttons... 
#define CONFIGURE -100 //arbitrary distinctive negative value other than -1
#define INFO -101

/* Temporary storage for file names */
#define FILE_NAME_CHARS 256
char fileName[FILE_NAME_CHARS];

#define NUMBER_OF_BUTTONS 20 //up to this many buttons
StdButton buttons[NUMBER_OF_BUTTONS+1];
char buttonFileName[NUMBER_OF_BUTTONS][FILE_NAME_CHARS];
 

#define SETTINGS_LINE_MAX_SIZE 80

void ReadTouchSettings(void) 	{
	struct TouchInfo *touchInfo = pTouchInfo;
	FILE *f = fopen("S:VSOS.INI", "r");

	if (f) {
		static char s[SETTINGS_LINE_MAX_SIZE];
		char *s2;
		while (!feof(f) && (s2 = fgets(s, SETTINGS_LINE_MAX_SIZE, f))) {
			u_int16 t;
			if ((t = FileParamInt(s, "touchMinX", 0xFFFFU)) != 0xFFFFU) {
				touchInfo->minX = t;
			}
			if ((t = FileParamInt(s, "touchMinY", 0xFFFFU)) != 0xFFFFU) {
				touchInfo->minY = t;
			}
			if ((t = FileParamInt(s, "touchMaxX", 0xFFFFU)) != 0xFFFFU) {
				touchInfo->maxX = t;
			}
			if ((t = FileParamInt(s, "touchMaxY", 0xFFFFU)) != 0xFFFFU) {
				touchInfo->maxY = t;
			}
		}
		fclose(f);
	}	
}

void ResetLcd(void) {
	lcd0.x = lcd0.y = lcd0.clipx1 = lcd0.clipy1 = 0;
	lcd0.clipx2 = lcd0.width-1;
	lcd0.clipy2 = lcd0.height-1;
	lcd0.textColor = lcd0.defaultTextColor;
	lcd0.backgroundColor = lcd0.defaultBackgroundColor;
	LcdFilledRectangle(0,0,lcd0.clipx2,lcd0.clipy2,0,lcd0.backgroundColor);
}


void ListDevices() {
	int i;
	char s[80];
	u_int16 textColorSave = lcd0.textColor;
	HighlightText();
	printf("System devices:\n"); //Decorative listout of all devices
	NormalText();
	for (i=0; i<26; i++) {
		if (vo_pdevices[i]) {
			DiskGeometry g = {0, 0};
			printf("%c: ", 'A'+i);
			if (ioctl(vo_pdevices[i], IOCTL_GET_GEOMETRY, (void *)&g) != S_ERROR) {
				g.totalSectors >>= 1; /* Now in KiB */
				if (g.totalSectors < 10000) {
					printf("%4ldK", g.totalSectors);
				} else if (g.totalSectors /= 1024, g.totalSectors < 10000) {
					printf("%4ldM", g.totalSectors);
				} else {
					printf("%4ldG", g.totalSectors/1024);
				}
			} else {
				printf("      ");
			}
			printf(" %s", vo_pdevices[i]->Identify(vo_pdevices[i],0,0));
			if (vo_pdevices[i]->fs) {
				printf(", handled by %s",vo_pdevices[i]->fs->Identify(vo_pdevices[i]->fs,0,0));
			}
			printf(".\n");
			
		}
	}
	
	HighlightText();
	printf("Filesystem drivers:\n");
	NormalText();
	for (i=0; i<4; i++) {
		if (vo_filesystems[i]) printf("* %s\n",vo_filesystems[i]->Identify(vo_filesystems[i],0,0));
	}		
	lcd0.textColor = textColorSave;
}


void InfoScreen(void) {
	static StdButton okButton[2]={0};
	static char s[20];
	u_int16 x,y;
	ResetLcd();
	sprintf(s,"VSOS %d.%02d",osVersion/100,osVersion%100);
	ioctl(vo_stdout, IOCTL_START_FRAME, s);
	printf("VSOS, the VLSI Solution Operating System.\n");
	printf("(C) 2012-2014 VLSI Solution Oy\n");
	printf("VS1005 ROM %08lx\n\n",ReadIMem(0xFFFFu));
	ListDevices();

	RunLibraryFunction("LIBLIST",0,0);

	SetVirtualResolution(5,6);
	CreateStdButton(okButton,1,BTN_NORMAL,4,5,1,1,"Ok");
	RenderStdButtons(okButton);
	while(!GetStdButtonPress(okButton)){
		Delay(1);
	}
}
  

void MakeFileButtons(StdButton *buttons) {
	FILE *fp = NULL;
	volatile int ff;
	volatile int i = 0;
	u_int16 number_of_files;
		
	InitDirectory(&fp);	// setup file searching
	ResetFindFile(fp);
	
	number_of_files = FindFile(fp, fileName, FILE_NAME_CHARS, "ap3", ofmLast, etFile);
	if (number_of_files < 5) {
		number_of_files = 5;
	}	
	if (number_of_files > NUMBER_OF_BUTTONS-3) {
		number_of_files = NUMBER_OF_BUTTONS-3;
	}
	SetVirtualResolution(1,number_of_files); // for calls to CreateStdButton
	ResetFindFile(fp);

	// Find each .ap3 file on the system disk and create a button for them.	
	while ((ff = FindFile (fp, fileName, FILE_NAME_CHARS, "ap3", ofmNext, etFile)) >= 0) {
		//FindFile returns a double-string which contains the short and long file name.
		char *longName = &buttonFileName[i][0]+strlen(fileName)+1;		
		//copy the file name into array buttonFileName for safe keeping...
		memcpy(&buttonFileName[i][0], fileName, FILE_NAME_CHARS);
		
		CreateStdButton(&buttons[i],i+1,BTN_NORMAL,BTN_SLOT,i,1,1,longName);

		if (++i>=NUMBER_OF_BUTTONS) {
			goto finally;
		}
	}

	finally:	
	EndDirectory(&fp);
}



int main(void) {	
	static char title[50];


restart:
	ResetLcd();
	sprintf(title,"VSOS %d.%02d Main Menu",osVersion/100,osVersion%100);
	ioctl(vo_stdout, IOCTL_START_FRAME, title);
			
	SetVirtualResolution(5,3);
	CreateStdButton(&buttons[0],INFO,BTN_NORMAL,3,1,2,1,"Info");
	       
	CreateStdButton(&buttons[1],CONFIGURE,BTN_NORMAL,3,0,2,1,"Calibrate Touch");
	CreateStdButton(&buttons[2],-1,BTN_TEXT|BTN_NO_BEVEL|BTN_DISABLED,3,2,2,1,"Please select app");

	CreateStdButton(&buttons[3],0,BTN_INVISIBLE,0,0,3,3,""); //Temporary holder for group	
	SetClippingRectangleToButton(&buttons[3]); //Place new buttons inside this "button"	
	MakeFileButtons(&buttons[3]);

	RenderStdButtons(buttons);
	 	
	while (1) {
		s_int16 i;
		i = GetStdButtonPress(buttons);
		
		if (i==CONFIGURE) {
			ConfigureTouch();
			ReadTouchSettings();
			goto restart;
		}
		
		if (i==INFO) {
			InfoScreen();
			goto restart;
		}


		if (i>0) {
			fclose(appFile); //Close the handle to current appFile;
			sprintf(fileName,"E:%s",buttonFileName[i-1]);
			appFile = fopen(fileName,"rb"); //Open a new appFile
			printf("appFile %s p %p\n", fileName, appFile);
			if (appFile) {
				// Exit with appFile pointing to another file.
				// The kernel will then load the next appFile.
				ResetLcd();
				ioctl(vo_stdout, IOCTL_START_FRAME, "Console");
				return S_OK; 
			} else {
				ioctl(vo_stdout, IOCTL_START_FRAME, "Error");
				printf("Hmmh, seems I cannot open %s!\n",fileName);
				while(1) {
					//Stop.
				}
			}
		}
	}

	return S_OK;
}
