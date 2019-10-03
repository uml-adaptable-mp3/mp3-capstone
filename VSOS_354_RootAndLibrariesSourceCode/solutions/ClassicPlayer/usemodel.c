/// \file usemodel.c Functionality to easily load and use models such as MP3MODEL
/// \author Panu-Kristian Poiksalo, VLSI Solution Oy

#include "usemodel.h"
#include <string.h>
#include <libaudiodec.h>
#include <lcd.h>
#include <volink.h>
#include "screenlayout.h"
#include "lcdstuff.h"
#include "plfatindex.h"

__mem_y ModelInfo model;

// Load a model library; usually MP3MODEL (S:SYS/MP3MODEL.DL3)
ioresult LoadModel (const char *modelName) {
	model.lib = LoadLibrary(modelName); 
	if (!model.lib) return S_ERROR;
	model.Call = model.lib[4]; //DLL entry to ModelCallback is in fixed position.
	model.DecodeID3 = model.lib[5]; //We can use MP3MODEL's exported DecodeID3 function - DANGER IF the model is NOT MP3MODEL!
	model.Call(0,UIMSG_VSOS_SET_CALLBACK_FUNCTION,(u_int32)ViewCallback);
	model.Call(0,UIMSG_VSOS_SET_SPECTRUM_ANALYZER_CALLBACK,(u_int32)CopySpectrumData);
	model.Call(0,UIMSG_VSOS_SET_CURRENT_PLAYLIST,(u_int32)&plFatIndex);
	model.Call(0,UIMSG_VSOS_GET_MSG_LIST,NULL);
	return S_OK;
}

// Drop the model library
void DropModel() {
	if (model.lib) {
		DropLibrary((void*)model.lib);
		model.lib = NULL;
	}
}


/// This is the handler for all messages coming from the model.
/// It's an essential piece of both basic functionality and the user interface
void ViewCallback(s_int16 index, u_int16 message, u_int32 value) {
	static u_int16 songStarting = 0;
	//fprintf(stderr,"MP3VIEW:%04x[%d]=%ld.\n", message,index,value);
	if (message == UIMSG_TEXT_LONG_FILE_NAME) {
	 	message = UIMSG_TEXT_SONG_NAME; //Treat file name as if it were song name
	}
	if (message == UIMSG_VSOS_MSG_LIST) {
		model.capabilities = (void*)value;
	} else if (message == UIMSG_VSOS_AUDIO_DECODER_LOADED) {
		model.decoder = (void*)value;
		model.format = index<9 ? index : 0;
		if (index) { //Song starting
			songStarting = 1;
		} else { //Song has ended
			if (currentScreen == NORMAL_SCREEN) {
				DrawBevel(PANEL_LEFT,Y_CURRENTSONG+12,PANEL_RIGHT,Y_CURRENTSONG+74,lowered_text);
			}
		}
	} else if (message == UIMSG_S16_PLAYING) {
		if (songStarting) {
			model.currentSongNumber = index;
			model.songNumberChanged = 1;
			songStarting = 0;
		}
	} else if (message == UIMSG_U32_PLAY_TIME_SECONDS) {
		FILE *f = model.decoder->inFp;
		model.decodeTimeSeconds = value;
		model.songLengthEstimate = ((float)value*(float)(f->fileSize))/(float)f->pos;	
		model.decodeTimeChanged = 1;
	} else if ((message >= UIMSG_TEXT_SONG_NAME) && (message <= UIMSG_TEXT_ARTIST_NAME) && (currentScreen == NORMAL_SCREEN)) {
		// Show Now playing Song Name, Album, Artist
		u_int16 n = message - UIMSG_TEXT_SONG_NAME;
		u_int16 x = LcdTextOutXY16(15,(n=Y_CURRENTSONG+15+n*20),PANEL_RIGHT-3, greenText,(char*)value);
		LcdFilledRectangle(x,n,PANEL_RIGHT-1,n+16,NULL,COLOR_BLACK);
	} else if (message == UIMSG_U32_PLAY_FILE_PERCENT) {
		model.playFilePercent = value;
	}
}


//For visualization - work in progress...
void CopySpectrumData (struct CodecServices *cs, s_int16 __mem_y *data, s_int16 n, s_int16 ch) {
	if (model.spectrum.hasData) return;
	model.spectrum.n = n;
	model.spectrum.ch = ch;
	memcpyYY(&model.spectrum.data[ch?0:30], data, 30);
	if(ch==1) model.spectrum.hasData = 1;
}





/*
///Print out a list of messages the model understands and provides
void ListCapabilities() {
	UiMessageListItem *u = model.capabilities;
	if (u) {
		while (u->message_type != UIMSG_VSOS_END_OF_LIST) {
			if (u->message_type == UIMSG_INPUTS) {
				printf("In: ");
			} else if (u->message_type == UIMSG_OUTPUTS) {
				printf("Out: ");
			} else {
				printf(" %04X ",u->message_type);
			}
			printf("%s\n",u->caption);
			u++;
		}
	}
}
*/
