/**
   \file audiofs.c Audio Filesystem driver for VsOS
   \author Henrik Herranen, VLSI Solution Oy
*/

#include <vo_stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <audio.h>
#include <timers.h>
#include <vsos.h>
#include <sysmemory.h>
#include <ringbuf.h>
#include <aucommon.h>
#include <exec.h>
#include <clockspeed.h>
#include <imem.h>
#include "auodac.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

void DacInterruptVector(void);
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

auto void MySetRate(register __reg_c u_int32 rate, register s_int16 shRight) {
  u_int32 f;

  /* Fractional conversion:
     rate(30:24) -> rate( 6:0)
     rate(23: 0) -> rate(30:7) */
  rate = (((rate & ~0x7f000000)<< 7) | (rate >> 25)) >> shRight;
  
  f = (131072.0*8.0) * rate / clockSpeed.peripClkHz + 0.5;

  PERIP32(DAC_SRCL) = f;
}


auto ioresult AudioSetRateAndBits(register s_int32 sampleRate,
				  register u_int16 hiBits) {
  u_int16 useHighRate = PERIP(DAC_MTEST) & DAC_MTEST_96K;
  u_int32 intSampleRate = sampleRate & ~0x7f000000;

  if (intSampleRate > 97500) {
    sampleRate = 97500;
  }
  audioOut.sampleRate = intSampleRate;
  Forbid();
  if (useHighRate) {
    if (intSampleRate > 95900) {
      MySetRate(sampleRate, 1);
    } else {
      PERIP(DAC_SRCH) = 0;
      PERIP(DAC_SRCL) = 0;
      PERIP(DAC_MTEST) &= ~DAC_MTEST_96K;
      MySetRate(sampleRate, 0);
    }
  } else {
    if (intSampleRate < 96000) {
      MySetRate(sampleRate, 0);
    } else {
      PERIP(DAC_SRCH) = 0;
      PERIP(DAC_SRCL) = 0;
      PERIP(DAC_MTEST) |= DAC_MTEST_96K;
      MySetRate(sampleRate, 1);
    }
  }
  Permit();


  /* If 32-bit status has changed */
  if ((hiBits ? AIN_32BIT : 0) ^ (audioOut.flags & AIN_32BIT)) {
    memsetY(audioOut.buf, 0, audioOut.bufSize);
    Disable();
    audioOut.wr = audioOut.rd = audioOut.buf;
    audioOut.flags ^= AIN_32BIT;
    Enable();
  }

  return 0;
}



ioresult AudioClose(void) {
  PERIP(DAC_LEFT) = 0;
  PERIP(DAC_LEFT_LSB) = 0;
  PERIP(DAC_RIGHT) = 0;
  PERIP(DAC_RIGHT_LSB) = 0;
  PERIP(INT_ENABLE0_HP) &= ~INTF_DAC;

  if (audioOut.bufSize) {
    FreeMemY(audioOut.buf, audioOut.bufSize);
    audioOut.bufSize = NULL;
  }
}


ioresult AllocateAudioBuffer(register struct AudioOut *a,
			     register u_int16 bufSize) {
  ioresult res = S_OK;
#if 1
  Disable();
#endif
#if 0
  printf("#1 bs=%d, a->bs=%d\n", bufSize, a->bufSize); Delay(10);
#endif
  if (a->bufSize) {
    FreeMemY(a->buf, a->bufSize);
  }
  if (bufSize == -1) {
#if 0
    printf("BIG BUFFER\n");
    Delay(10);
#endif
    a->buf = (u_int16 __mem_y *)0x7000;
    a->bufSize = 4096;
    memsetY(a->buf, 0, 4096);
    a->modifier = MAKEMODF(4096);
  } else if (!(a->buf = AllocMemY(bufSize, bufSize))) {
    static u_int16 __mem_y panicBuffer[4];
#if 0
    printf("NORMAL BUFFER FAIL %d\n", bufSize);
    Delay(10);
#endif
    a->buf = panicBuffer;
    a->bufSize = 0;
    a->modifier = MAKEMODF(4);
    res = S_ERROR;
  } else {
#if 0
    printf("NORMAL BUFFER OK %d\n", bufSize);
    Delay(10);
#endif
    memsetY(a->buf, 0, bufSize);
    a->bufSize = bufSize;
    a->modifier = MAKEMODF(bufSize);
  }
  a->wr = a->rd = (__mem_y s_int16 *)(a->buf);
#if 1
  Enable();
#endif
  return res;
}

IOCTL_RESULT AudioIoctl(register __i0 VO_FILE *self, s_int16 request,
			IOCTL_ARGUMENT argp) {
  ioresult res = S_OK;

#if 0
  printf("AUODAC Ioctl(%p, %d, %p)\n", self, request, argp);
#endif

  switch (request) {
#if 0
  case IOCTL_START_FRAME:			
    break;
		
  case IOCTL_END_FRAME:
    break;
#endif

  case IOCTL_RESTART:
    PERIP(INT_ENABLE0_LP) &= ~INTF_DAC;
    PERIP(INT_ENABLE0_HP) &= ~INTF_DAC;
    PERIP(ANA_CF1) |= ANA_CF1_DRV_ENA;
    WriteIMem(0x20 + INTV_DAC, ReadIMem((u_int16)DacInterruptVector));  
    AudioSetRateAndBits(48000L, 0);
    if (AllocateAudioBuffer(&audioOut, ((s_int16)argp == -1) ? -1 : 512)) {
      res = S_ERROR;
    }
    break;

  case IOCTL_AUDIO_GET_ORATE:
    *((u_int32 *)argp) = audioOut.sampleRate;
    break;

  case IOCTL_AUDIO_SET_ORATE:
    res = AudioSetRateAndBits(*((u_int32 *)argp), audioOut.flags & AIN_32BIT);
    break;

  case IOCTL_AUDIO_SET_RATE_AND_BITS:
    {
      s_int32 rate = *((u_int32 *)argp);
      s_int16 hiBits = (rate < 0);
      rate = labs(rate);
      res = AudioSetRateAndBits(rate, hiBits);
    }
    break;

  case IOCTL_AUDIO_GET_BITS:
    return (audioOut.flags & AIN_32BIT) ? 32 : 16;
    break;

  case IOCTL_AUDIO_SET_BITS:
    res = AudioSetRateAndBits(audioOut.sampleRate, (u_int16)argp > 16);
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

  case IOCTL_AUDIO_GET_VOLUME:
    return (volumeReg & 0xFF) + 256;
    break;

  case IOCTL_AUDIO_SET_VOLUME:
    {
      s_int16 t = (s_int16)argp - 256;
      if (t < 0) {
	t = 0;
      } else if (t > 255) {
	t = 255;
      }
      volumeReg = t * 0x0101;
      SetVolume();
    }
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
      /*__MASK_READABLE |*/ __MASK_WRITABLE;
    PERIP(INT_ENABLE0_HP) |= INTF_DAC;
  } else {
    audioFile.flags = __MASK_PRESENT | __MASK_OPEN | __MASK_CHARACTER_DEVICE |
      __MASK_FILE;
    PERIP(INT_ENABLE0_HP) &= ~INTF_DAC;
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
  strncpy(buf, "AUODAC", bufSize);
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
  __MASK_PRESENT|__MASK_OPEN|__MASK_WRITABLE|/*__MASK_READABLE|*/__MASK_FILE, /* Flags */
  Identify, /* Identify() */
  &audioFileOps,
  0, /* pos */
  0, /* ungetc_buffer */
  NULL, /* dev */
};
