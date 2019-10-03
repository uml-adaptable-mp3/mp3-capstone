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
#include <aucommon.h>
#include <agc.h>
#include "ftAgc32.h"
#include "copy.h"

extern FILE *audio_orig;

#ifdef USE_AGC
struct Agc32 agc32 = {
   3,     /* Attack/ms */
  40,     /* Decay/ms */
  -6,     /* targetLevel/dB */
  12,     /* maxGain/dB */
   0,     /* minGain/dB */
   48000, /* sampleRate/Hz */
   /* Rest of the field not for the user */
};
#else
s_int32 sampleRate = 48000;
#endif
struct DcBlock32 dcBlock[2];
u_int16 dcbFlags = DC_BLOCK_AUTO | 8;


#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define SAFETY_BITS 3

#define MAX_FILTERS 16

#define TMP_BUF_SAMPLES 32

s_int32 tmpAudioBuf[2][TMP_BUF_SAMPLES];
s_int16 bits = 16;


void AudioSetRate(void) {
  s_int16 baseShift = 12; /* DC_BLOCK_HIFI */
  s_int32 __fract compareRate = (s_int32 __fract)160000;
  u_int16 __fract step = 0.5; /* 0.5 */
#ifdef USE_AGC
  SetAgc32(&agc32);
#endif
#if 0
  printf("sampleRate = %6ld, compareRate %6ld, DCBFlags = %04x ->",
	 agc32.sampleRate, compareRate, dcbFlags);
#endif
  if ((dcbFlags & 0xC000) == DC_BLOCK_AUTO) {
    step = 0.77; /* 1/sqrt(2) */
    baseShift = 15;
  } else if ((dcbFlags & 0xC000) == DC_BLOCK_SPEECH) {
    baseShift = 8;
  } else if ((dcbFlags & 0xC000) == DC_BLOCK_FORCE) {
    /* Forced; we will not change it. */
#if 0
    printf("no-op %04x\n", dcbFlags);
#endif
    return;
  }
  while ((s_int32)compareRate >
#ifdef USE_AGC
	 agc32.sampleRate
#else
	 sampleRate
#endif
	 ) {
    compareRate *= step;
    baseShift--;
    //    printf("#%ld,%d#", compareRate, baseShift);
  }
  if (baseShift <= 0) {
    baseShift = 1;
  } else if (baseShift > 15) {
    baseShift = 15;
  }
  dcbFlags = (dcbFlags & ~0xF) | baseShift;
#if 0
  printf("%04x\n", dcbFlags);
#endif
}

IOCTL_RESULT AudioIoctl(register __i0 VO_FILE *self, s_int16 request,
			IOCTL_ARGUMENT argp) {
#if 0
  {
    int i, j;
    for (i=0; i<4; i++) {
      u_int32 rates[] = {8000, 12000, 16000, 24000, 32000, 48000, 96000, 192000};
      for (j=0; j<sizeof(rates)/sizeof(rates[0]); j++) {
	agc32.sampleRate = rates[j];
	dcbFlags = (i<<14) | j+4;
	AudioSetRate();
      }
    }
    while (j++) {
      Delay(1000);
    }
  }
#endif


#if 0
  fprintf(stderr, "Ioctl(%p, %d, 0x%p)\n", self, request, argp);
#endif

  switch (request) {
  case IOCTL_RESTART:
    ioctl(audio_orig, IOCTL_RESTART, argp);
    if ((bits = ioctl(audio_orig, IOCTL_AUDIO_GET_BITS, NULL)) < 0) {
      bits = 16;
    }
#if defined(USE_STDAUDIOIN)
#ifdef USE_AGC
    ioctl(audio_orig, IOCTL_AUDIO_GET_IRATE, &agc32.sampleRate);
#else
    ioctl(audio_orig, IOCTL_AUDIO_GET_IRATE, &sampleRate);
#endif
#endif
#if defined(USE_STDAUDIOOUT)
#ifdef USE_AGC
    ioctl(audio_orig, IOCTL_AUDIO_GET_ORATE, &agc32.sampleRate);
#else
    ioctl(audio_orig, IOCTL_AUDIO_GET_ORATE, &sampleRate);
#endif
#endif
    AudioSetRate();
    return S_OK;
    break;

#if defined(USE_STDAUDIOIN)
  case IOCTL_AUDIO_SET_IRATE:
#endif
#if defined(USE_STDAUDIOOUT)
  case IOCTL_AUDIO_SET_ORATE:
#endif
#ifdef USE_AGC
    agc32.sampleRate = *((u_int32 *)argp);
#else
    sampleRate = *((u_int32 *)argp);
#endif
    AudioSetRate();
    break;

  case IOCTL_AUDIO_SET_RATE_AND_BITS:
    {
      s_int32 rateAndBits = *((u_int32 *)argp);
      bits = (rateAndBits < 0) ? 32 : 16;
#ifdef USE_AGC
      agc32.sampleRate=labs(rateAndBits);
#else
      sampleRate=labs(rateAndBits);
#endif
      AudioSetRate();
    }
    break;

  case IOCTL_AUDIO_SET_BITS:
    bits = (u_int16)argp;
    break;

#ifdef USE_AGC
  case IOCTL_AUDIO_GET_AGC32:
    {
      struct Agc32 *tmpAgc32 = (void *)argp;
      tmpAgc32->attack      = agc32.attack;
      tmpAgc32->decay       = agc32.decay;
      tmpAgc32->targetLevel = agc32.targetLevel;
      tmpAgc32->maxGain     = agc32.maxGain;
      tmpAgc32->minGain     = agc32.minGain;
      tmpAgc32->sampleRate  = agc32.sampleRate;
      return S_OK;
    }
    break;

  case IOCTL_AUDIO_SET_AGC32:
    {
      struct Agc32 *tmpAgc32 = (void *)argp;
      agc32.attack      = tmpAgc32->attack;
      agc32.decay       = tmpAgc32->decay;
      agc32.targetLevel = tmpAgc32->targetLevel;
      agc32.maxGain     = tmpAgc32->maxGain;
      agc32.minGain     = tmpAgc32->minGain;
      SetAgc32(&agc32);
      return S_OK;
    }
    break;
#endif

  case IOCTL_AUDIO_SET_DC_BLOCK:
    dcbFlags = (u_int16)argp;
    AudioSetRate();
    return S_OK;
    break;

  case IOCTL_AUDIO_GET_DC_BLOCK:
    return (void *)dcbFlags;
    break;

  } /* switch() */

  return ioctl(audio_orig, request, argp);
}


#ifdef USE_STDAUDIOIN
u_int16 AudioRead(register __i0 VO_FILE *self, u_int16 *buf,
		  u_int16 destinationIndex, u_int16 bytes) {
  int wordsPerSample=(bits == 16) ? 2 : 4;
  u_int16 samplesLeft = bytes>>(2+(bits>>5));
  u_int16 samplesToRead;
  u_int16 samplesRead = 0;

  while (samplesToRead = MIN(samplesLeft, TMP_BUF_SAMPLES)) {
    s_int16 ch;
    s_int16 *bufPlusCh = (s_int16 *)buf;

    samplesRead += fread(buf, wordsPerSample, samplesToRead, audio_orig);
    //    printf("#1: %9ld %9ld\n", buf[0], buf[1]);

    bufPlusCh = (s_int16 *)buf;
    for (ch=0; ch<2; ch++) {
      if (bits == 16) {
	Copy16iTo32Scale(tmpAudioBuf[ch], (s_int16 *)bufPlusCh, samplesToRead, 0);
	bufPlusCh++;
      } else {
	Copy32iTo32Scale(tmpAudioBuf[ch], (s_int32 *)bufPlusCh, samplesToRead, 0);
	bufPlusCh += 2;
      }
    }


    DcBlock32N(dcBlock+0, dcbFlags & 0xF, tmpAudioBuf[0], samplesToRead);
    DcBlock32N(dcBlock+1, dcbFlags & 0xF, tmpAudioBuf[1], samplesToRead);

#ifdef USE_AGC
    if (agc32.minGain || agc32.maxGain) {
      Agc32(&agc32, tmpAudioBuf[0], tmpAudioBuf[1], samplesToRead);
    }
#endif

    bufPlusCh = (s_int16 *)buf;
    for (ch=0; ch<2; ch++) {
      if (bits == 16) {
	Copy32To16iScale((s_int16 *)bufPlusCh, tmpAudioBuf[ch], samplesToRead, 0,0);
	bufPlusCh++;
      } else {
	Copy32To32iScale((s_int32 *)bufPlusCh, tmpAudioBuf[ch], samplesToRead, 0);
	bufPlusCh += 2;
	}
    }
    samplesLeft -= samplesToRead;
    //      printf("#2: %9ld %9ld\n", buf[0], buf[1]);
    buf += wordsPerSample*samplesToRead;
  }
  return wordsPerSample*samplesRead;
}
#endif /* USE_STDAUDIOIN */


#ifdef USE_STDAUDIOOUT
u_int16 AudioWrite(register __i0 VO_FILE *self, u_int16 *buf,
		   u_int16 sourceIndex, u_int16 bytes) {
  return fwrite(buf, wordsPerSample, samplesToRead, audio_orig);
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
  __MASK_PRESENT|__MASK_OPEN|__MASK_WRITABLE|__MASK_READABLE|__MASK_FILE, /* Flags */
  NULL, /* Identify() */
  &audioFileOps,
  0, /* pos */
  0, /* ungetc_buffer */
  NULL, /* dev */
};
