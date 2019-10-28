#ifndef __STD_AUDIO_H__
#define __STD_AUDIO_H__

#include "vo_stdio.h"
#include <vstypes.h>
#include <codec.h>
struct StdAudioOut;
typedef struct StdAudioOut STD_AUDIO_OUT;

struct StdAudioOut {
  struct CodecServices cs;
  FILE *inFp;
  struct Codec *cod;
  s_int16 pause;
  u_int32 samplesSinceLastDelay;
  void (*callback)(STD_AUDIO_OUT *sao, u_int16 samples);
};

extern struct StdAudioOut stdAudioOut;

STD_AUDIO_OUT *OpenStdAudioOut(const char *name,
			       void (*callback)(STD_AUDIO_OUT *sao,
						u_int16 samples));
int CloseStdAudioOut(STD_AUDIO_OUT *sao);


#endif /* __STD_AUDIO_H__ */
