/**
   \file audiofs.c Audio Filesystem driver for VsOS
   \author Henrik Herranen, VLSI Solution Oy
*/

#include <vo_stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <vstypes.h>
#include <vs1005h.h>
#include <timers.h>
#include <vsos.h>
#include <sysmemory.h>
#include <ringbuf.h>
#include "auxi2sm.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

struct SmallAudio {
  u_int16 flags;
  u_int32 sampleRate;
};

struct SmallAudio smallAudio;

const struct I2SComb {
  u_int32 sampleRateAndHiBits;
  u_int16 pattern;
} __mem_y i2sComb[] = {
  { 24000L*2+1,  I2S_CF_FS24K32B},
  { 48000L*2+0,  I2S_CF_FS48K16B},
  { 48000L*2+1,  I2S_CF_FS48K32B},
  { 96000L*2+0,  I2S_CF_FS96K16B},
  { 96000L*2+1,  I2S_CF_FS96K32B},
  {192000L*2+0, I2S_CF_FS192K16B},
  {          0,               -1}
};


auto ioresult AudioSetRate(register s_int32 sampleRate,
			   register u_int16 hiBits) {
  struct I2SComb __mem_y *ic = i2sComb;
  sampleRate &= ~0x7F000000; /* Mask away fractional sample rate bits 30:24 */
  sampleRate <<= 1;
  if (hiBits) {
    sampleRate++;
  }

  while (ic->sampleRateAndHiBits != sampleRate) {
    if (!ic->sampleRateAndHiBits)
      return S_ERROR;
    ic++;
  }

  Disable();
  PERIP(I2S_CF) = 0;
  smallAudio.sampleRate = sampleRate;
  smallAudio.flags &= ~AIN_32BIT;
  if (hiBits) {
    smallAudio.flags |= AIN_32BIT;
  }
  PERIP(I2S_CF) = ic->pattern | (I2S_CF_MASTER | I2S_CF_ENAMCK | I2S_CF_ENA);
  Enable();

  return 0;
}



ioresult AudioClose(void) {
  PERIP(I2S_CF) = 0;
}



IOCTL_RESULT AudioIoctl(register __i0 VO_FILE *self, s_int16 request,
			IOCTL_ARGUMENT argp) {
  ioresult res = S_OK;

#if 0
  fprintf(stderr, "Ioctl(%p, %d, %p)\n", self, request, argp);
#endif

  switch (request) {
#if 0
  case IOCTL_START_FRAME:			
    break;
		
  case IOCTL_END_FRAME:
    break;
#endif

  case IOCTL_RESTART:
    /* Set to Perip Mode  I2S_DI    I2S_DO    I2S_BCK   I2S_FRM   I2S_12M */
    PERIP(GPIO1_MODE) |= ((1<<10) | (1<<11) | (1<<12) | (1<<13) | (1<<14));
    AudioSetRate(96000, 1);
    audioFile.flags = __MASK_PRESENT | __MASK_OPEN | __MASK_CHARACTER_DEVICE |
      __MASK_FILE |
      __MASK_READABLE | __MASK_WRITABLE;
    break;

  case IOCTL_AUDIO_GET_ORATE:
    *((u_int32 *)argp) = smallAudio.sampleRate>>1;
    break;

  case IOCTL_AUDIO_SET_ORATE:
    res = AudioSetRate(*((u_int32 *)argp), smallAudio.flags & AIN_32BIT);
    break;

  case IOCTL_AUDIO_SET_RATE_AND_BITS:
    {
      s_int32 rate = *((u_int32 *)argp);
      s_int16 hiBits = (rate < 0);
      rate = labs(rate);
      res = AudioSetRate(rate, hiBits);
    }
    break;

  case IOCTL_AUDIO_GET_BITS:
    return (smallAudio.flags & (AIN_32BIT)) ? 32 : 16;
    break;

  case IOCTL_AUDIO_SET_BITS:
    res = AudioSetRate(smallAudio.sampleRate, (u_int16)argp > 16);
    break;

  default:
    res = S_ERROR;
    break;
  }

  return res;
}

char *Identify(register __i0 void *self, char *buf, u_int16 bufSize) {
  strncpy(buf, "AUOI2SMA", bufSize);
  buf[bufSize-1] = '\0';
  return buf;
}

FILEOPS audioFileOps = {
  NULL, /* Open() */
  NULL, /* Close() */
  AudioIoctl, /* Ioctl() */
  CommonOkResultFunction, /* Read() */
  CommonOkResultFunction /* Write() */
};

SIMPLE_FILE audioFile = {
  __MASK_PRESENT|__MASK_OPEN|__MASK_WRITABLE|__MASK_READABLE|__MASK_FILE, /* Flags */
  Identify, /* Identify() */
  &audioFileOps,
  0, /* pos */
  0, /* ungetc_buffer */
  NULL, /* dev */
};
