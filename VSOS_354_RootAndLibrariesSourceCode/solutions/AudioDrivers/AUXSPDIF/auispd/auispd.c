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
#include <apploader.h>
#include <clockspeed.h>
#include <imem.h>
#include "auxspd.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

void SPDInterruptVector(void);

struct AudioIn audioIn = {
  NULL
};

struct AuiSpdRegisters auiSpdRegisters[2];

void AuiSpdCyclicFunc(register struct CyclycNode *cyclicNode) {
  static u_int32 lastSampleCounter = 0;
  static u_int16 phase = -1;
  u_int32 newSampleCounter = audioIn.sampleCounter;
  u_int32 newSamples = newSampleCounter-lastSampleCounter;

#if 0
  printf("phase = %d, sc %ld - lsc %ld = %ld\n",
	 phase, newSampleCounter, lastSampleCounter, newSamples);
#endif

  if (newSamples < 20000/10) {
    /* Switch between 56.448 / 61.44 MHz for 44100 / 48000 kHz reception */
    struct AuiSpdRegisters *r;
    static const u_int32 speed[2] = {56448000, 61440000};
    phase = (phase + 1) & 3;
    if (phase & 2) {
      PERIP(SP_RX_CLKDIV) = 4*5/2; /* 88.2 & 96 kHz */
    } else {
      PERIP(SP_RX_CLKDIV) = 4*5;   /* 44.1 & 48 kHz */
    }
    r = auiSpdRegisters+(phase&1);
    clockSpeed.cpuClkHz = speed[phase&1];
    PERIP32(FMCCF_LO) = r->fmCcf;
    PERIP32(ANA_CF3) = PERIP32(ANA_CF3) &
      ~(ANA_CF3_DIV_MASK|ANA_CF3_FMDIV_MASK|ANA_CF3_2GCNTR_MASK) | r->anaCf3;
  }

  lastSampleCounter = newSampleCounter;
}

auto u_int16 AudioInFill(void) {
  s_int16 fi = audioIn.wr - audioIn.rd;
  if (fi < 0)
    fi += audioIn.bufSize;
  return fi & ~3;
}

auto ioresult AudioSetHiBits(register u_int16 hiBits) {
  Disable();
  audioIn.wr = audioIn.rd = audioIn.buf;
  audioIn.flags &= ~AIN_32BIT;
  if (hiBits) {
    audioIn.flags |= AIN_32BIT;
  }
  Enable();

  return 0;
}



ioresult AudioClose(void) {
  PERIP(SP_RX_CF) = 0;
  PERIP(INT_ENABLE1_HP) &= ~INTF1_SRX;

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
    PERIP(SP_RX_CF) = 0;
    PERIP(INT_ENABLE1_LP) &= ~INTF1_SRX;
    PERIP(INT_ENABLE1_HP) &= ~INTF1_SRX;
    /* Set to Perip Mode  SPDIF_IN */
    PERIP(GPIO0_MODE) |= (1<<12);
    WriteIMem(0x20 + INTV_SRX, ReadIMem((u_int16)SPDInterruptVector));  
    AudioSetHiBits(0);
    PERIP(SP_RX_CLKDIV) = 4*5; /* 48 kHz: 5x clock * (12.288MHz/)4. */
    PERIP(SP_RX_CF) = SP_RX_CF_EN|SP_RX_CF_INT_ENA;
    if (AllocateAudioBuffer(&audioIn, 512)) {
      res = S_ERROR;
    }
    break;

  case IOCTL_AUDIO_GET_BITS:
    return (audioIn.flags & (AIN_32BIT)) ? 32 : 16;
    break;

  case IOCTL_AUDIO_SET_BITS:
    res = AudioSetHiBits((u_int16)argp > 16);
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

  default:
    res = S_ERROR;
    break;
  }

  if (audioIn.bufSize) {
    audioFile.flags = __MASK_PRESENT | __MASK_OPEN | __MASK_CHARACTER_DEVICE |
      __MASK_FILE |
      __MASK_READABLE | __MASK_WRITABLE;
    PERIP(INT_ENABLE1_HP) |= INTF1_SRX;
  } else {
    audioFile.flags = __MASK_PRESENT | __MASK_OPEN | __MASK_CHARACTER_DEVICE |
      __MASK_FILE;
    PERIP(INT_ENABLE1_HP) &= ~INTF1_SRX;
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
  strncpy(buf, "AUISPD", bufSize);
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
