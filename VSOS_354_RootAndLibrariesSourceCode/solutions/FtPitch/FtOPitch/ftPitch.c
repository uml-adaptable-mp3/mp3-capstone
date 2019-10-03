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
#include <speedshift.h>
#include "ftPitch.h"

extern FILE *audio_orig;

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

/* These defines are missing from VSOS releases prior to 3.42 */
#ifndef IOCTL_AUDIO_GET_PITCH
#define IOCTL_AUDIO_GET_PITCH		270
#define IOCTL_AUDIO_SET_PITCH		271
#define IOCTL_AUDIO_GET_SPEED		272
#define IOCTL_AUDIO_SET_SPEED		273
#endif /* !IOCTL_AUDIO_GET_SPEED */

#define TMP_BUF_SAMPLES 128
u_int16 tmpAudioBuf[TMP_BUF_SAMPLES];

u_int32 sampleRate = 48000UL;
s_int16 bits = 16;
double currentSpeed = 1.0, currentPitch = 1.0;

IOCTL_RESULT AudioIoctl(register __i0 VO_FILE *self, s_int16 request,
			IOCTL_ARGUMENT argp) {
#if 0
  fprintf(stderr, "###0 Ioctl(%p, %d, 0x%p)\n", self, request, argp);
#endif

  switch (request) {
  case IOCTL_RESTART:
    if ((s_int16)argp != -1) {
      ioctl(audio_orig, IOCTL_RESTART, argp);
    }
    if ((bits = ioctl(audio_orig, IOCTL_AUDIO_GET_BITS, NULL)) < 0) {
      bits = 16;
    }

    return S_OK;
    break;

#if defined(USE_STDAUDIOIN)
  case IOCTL_AUDIO_SET_IRATE:
#endif
#if defined(USE_STDAUDIOOUT)
  case IOCTL_AUDIO_SET_ORATE:
#endif
    sampleRate = *((u_int32 *)argp);
    return SetSampleRate();
    break;

  case IOCTL_AUDIO_SET_RATE_AND_BITS:
    {
      s_int32 rateAndBits = *((u_int32 *)argp);
      bits = (rateAndBits < 0) ? 32 : 16;
      sampleRate = labs(rateAndBits);
      return SetSampleRate();
    }
    break;

  case IOCTL_AUDIO_SET_BITS:
    bits = (u_int16)argp;
    break;

  case IOCTL_AUDIO_GET_SPEED:
    return (u_int16)(currentSpeed*16384.0 + 0.5);
    break;

  case IOCTL_AUDIO_SET_SPEED:
    currentSpeed = (u_int16)argp * (1.0/16384.0);
    {
      double speedPerShift = currentSpeed/currentPitch;
      if (speedPerShift < 0.68) {
	currentSpeed = 0.68*currentPitch;
	speedPerShift = 0.68;
      } else if (speedPerShift > 1.64) {
	currentSpeed = 1.64*currentPitch;
	speedPerShift = 1.64;
      }
      SpeedShiftSet(speedPerShift);
    }
    return S_OK;
    break;

  case IOCTL_AUDIO_GET_PITCH:
    return (u_int16)(currentPitch*16384.0 + 0.5);
    break;

  case IOCTL_AUDIO_SET_PITCH:
    currentPitch = (u_int16)argp * (1.0/16384.0);
    {
      double speedPerShift = currentSpeed/currentPitch;
      if (speedPerShift < 0.68) {
	currentPitch = currentSpeed/0.68;
	speedPerShift = 0.68;
      } else if (speedPerShift > 1.64) {
	currentPitch = currentSpeed/1.64;
	speedPerShift = 1.64;
      }
      SpeedShiftSet(speedPerShift);
    }

    SpeedShiftSet(currentSpeed/currentPitch);
    return SetSampleRate();
    break;
  }

  return ioctl(audio_orig, request, argp);
}


#ifdef USE_STDAUDIOIN
u_int16 AudioRead(register __i0 VO_FILE *self, u_int16 *buf,
		  u_int16 destinationIndex, u_int16 bytes) {
  u_int16 left = bytes>>1;

#if 0
  /* Removed, saving > 20 instruction words */
  /* Only whole samples are allowed. This is not a character device driver. */
  if (destinationIndex || (bytes&((bits==32) ? 7 : 3))) {
    return 0;
  }
#endif

  return fread(buf, 1, bytes>>1, audio_orig);
}
#endif /* USE_STDAUDIOIN */


#ifdef USE_STDAUDIOOUT
u_int16 AudioWrite(register __i0 VO_FILE *self, u_int16 *buf,
		   u_int16 sourceIndex, u_int16 bytes) {
#if 0
  /* Removed, saving > 20 instruction words */
  /* Only whole samples are allowed. This is not a character device driver. */
  if (sourceIndex || (bytes&((bits==32) ? 7 : 3))) {
    return 0;
  }
#endif

  {
    int wordsPerSample = (bits == 16) ? 2 : 4;
    u_int16 samplesLeft = bytes>>(2+(bits>>5));
    u_int16 samplesToWrite;

    while (samplesToWrite = MIN(samplesLeft, TMP_BUF_SAMPLES)) {
      s_int16 ch;

      if (bits == 16) {
	SpeedShift(buf, samplesToWrite);
      } else {
	ringcpy(buf, 1, buf+1, 2, samplesToWrite*2);
	SpeedShift(buf, samplesToWrite);
      }
      samplesLeft -= samplesToWrite;
      buf += wordsPerSample*samplesToWrite;
    }
    return bytes;
  }
}
#endif /* USE_STDAUDIOOUT */

FILEOPS audioFileOps = {
  NULL, /* Open() */
  NULL, /* Close() */
  AudioIoctl, /* Ioctl() */
#ifdef USE_STDAUDIOIN
  AudioRead,  /* Read()  (if this is in the input path)  */
#else
  CommonOkResultFunction,
#endif
#ifdef USE_STDAUDIOOUT
  AudioWrite,  /* Write() (if this is in the output path) */
#else
  CommonOkResultFunction,
#endif
};

SIMPLE_FILE audioFile = {
#ifdef USE_STDAUDIOOUT
  __MASK_PRESENT|__MASK_OPEN|__MASK_WRITABLE|__MASK_FILE, /* Flags */
#else
  __MASK_PRESENT|__MASK_OPEN|__MASK_READABLE|__MASK_FILE, /* Flags */
#endif
  NULL, /* Identify() */
  &audioFileOps,
  0, /* pos */
  0, /* ungetc_buffer */
  NULL, /* dev */
};


auto ioresult SetSampleRate(void) {
  s_int32 rate = (s_int32)(sampleRate * currentPitch + 0.5);
  return ioctl(audio_orig, IOCTL_AUDIO_SET_ORATE, &rate);
}


auto s_int16 AudioOutPart2(s_int16 *p, s_int16 n) {
  if (n != 32) {
#if 0
    printf("E: FTPITCH samples=%d (must be 32)\n", p, n);
#endif
    return 0;
  }
  if (bits == 16) {
    fwrite(p, 2, n, audio_orig);
  } else {
    ringcpy(tmpAudioBuf+1, 2, p, 1, 2*n);
    fwrite(tmpAudioBuf, 4, n, audio_orig);
  }
  return n;
}
