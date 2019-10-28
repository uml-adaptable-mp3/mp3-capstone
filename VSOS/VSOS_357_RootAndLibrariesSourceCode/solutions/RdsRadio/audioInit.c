#include <vstypes.h>
#include <aucommon.h>
#include "audioInit.h"




void InitAudio(void) {
  s_int32 sampleRate;
  ioresult ior;
  s_int16 i;
	
  // Set input audio path to FM receiver
  ior = ioctl(stdaudioin, IOCTL_AUDIO_SELECT_INPUT, (void *)(AID_FM|AID_DEC6));
	
  sampleRate = 32000;
  ior = ioctl(stdaudioin, IOCTL_AUDIO_SET_IRATE, (void *)(&sampleRate));
  ior = ioctl(stdaudioin, IOCTL_AUDIO_GET_IRATE, (void *)(&sampleRate));

  if (ior == S_OK) {
    ior = ioctl(stdaudioout, IOCTL_AUDIO_SET_ORATE, (void *)(&sampleRate));
  }

  /* Remove initial pop/click sound after startup */
  for (i=0; i<64; i++) {
    u_int16 t[16];
    fread(t, 1, sizeof(t), stdaudioin);
  }
}
