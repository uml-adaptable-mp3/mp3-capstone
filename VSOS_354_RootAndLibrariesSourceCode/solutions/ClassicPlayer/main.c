/// \file main.c VLSI Classic Player 0.1 for VSOS 3.21
/// MP3 player software with classic UI style from the days when MP3 format was born and we all fell in love with it.

/// \author Panu-Kristian Poiksalo, VLSI Solution Oy

/// Uses MP3MODEL to handle file playing - this is really just a user interface to command and visualize the model.
/// Uses ID3 decoder from MP3MODEL. MP3MODEL Uses AUDIODEC, DECMP3, DECWMA, DECAAC, DECVORB, DECFLAC and other codec libs.
/// Uses STDBUTTONS with a couple of custom renderes. Use the "normal" STDBTCH - if you change it the UI will look weird.
/// Uses LCD288 horizontal - this software has fixed screen coordinates that require a 320x240 LCD screen resolution.

/* For free support for VSIDE, please visit wwwindow.vsdsp-forum.com */

#include <vo_stdio.h>
#include <volink.h>     
#include <apploader.h>  
#include <vs1005h.h>
#include <timers.h>
#include <uimessages.h>
#include <string.h>
#include <stdbuttons.h>
#include <audio.h>
#include <kernel.h>
#include "lcdlayer.h" // Support for handling a dialog window on top of LCD client area - not used yet
#include "usemodel.h" // Easily load and use VSOS models such as MP3MODEL
#include "visualize.h" // MP3 visualizer - work in progress
#include "screenlayout.h" // Screen coordinate definitions
#include "lcdstuff.h" // Random functions for drawing random stuff on the screen
#include "browsefiles.h" // File browser
#include "plfatindex.h" // FAT Indexed Playlist - the heart of this program 



#define BBW 40

void ProgressBarRender (register struct stdButtonStruct *button, register u_int16 op, register u_int16 x, register u_int16 y);
void VolumeButtonRender (register struct stdButtonStruct *button, register u_int16 op, register u_int16 x, register u_int16 y);
char *fmtIndex[] = {"    ","MP3 ","WAV ","OGG ","FLAC","ALAC","WMA ","AAC ","MIDI"};
u_int16 jumpTo;

#define BUTTON_FILE -1
#define BUTTON_PLAYLIST -2
#define BUTTON_EASTER_EGG -2
#define BUTTON_LEFT -3
#define BUTTON_RIGHT -4
#define BUTTON_PROGRESSBAR -5
#define BUTTON_VOLUME_UP -6
#define BUTTON_VOLUME_DOWN -7

/// Main user interface StdButtons - declared this way to save program memory. Fixed locations for 320x240 resolution
StdButton buttons[] = {
	{BTN_NORMAL, BUTTON_FILE, 3,3,40,20,"File",NULL,NULL},
	{BTN_NO_FACE|BTN_NO_BEVEL,  BUTTON_EASTER_EGG, /*43*/150,3,310,20,"",NULL,NULL},
	{BTN_NORMAL,  BUTTON_LEFT,  PANEL_RIGHT - 64, Y_NEXTSONG, PANEL_RIGHT - 34, Y_NEXTSONG + 20, "\xab",NULL,NULL},
	{BTN_NORMAL,  BUTTON_RIGHT, PANEL_RIGHT - 30, Y_NEXTSONG, PANEL_RIGHT, Y_NEXTSONG + 20, "\xbb",NULL,NULL},
	{BTN_NO_FACE, BUTTON_PROGRESSBAR, PROGRESSBAR_X1-1, PROGRESSBAR_Y1-1, PROGRESSBAR_X2+1, PROGRESSBAR_Y2+1 , "",NULL,ProgressBarRender},
	{BTN_NO_FACE, UIMSG_BUT_NEXT, PANEL_LEFT+32, Y_NEXTSONG, PANEL_RIGHT - 68, Y_NEXTSONG + 20, "", NULL, NULL},
	{BTN_NO_FACE|BTN_NO_BEVEL, UIMSG_BUT_PAUSE_TOGGLE, PANEL_LEFT, 30, PANEL_RIGHT, Y_NEXTSONG - 10, "", NULL, NULL},
	{BTN_NORMAL,  UIMSG_BUT_STOP,     PANEL_BORDER_LEFT + 0*BBW, Y_CURRENTSONG + 119, PANEL_BORDER_LEFT + 1*BBW -3, 234,"\x1", NULL, NULL},
	{BTN_NORMAL,  UIMSG_BUT_FIRST,    PANEL_BORDER_LEFT + 1*BBW, Y_CURRENTSONG + 119, PANEL_BORDER_LEFT + 2*BBW -3, 234,"\x81\xab", NULL, NULL},
	{BTN_NORMAL,  UIMSG_BUT_PREVIOUS, PANEL_BORDER_LEFT + 2*BBW, Y_CURRENTSONG + 119, PANEL_BORDER_LEFT + 3*BBW -3, 234,"\xab-", NULL, NULL},
	{BTN_NORMAL,  UIMSG_BUT_NEXT,     PANEL_BORDER_LEFT + 3*BBW, Y_CURRENTSONG + 119, PANEL_BORDER_LEFT + 4*BBW -3, 234,"-\xbb", NULL, NULL},
	{BTN_NORMAL,  UIMSG_BUT_VOLUME_UP,   PANEL_BORDER_RIGHT- 1*BBW+3, Y_CURRENTSONG + 119, PANEL_BORDER_RIGHT- 0*BBW , 234,"\x8f+", NULL, VolumeButtonRender},
	{BTN_NORMAL,  UIMSG_BUT_VOLUME_DOWN, PANEL_BORDER_RIGHT- 2*BBW+3, Y_CURRENTSONG + 119, PANEL_BORDER_RIGHT- 1*BBW , 234,"\x8f-", NULL, VolumeButtonRender},
	{0}
};
StdButton noButton;

/// Array of structures to draw bevels - declared this way to save program memory
__mem_y struct BevelStruct {int x1; int y1; int x2; int y2; u_int16 *colors;} bevels[] = {
	{0,0,319,239,raised},	
	{2,PANEL_CLIENT_Y1,317,237,lowered}, //Client Area
	{PANEL_BORDER_LEFT,Y_TIME+0,PANEL_DIVIDER_1+3,Y_CURRENTSONG-4,raised}, //Time display raised bevel
	{PANEL_LEFT,Y_TIME+2,PANEL_DIVIDER_1+0,Y_CURRENTSONG-6,lowered_text}, //Time display
	{PANEL_DIVIDER_1+7,Y_TIME+0,PANEL_BORDER_RIGHT,Y_CURRENTSONG-4,raised}, //Visualizer raised bevel
	{PANEL_DIVIDER_1+10,Y_TIME+2,PANEL_RIGHT,Y_CURRENTSONG-6,lowered_text}, //Visualizer display
	{PANEL_BORDER_LEFT,Y_CURRENTSONG,PANEL_BORDER_RIGHT,Y_CURRENTSONG+115,raised}, //Current song raised bevel
	{PANEL_LEFT,Y_CURRENTSONG+12,PANEL_RIGHT,Y_CURRENTSONG+74,lowered_text},//Song, Artist, Album display
	{0,0,0,0,NULL}
};

void DrawBevelStruct(__mem_y register struct BevelStruct *b) {
	DrawBevel(b->x1,b->y1,b->x2,b->y2,b->colors);	
}

/// Custom StdButton renderer for the volume buttons
void VolumeButtonRender (register struct stdButtonStruct *button, register u_int16 opcode, register u_int16 x, register u_int16 y) {
	SystemUiMessage(0,button->result,1);
	StdRender(button,opcode,x,y);
	Delay(20);
}

/// Custom StdButton renderer for the Progress Bar
void ProgressBarRender (register struct stdButtonStruct *button, register u_int16 opcode, register u_int16 x, register u_int16 y) {
	StdRender(button,opcode,x,y);
	if (opcode==1) {
		jumpTo = ((x - PROGRESSBAR_X1) * (u_int32)model.songLengthEstimate) / (u_int32)(PROGRESSBAR_X2 - PROGRESSBAR_X1);
	}
}


/// First phase of repaint - clears screen and initializes StdRender and LCD structures
void Repaint1() {
	s_int16 i;
	currentScreen = NO_SCREEN;
	DrawBevelStruct(&bevels[0]);
	DrawBevelStruct(&bevels[1]);
	// Initialize buttons and get pointer to std renderer first, because other modules might need it
	CreateStdButton(&noButton,0,0,0,0,0,0,NULL); //Create this object just to populate its methods
	StdRender = noButton.render; //We need the pointer to StdButtons default renderer because we create buttons manually
	for (i=0; i<sizeof(buttons)/sizeof(buttons[0]); i++) {
		if (!buttons[i].render) buttons[i].render = StdRender;
	}
	
	lcd0.backgroundColor = lcd0.defaultBackgroundColor = lcd0.buttonFaceColor = COLOR_WINDOW;
	lcd0.y = lcd0.clipy1 = 160;
	lcd0.clipy2 = lcd0.clipy1+30;
	lcd0.x = lcd0.clipx1 = 10;
	lcd0.clipx2 = 310;

}

/// Complete repaint - draws the main UI screen
void Repaint() {
	u_int16 i;
	__mem_y struct BevelStruct *b = &bevels[2];
	Repaint1();			
	LcdTextOutXY16(150,3,317,greyText,"VLSI Classic Player 0.1");

	while (b->colors) {
		DrawBevelStruct(b++); //Draw all bevels. Done this way instead of separate calls to save program memory
	}
	PutDigits();

	lcd0.textColor = COLOR_DARK;
	LcdTextOutXY(PANEL_LEFT+1,Y_CURRENTSONG+2,"Now Playing");
	LcdTextOutXY(PANEL_LEFT+1, Y_NEXTSONG+2, "Next");
	LcdTextOutXY(PANEL_LEFT+1, Y_NEXTSONG+10, "Song");
	LcdTextOutXY16(150,3,317,greyText,"VLSI Classic Player 0.2");
	lcd0.textColor = COLOR_WHITE;
	currentScreen = NORMAL_SCREEN;

}


/// VLSI Classic Player Proper
int main(void) {
	s_int16 r=0; //Button Result
	strcpy(path,"@:"); //special startup value: '@'=='A'-1.
	if (LoadModel("MP3MODEL") != S_OK) return SysError ("Cant ld model");
	goto FileSelect; //Start the player as if the "File" button were pressed
	
	while(r != UIMSG_BUT_STOP) {
		char *s;
		Visualize();
		r=GetStdButtonPress(buttons);
		if (r>0) {
			model.Call(0, r, 1); //retransmit the buttons's result as a uiMessage to the model
		}
		if (r==BUTTON_FILE) {
			FileSelect:
			Repaint1();
			currentScreen = BROWSER_SCREEN;
			SelectFileFromFolder();
			Repaint();
			RenderStdButtons(buttons);
			model.Call(0, UIMSG_S16_SONG_NUMBER, currentSongNumber);
		} else if (r==-2) {
			//RunProgram("LIBLIST2",NULL);
			ViewCallback(0, UIMSG_TEXT_SONG_NAME, (long)"VLSI Classic Player");
			ViewCallback(0, UIMSG_TEXT_ALBUM_NAME, (long)"VLSI Solution Oy 2015");
			ViewCallback(0, UIMSG_TEXT_ARTIST_NAME, (long)"Panu-Kristian Poiksalo");
		} else if (r==BUTTON_LEFT) {
			if (model.nextSongNumber > 1) {
				model.nextSongNumber--;
				model.Call(0,UIMSG_S16_NEXT_SONG_NUMBER,model.nextSongNumber);
				goto UpdateNextSongInfo;
			}
		} else if (r==BUTTON_RIGHT) {
			if (model.nextSongNumber < nFiles) {
				model.nextSongNumber++;
				model.Call(0,UIMSG_S16_NEXT_SONG_NUMBER,model.nextSongNumber);
				goto UpdateNextSongInfo;
			}
		} else if (r==BUTTON_PROGRESSBAR) {
			model.Call(0,UIMSG_U32_PLAY_TIME_SECONDS,jumpTo);
		}
		
		if (model.songNumberChanged) {
			model.songNumberChanged = 0;
			if (model.currentSongNumber < nFiles) {
				model.nextSongNumber = model.currentSongNumber + 1;
			} else {
				model.nextSongNumber = 1;
				model.Call(0,UIMSG_S16_NEXT_SONG_NUMBER,1);
			}
			PutDigits();
			UpdateNextSongInfo:			
			DrawBevel(PANEL_LEFT + 32, Y_NEXTSONG, PANEL_RIGHT-68, Y_NEXTSONG + 20, raised);
			s = GetPlayListItemInfo(model.nextSongNumber);
			LcdTextOutXY16(PANEL_LEFT + 36, Y_NEXTSONG+2, PANEL_RIGHT-70, blackText, s);
			lcd0.textColor = COLOR_BLACK;
			LcdTextOutXY(PANEL_LEFT+90,Y_CURRENTSONG+2,fmtIndex[model.format]);
			lcd0.textColor = COLOR_WHITE;
		}
		if (model.decodeTimeChanged) {
			model.decodeTimeChanged = 0;
			PutDigits();
			{
				u_int16 i = (((PROGRESSBAR_X2-PROGRESSBAR_X1) * model.playFilePercent) / 100) + PROGRESSBAR_X1;
				LcdFilledRectangle(PROGRESSBAR_X1,PROGRESSBAR_Y1, i, PROGRESSBAR_Y2, NULL, __RGB565RGB(0x4f,0xff,0x4f));
				LcdFilledRectangle(i,PROGRESSBAR_Y1, PROGRESSBAR_X2, PROGRESSBAR_Y2, NULL, COLOR_BLACK);							
			}

		}
	}			
	return S_OK;
}


// Handle unloading from memory
void fini(void) {
	DropModel();
	FiniLcdLayer();
}

