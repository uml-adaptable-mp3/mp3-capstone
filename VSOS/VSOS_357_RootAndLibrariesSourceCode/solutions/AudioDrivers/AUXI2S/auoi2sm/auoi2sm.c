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

void I2SInterruptVector(void);
extern u_int32 auxi;

struct AudioOut audioOut = {
  NULL
};

auto u_int16 AudioOutFree(void) {
  s_int16 fi = audioOut.wr - audioOut.rd;
  s_int16 fr;
  if (fi < 0)
    fi += audioOut.bufSize;
  fr = audioOut.bufSize - fi - 1;
  if (fr < 0)
    return 0;
  return fr & ~3;
}

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
    if (!ic->sampleRateAndHiBits) {
      return S_ERROR;
    }
    ic++;
  }

  memsetY(audioOut.buf, 0, audioOut.bufSize);
  Disable();
  PERIP(I2S_CF) = 0;
  audioOut.wr = audioOut.rd = audioOut.buf;
  audioOut.sampleRate = sampleRate;
  audioOut.flags &= ~AIN_32BIT;
  if (hiBits) {
    audioOut.flags |= AIN_32BIT;
  }
  PERIP(I2S_CF) = ic->pattern | (I2S_CF_MASTER | I2S_CF_ENAMCK |
				 I2S_CF_ENA | I2S_CF_MODE | I2S_CF_INTENA);
  Enable();

  return 0;
}



ioresult AudioClose(void) {
  PERIP(I2S_CF) = 0;
  PERIP(INT_ENABLE0_HP) &= ~INTF_I2S;

  if (audioOut.bufSize) {
    FreeMemY(audioOut.buf, audioOut.bufSize);
    audioOut.bufSize = NULL;
  }
}


ioresult AllocateAudioBuffer(register struct AudioOut *a,
			     register u_int16 bufSize) {
  ioresult res = S_OK;
  Disable();
  if (a->bufSize) {
    FreeMemY(a->buf, a->bufSize);
  }
  if (!(a->buf = AllocMemY(bufSize, bufSize))) {
    static u_int16 __mem_y panicBuffer[4];
    a->buf = panicBuffer;
    a->bufSize = 0;
    a->modifier = MAKEMODF(4);
    res = S_ERROR;
  } else {
    memsetY(a->buf, 0, bufSize);
    a->bufSize = bufSize;
    a->modifier = MAKEMODF(bufSize);
  }
  a->wr = a->rd = (__mem_y s_int16 *)(a->buf);
  Enable();
  return res;
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
    PERIP(INT_ENABLE0_LP) &= ~INTF_I2S;
    PERIP(INT_ENABLE0_HP) &= ~INTF_I2S;
    /* Set to Perip Mode  I2S_DI    I2S_DO    I2S_BCK   I2S_FRM   I2S_12M */
    PERIP(GPIO1_MODE) |= ((1<<10) | (1<<11) | (1<<12) | (1<<13) | (1<<14));
    WriteIMem((u_int16 *)(0x20 + INTV_I2S), ReadIMem(I2SInterruptVector));  
    AudioSetRate(48000, 0);
    if (AllocateAudioBuffer(&audioOut, 512)) {
      res = S_ERROR;
    }
    break;

  case IOCTL_AUDIO_GET_ORATE:
    *((u_int32 *)argp) = audioOut.sampleRate>>1;
    break;

  case IOCTL_AUDIO_SET_ORATE:
    res = AudioSetRate(*((u_int32 *)argp), audioOut.flags & AIN_32BIT);
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
    return (audioOut.flags & (AIN_32BIT)) ? 32 : 16;
    break;

  case IOCTL_AUDIO_SET_BITS:
    res = AudioSetRate(audioOut.sampleRate>>1, (u_int16)argp > 16);
    break;

  case IOCTL_AUDIO_GET_OUTPUT_BUFFER_SIZE:
    return audioOut.bufSize;
    break;

  case IOCTL_AUDIO_SET_OUTPUT_BUFFER_SIZE:
    if (AllocateAudioBuffer(&audioOut, (u_int16)argp)) {
      res = S_ERROR;
    }
    break;

  case IOCTL_AUDIO_GET_OUTPUT_BUFFER_FREE:
    return AudioOutFree();
    break;

  case IOCTL_AUDIO_GET_SAMPLE_COUNTER:
    Disable();
    *((u_int32 *)argp) = audioOut.sampleCounter;
    Enable();
    break;

  case IOCTL_AUDIO_GET_UNDERFLOWS:
    Disable();
    *((u_int32 *)argp) = audioOut.underflow;
    Enable();
    break;

  default:
    res = S_ERROR;
    break;
  }

  if (audioOut.bufSize) {
    audioFile.flags = __MASK_PRESENT | __MASK_OPEN | __MASK_CHARACTER_DEVICE |
      __MASK_FILE |
      __MASK_READABLE | __MASK_WRITABLE;
    PERIP(INT_ENABLE0_HP) |= INTF_I2S;
  } else {
    audioFile.flags = __MASK_PRESENT | __MASK_OPEN | __MASK_CHARACTER_DEVICE |
      __MASK_FILE;
    PERIP(INT_ENABLE0_HP) &= ~INTF_I2S;
  }

  return res;
}



u_int16 AudioWrite(register __i0 VO_FILE *self, u_int16 *buf,
		   u_int16 sourceIndex, u_int16 bytes) {
  u_int16 left = bytes>>1;
  if (sourceIndex || (bytes&((audioOut.flags & AIN_32BIT) ? 7 : 3))) {
    return 0;
  }
  while (left) {
    u_int16 toWrite = MIN(left, 16);
    while (AudioOutFree() < toWrite) {
      Delay(1);
    }
    audioOut.wr =
      RING_XY_D(ringcpyXY(audioOut.wr, audioOut.modifier, buf, 1, toWrite));
    buf += toWrite;
    left -= toWrite;
  }
  return bytes;
}

char *Identify(register __i0 void *self, char *buf, u_int16 bufSize) {
  strncpy(buf, "AUOI2SM", bufSize);
  buf[bufSize-1] = '\0';
  return buf;
}

FILEOPS audioFileOps = {
  NULL, /* Open() */
  NULL, /* Close() */
  AudioIoctl, /* Ioctl() */
  CommonOkResultFunction, /* Read() */
  AudioWrite /* Write() */
};

SIMPLE_FILE audioFile = {
  __MASK_PRESENT|__MASK_OPEN|__MASK_WRITABLE|__MASK_READABLE|__MASK_FILE, /* Flags */
  Identify, /* Identify() */
  &audioFileOps,
  0, /* pos */
  0, /* ungetc_buffer */
  NULL, /* dev */
};
