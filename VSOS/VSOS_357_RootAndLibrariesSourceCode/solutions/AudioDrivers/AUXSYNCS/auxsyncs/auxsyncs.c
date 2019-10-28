/**
   \file auxsyncs.c Audio slave input to output synchronization driver for VSOS
   \author Henrik Herranen, VLSI Solution Oy
*/

#include <vo_stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <vstypes.h>
#include <vs1005h.h>
#include <timers.h>
#include <vsos.h>
#include <audio.h>
#include <exec.h>
#include "auxsyncs.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#if 0
#define DEBUG_PRINT
#endif

__mem_y double calculatedSampleRate = 0.0;
__mem_y int glueToStandardRate = 3;
__mem_y u_int32 lastTimeCount = 0;
__mem_y u_int32 lastSamples = 0;
u_int32 oRate = 0;
double bufferMultiplier = 1.0;



#if 0
ioresult AudioClose(void) {
}
#endif


IOCTL_RESULT AudioIoctl(register __i0 VO_FILE *self, s_int16 request,
			IOCTL_ARGUMENT argp) {
  ioresult res = S_OK;

#if 0
  fprintf(stderr, "Ioctl(%p, %d, %p)\n", self, request, argp);
#endif

  switch (request) {
  case IOCTL_RESTART:
    /* Force recalculation of sample rate */
    calculatedSampleRate = 0.0;
    break;

  case IOCTL_AUDIO_GET_IRATE:
    {
      u_int32 newTimeCount = ReadTimeCount();
      /* If not calculated in a while, or at all, force recalculation. */
      if (newTimeCount - lastTimeCount > TICKS_PER_SEC/5) {
	calculatedSampleRate = 0.0;
      }
      while ((!calculatedSampleRate || glueToStandardRate) &&
	     ReadTimeCount()-newTimeCount < TICKS_PER_SEC) {
	AudioWrite(stdaudioin, NULL, 0, 0);
      }
    }
    *((u_int32 *)argp) = (u_int32)(calculatedSampleRate + 0.5);
    break;

  default:
    res = stdaudioin_ioctl_orig(stdaudioin, request, argp);
    break;
  }

  return res;
}




struct StandardRates {
  u_int32 sampleRate;
  u_int16 maxError;
};

struct StandardRates standardRates[] = {
  {  8000,   160},
  { 11025,   220},
  { 12000,   240},
  { 16000,   320},
  { 22050,   441},
  { 24000,   480},
  { 32000,   640},
  { 44100,   882},
  { 48000,   960},
  { 88200,  1764},
  { 96000,  1920},
  {176400,  3528},
  {192000,  3840},
  {     0,     0}
};


u_int16 AudioWrite(register __i0 VO_FILE *self, void *buf,
		   u_int16 sourceIndex, u_int16 bytes) {
  u_int16 res = bytes ? 
    stdaudioout_write_orig(stdaudioout, buf, sourceIndex, bytes) : 0;

  if (ReadTimeCount() - lastTimeCount > TICKS_PER_SEC/10) {
    u_int32 currTime;
    u_int32 currSamples;
    double newRate = 0.0, ratio = 0.0;
    s_int32 intRate;
    s_int16 inBufFill;
    s_int16 outBufFree;
    s_int16 outBufSize;
    s_int16 totFill;

    Forbid();
    currTime = ReadTimeCount();
    ioctl(stdaudioin, IOCTL_AUDIO_GET_SAMPLE_COUNTER, (void *)(&currSamples));
    inBufFill = ioctl(stdaudioin, IOCTL_AUDIO_GET_INPUT_BUFFER_FILL, NULL);
    outBufFree = ioctl(stdaudioout, IOCTL_AUDIO_GET_OUTPUT_BUFFER_FREE, NULL);
    outBufSize = ioctl(stdaudioout, IOCTL_AUDIO_GET_OUTPUT_BUFFER_SIZE, NULL);
    totFill = inBufFill+outBufSize-outBufFree;
    Permit();

    /* Only calculate if less than one second since last calculation */
    if (currTime - lastTimeCount < TICKS_PER_SEC) {
      newRate = (u_int32)((currSamples-lastSamples)*TICKS_PER_SEC/
			  (double)(currTime-lastTimeCount)+0.5);
    }
    if (newRate && calculatedSampleRate) {
      ratio = newRate / calculatedSampleRate;
    }
    if (ratio < 0.98 || ratio > 1.02 || glueToStandardRate > 1) {
      calculatedSampleRate = newRate;
    } else {
      calculatedSampleRate = calculatedSampleRate*0.995 +
	newRate*0.005;
    }
    if (calculatedSampleRate) {
      if (glueToStandardRate) {
	int i;
	struct StandardRates *sr = standardRates;
	glueToStandardRate--;
	while (sr->maxError) {
	  if (labs((s_int32)calculatedSampleRate - sr->sampleRate) < sr->maxError) {
	      calculatedSampleRate = sr->sampleRate;
	  }
	  sr++;
	}
      }
    } else {
      glueToStandardRate = 3;
    }
    {
      s_int16 tippingPoint = outBufSize*3/4;
      bufferMultiplier = pow(2.0, (double)(totFill-tippingPoint)/(outBufSize*100.0));
    }
#ifdef DEBUG_PRINT
    printf("%ld in %ld: %7.2f->%7.2f (ratio %6.4f, glue %d), bm %6.4f, inFi %4d : outFr %4d -> %4d ->",
	   currSamples-lastSamples,
	   currTime-lastTimeCount,
	   newRate, calculatedSampleRate,
	   ratio, glueToStandardRate,
	   bufferMultiplier,
	   inBufFill, outBufFree, totFill);
#endif
    lastTimeCount = currTime;
    lastSamples = currSamples;
    /* Calculate with 1/128 Hz accuracy. */
    intRate = (s_int32)(calculatedSampleRate*bufferMultiplier*128.0+0.5);
    /* Fractional conversion:
       rate( 6:0) -> rate(30:24)
       rate(30:7) -> rate(23: 0) */
    intRate = (intRate >> 7) | ((intRate & 127) << 24);
#ifdef DEBUG_PRINT
    printf("#%08lx#", intRate);
#endif
    if (intRate > 1000L*128 && intRate != oRate) {
      oRate = intRate;
      ioctl(stdaudioout, IOCTL_AUDIO_SET_ORATE, (char *)(&oRate));
#ifdef DEBUG_PRINT
      printf("* %04x:%04x", PERIP(DAC_SRCH), PERIP(DAC_SRCL));
#endif
    }
#ifdef DEBUG_PRINT
    printf("\n");
#endif
  }

  return res;
}

char *Identify(register __i0 void *self, char *buf, u_int16 bufSize) {
  strncpy(buf, "AUXSYNCS", bufSize);
  buf[bufSize-1] = '\0';
  return buf;
}

