/**
   \file ftRev.c Reverb driver for VSOS
   \author Henrik Herranen, VLSI Solution Oy
*/

#include <vo_stdio.h>
#include <string.h>
#include <stdlib.h>
#include <apploader.h>
#include <ctype.h>
#include <vstypes.h>
#include <vs1005h.h>
#include <timers.h>
#include <vsos.h>
#include <sysmemory.h>
#include <ringbuf.h>
#include <aucommon.h>
#include "nrFilt.h"

extern FILE *audio_orig;

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

/* These defines are missing from VSOS releases prior to 3.60 */
#ifndef IOCTL_AUDIO_GET_NOISE_KILLER
#define IOCTL_AUDIO_GET_NOISE_KILLER	290
#define IOCTL_AUDIO_SET_NOISE_KILLER	291
#endif /* !IOCTL_AUDIO_GET_NOISE_KILLER */


auto ioresult SetSampleRate(void);

u_int16 tmpBuf[2*NOISE_KILLER_HANDLE_SAMPLES];
u_int16 tmpBufSamples = 0;

u_int32 sampleRate = 48000UL;
s_int16 bits = 16;


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
    SetSampleRate();
    break;

  case IOCTL_AUDIO_SET_RATE_AND_BITS:
    {
      s_int32 rateAndBits = *((u_int32 *)argp);
      bits = (rateAndBits < 0) ? 32 : 16;
      sampleRate = labs(rateAndBits);
      SetSampleRate();
    }
    break;

  case IOCTL_AUDIO_SET_BITS:
    bits = (u_int16)argp;
    break;

  case IOCTL_AUDIO_GET_NOISE_KILLER:
    return GetNoiseKiller();
    break;

  case IOCTL_AUDIO_SET_NOISE_KILLER:
    SetNoiseKiller((s_int16)argp);
    return S_OK;
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

  if (bits != 16) return fread(buf, 1, bytes>>1, audio_orig);

  while (left) {
    s_int16 toHandle = MIN(2*NOISE_KILLER_HANDLE_SAMPLES-tmpBufSamples, left);
    memcpy(buf, tmpBuf+tmpBufSamples, toHandle);
    buf += toHandle;
    left -= toHandle;
    if ((tmpBufSamples += toHandle) >= 2*NOISE_KILLER_HANDLE_SAMPLES) {
      tmpBufSamples = 0;
      fread(tmpBuf, 1, 2*NOISE_KILLER_HANDLE_SAMPLES, audio_orig);
      RunFmNoiseKiller(tmpBuf, NOISE_KILLER_HANDLE_SAMPLES);
    }
  }
}
#endif /* USE_STDAUDIOIN */


#ifdef USE_STDAUDIOOUT
u_int16 AudioWrite(register __i0 VO_FILE *self, u_int16 *data,
		   u_int16 sourceIndex, u_int16 bytes) {
#if 0
  /* Removed, saving > 20 instruction words */
  /* Only whole samples are allowed. This is not a character device driver. */
  if (sourceIndex || (bytes&((bits==32) ? 7 : 3))) {
    return 0;
  }
#endif

  return bytes;
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
  return S_OK;
}
