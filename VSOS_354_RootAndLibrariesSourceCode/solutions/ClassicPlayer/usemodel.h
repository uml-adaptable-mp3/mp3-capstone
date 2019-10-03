/// Load and use models such as MP3MODEL

#ifndef USEMODEL_H
#define USEMODEL_H

#include <vo_stdio.h>
#include <apploader.h>
#include <timers.h>
#include <uimessages.h>
#include <libaudiodec.h>

typedef struct SpectrumDataStruct {
	u_int16 type;
	//u_int16 floor;
	//u_int16 emphasis;
	u_int16 hasData;
	u_int16 n;
	u_int16 ch;
	s_int16 data[64];
} SpectrumData;

typedef struct ModelInfoStruct {
	void **lib; //The model library we're going to load
	int (*Call)(s_int16 index, u_int16 message, u_int32 value);
	void (*DecodeID3)(register FILE* fp,register UICallback callback); //MP3MODEL provides this function
	UiMessageListItem *capabilities; ///array of messages the model can understand and/or provide
	const AUDIO_DECODER *decoder; //Pointer to model's decoder
	u_int16 format;
	u_int16 decoderUpdated;
	u_int16 songLengthEstimate;
	SpectrumData spectrum;
	u_int16 nextSongNumber;
	u_int16 currentSongNumber;
	u_int16 songNumberChanged;
	u_int16 decodeTimeSeconds;
	u_int16 playFilePercent;
	u_int16 decodeTimeChanged;
} ModelInfo;

extern __mem_y ModelInfo model;

void ListCapabilities(); ///Print out a list of messages the model understands and provides
void ViewCallback(s_int16 index,u_int16 message,u_int32 value);
void CopySpectrumData (struct CodecServices *cs, s_int16 __mem_y *data, s_int16 n, s_int16 ch);
ioresult LoadModel (const char *modelName);
void DropModel();


u_int16 LcdTextOutXY16 (u_int16 x1, u_int16 y1, u_int16 x2, u_int16 *klut, char *s);

#endif