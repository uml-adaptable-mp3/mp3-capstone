/**
   \file auiadc.c ADC driver for VSOS
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
#include <imem.h>
#include "auiadc.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

void AdcInterruptVector(void);
void Dec6InterruptVector(void);

extern u_int32 auxi;

struct AudioIn audioIn = {
  NULL
};

auto u_int16 AudioInFill(void) {
  s_int16 fi = audioIn.wr - audioIn.rd;
  if (fi < 0)
    fi += audioIn.bufSize;
  return fi & ~3;
}


auto ioresult AudioSetRate(register s_int32 sampleRate,
			   register u_int16 hiBits);


auto void AudioClose(void) {
  AudioCloseInput();

  if (audioIn.bufSize) {
    FreeMemY(audioIn.buf, audioIn.bufSize);
    audioIn.bufSize = 0;
  }
}


ioresult AllocateAudioBuffer(register struct AudioIn *a,
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
    if (AllocateAudioBuffer(&audioIn, 512)) {
      res = S_ERROR;
    } else {
#if 1
      AudioSetInput(AID_LINE1_1|AID_LINE1_3);
#else
      AudioSetInput(AID_MIC1|AID_MIC2);
#endif
    }
    WriteIMem((0x20 + INTV_MAC0), ReadIMem((u_int16)AdcInterruptVector));  
    WriteIMem((0x20 + INTV_MAC1), ReadIMem((u_int16)Dec6InterruptVector));  
    break;

  case IOCTL_AUDIO_GET_IRATE:
    *((u_int32 *)argp) = audioIn.sampleRate;
    break;

  case IOCTL_AUDIO_SET_IRATE:
    res = AudioSetRate(*((u_int32 *)argp), audioIn.flags & AIN_32BIT);
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
    return (audioIn.flags & (AIN_32BIT)) ? 32 : 16;
    break;

  case IOCTL_AUDIO_SET_BITS:
    res = AudioSetRate(audioIn.sampleRate, (u_int16)argp > 16);
    break;

  case IOCTL_AUDIO_GET_INPUT_BUFFER_SIZE:
    return audioIn.bufSize;
    break;

  case IOCTL_AUDIO_SET_INPUT_BUFFER_SIZE:
    if (AllocateAudioBuffer(&audioIn, (u_int16)argp)) {
      res = S_ERROR;
    }
    break;

  case IOCTL_AUDIO_GET_INPUT_BUFFER_FILL:
    return AudioInFill();
    break;

  case IOCTL_AUDIO_GET_SAMPLE_COUNTER:
    Disable();
    *((u_int32 *)argp) = audioIn.sampleCounter;
    Enable();
    break;

  case IOCTL_AUDIO_GET_OVERFLOWS:
    Disable();
    *((u_int32 *)argp) = audioIn.overflow;
    Enable();
    break;

  case IOCTL_AUDIO_SELECT_INPUT:
    res = AudioSetInput((u_int16)argp);
    break;

  default:
    res = S_ERROR;
    break;
  }

  if (audioIn.bufSize) {
    audioFile.flags = __MASK_PRESENT | __MASK_OPEN | __MASK_CHARACTER_DEVICE |
      __MASK_FILE |
      __MASK_READABLE | __MASK_WRITABLE;

    Disable();
    {
      u_int16 intAddr =
	(adInts[adc.iChannel] >> 4)*(INT_ENABLE1_LP-INT_ENABLE0_LP);
      u_int16 intVec = adInts[adc.iChannel] & 15;
      
      //    printf("Enable int addr %d, vec %d\n", intAddr, intVec);
      PERIP(INT_ENABLE0_LP+intAddr) &= ~(1<<intVec);
      PERIP(INT_ENABLE0_HP+intAddr) |= 1<<intVec;
    }
    Enable();


#if 0
    PERIP(INT_ENABLE0_HP) |= INTF_MAC0;
#endif
  } else {
    audioFile.flags = __MASK_PRESENT | __MASK_OPEN | __MASK_CHARACTER_DEVICE |
      __MASK_FILE;
#if 1
    PERIP(INT_ENABLE0_HP) &= ~(INTF_MAC0|INTF_MAC1);
#endif
  }

  return res;
}



u_int16 AudioRead(register __i0 VO_FILE *self, u_int16 *buf,
		  u_int16 destinationIndex, u_int16 bytes) {
  u_int16 left = bytes>>1;
  /* Only whole samples are allowed. This is not a character device driver. */
  if (destinationIndex || (bytes&((audioIn.flags & AIN_32BIT) ? 7 : 3))) {
    return 0;
  }
  while (left) {
    u_int16 toRead = MIN(left, 16);
    while (AudioInFill() < toRead) {
      Delay(1);
    }
    audioIn.rd =
      RING_YX_S(ringcpyYX(buf, 1, audioIn.rd, audioIn.modifier, toRead));
    buf += toRead;
    left -= toRead;
  }
  return bytes;
}

char *Identify(register __i0 void *self, char *buf, u_int16 bufSize) {
  strncpy(buf, "AUIADC", bufSize);
  buf[bufSize-1] = '\0';
  return buf;
}

FILEOPS audioFileOps = {
  NULL, /* Open() */
  NULL, /* Close() */
  AudioIoctl, /* Ioctl() */
  AudioRead, /* Read() */
  CommonOkResultFunction /* Write() (does not exist, always returns 0) */
};

SIMPLE_FILE audioFile = {
  __MASK_PRESENT|__MASK_OPEN|__MASK_WRITABLE|__MASK_READABLE|__MASK_FILE, /* Flags */
  Identify, /* Identify() */
  &audioFileOps,
  0, /* pos */
  0, /* ungetc_buffer */
  NULL, /* dev */
};
