/*

  FM Model (C) 2012-07-13 VLSI Solution Oy

*/
#ifndef VSOS_FM_MODEL_H
#define VSOS_FM_MODEL_H

#include <vstypes.h>

// number of channels to search
#define SEARCH_CH 6

#define FM_STEP 100
#define FM_FINE_STEP 10

// lower and upper bounds of proper FM frequency
// OIRT
//#define FM_LOW 65800L
//#define FM_HIGH 74000L

// Japan
//#define FM_LOW 76000L
//#define FM_HIGH 90000L

// Default
#define FM_LOW 87500L
#define FM_HIGH 108000L


// TEST ONLY
//#define FM_LOW 85000L
//#define FM_HIGH 112000L




/* Function pointer to caller callback function. */
//void (*fmCallbackFunction)(s_int16 index, u_int16 message, u_int32 value)
//  = NULL;
  

/* Calling this controls the model */
void SetCallbackFunction(register u_int32 value);
//void FmModelCallback(s_int16 index, u_int16 message, u_int32 value);
void SetFrequency(register u_int32 freq);
void ToggleMS(void);
void SetMono(void);
void SetStereo(void);

void SimpleNext(void);
void SimplePrev(void);
void SimpleTune(void);

void PleaseDie(void);

void FmModelTask(void);
void ParseTime(register struct rdsInfo *rdsInfo);

#define FM_AUDIO_BUFSIZE 128
extern s_int16 fmAudioBuf[FM_AUDIO_BUFSIZE];

extern u_int16 playerFlags;

#endif /* !VSOS_FM_MODEL_H */
