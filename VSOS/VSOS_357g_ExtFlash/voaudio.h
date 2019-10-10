#ifndef VO_AUDIO_H
#define VO_AUDIO_H
#include <audio.h>
#include <vsos.h>

extern s_int32 peripClkHz;
extern __y struct AUDIOPTR audioPtr;
extern __y u_int32 timeCount;
extern s_int32 peripClkHz;
extern struct Codec *voCodecMpg;
extern const SIMPLE_FILE audioout;

#endif