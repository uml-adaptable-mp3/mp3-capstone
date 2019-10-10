/**
   \file ftRev23.c Reverb driver for VSOS that uses VS23S0x0 SRAM for buffers
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
#include <fir.h>
#include <ringbuf.h>
#include <saturate.h>
#include <limits.h>
#include <mutex.h>
#include "reverb_int.h"
#include "ftRev.h"
#include "vs23s0x0_misc.h"
#include "downup3.h"
#include "interleave.h"
#include "miscAsm.h"

extern FILE *audio_orig;

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

/* These defines are missing from VSOS releases prior to 3.55 */
#ifndef IOCTL_AUDIO_GET_REV_ROOM
#define IOCTL_AUDIO_GET_REV_ROOM	280
#define IOCTL_AUDIO_SET_REV_ROOM	281

struct Room {
  /* These fields filled in by the caller of DesignRoom() */
  u_int16 firstReflectionMS; /* How many milliseconds before first reflection */
  u_int16 roomSizeCM;	/* Room size in cm */
  u_int16 reverbTimeMS;	/* Room reverb time in ms = -60 dB attenuation time */
  u_int16 softness;	/* Room softness, 0 = hard, 65535 = soft */
  u_int32 sampleRate;	/* sample rate used for making room design */
  s_int16 dryGain;	/* 1024 = 1 */
  s_int16 wetGain;	/* 1024 = 1 */
  /* Following fields needed only if an external memory is used for storage */
  s_int16 (*Read )(register       u_int16 *d, register u_int32 addr, register u_int16 words); /* Function to read from external memory */
  s_int16 (*Write)(register const u_int16 *d, register u_int32 addr, register u_int16 words); /* Function to write to external memory */
  u_int32 memSizeWords; /* External memory size */
  /* Rest of the fields filled in by the room designer */
  s_int16 delays;	/* How many delays per channel used */
};

#endif /* !IOCTL_AUDIO_GET_REV_ROOM */


/* NOTE! TMP_BUF_SAMPLES _MUST_ be divisible by both 3 and 4 -> 12! */
#define TMP_BUF_SAMPLES (256/12*12)
s_int16 tmpBuf[2][TMP_BUF_SAMPLES];
u_int16 downsample = 0;


struct RoomReverb23 __mem_y room = {
  20,		/* firstReflectionMS */
  600,		/* roomSizeCM, 10 - 10300, recommended 200-1200 */
  1000,		/* reverbTimeMS, recommended 100-10000 */
  32767,	/* Softness, 0 = hard, 65535 = soft */
  48000,	/* sampleRate */
  1024,		/* dryGain */
  256,		/* wetGain */
  ReadVS23S0x0, /* Read */
  WriteVS23S0x0,/* Write */
  0		/* memSizeWords */
};
s_int16 myDryGain = 1024;


auto ioresult SetSampleRate(void);

u_int32 sampleRate = 48000UL;
s_int16 bits = 16;

u_int16 myMutex[2] = {1,1};

/* First reflection delay pointer */
s_int16 __mem_y *fDelay = NULL, *fDelayP = NULL;
u_int16 fDelayModF = 0, fDelayLen = 0;

int MyDesignRoom23(register struct RoomReverb23 __mem_y *room) {
  int res;
  u_int16 t;
  u_int16 wordsPerMS = (u_int16)(sampleRate*bits/8000);
  res = DesignRoom23(room);
  if (fDelay) {
    FreeMemY(fDelay, fDelayLen);
    fDelay = fDelayP = NULL;
  }
  if (downsample) {
    room->dryGain = myDryGain;
    fDelayLen = 0;
    room->firstReflectionMS = 0;
  } else {
    u_int32 t32 = (u_int32)wordsPerMS*room->firstReflectionMS;

    room->dryGain = 0;
    fDelayLen = (u_int16)(MIN(t32,8192));
    if (fDelayLen < 2*wordsPerMS) fDelayLen = 2*wordsPerMS;
    while (!(fDelay = AllocMemY(fDelayLen, 1<<QsortLog2(fDelayLen-1)))) {
      fDelayLen = (u_int16)(((u_int32)fDelayLen*127)/128);
    }
    room->firstReflectionMS = fDelayLen/wordsPerMS;
    memsetY(fDelay, 0, fDelayLen);
    fDelayP = fDelay;
    fDelayModF = MAKEMODF(fDelayLen);
  }
  return res;
}

#if 0
void CombineDryWet16(register __i0 s_int16 *dry, register __d0 u_int16 dryGain, register __i1 s_int16 *wet, register __d1 u_int16 n) {
  int i;
  for (i=0; i<n; i++) {
    *dry = Sat32To16((s_int32)*wet++ + (((s_int32)*dry * dryGain) >> 10));
    dry++;
  }
}
#endif

#if 0
void CombineDryWet32(register __i0 s_int16 *dry, register __d0 u_int16 dryGain, register __i1 s_int16 *wet, register __d1 u_int16 n) {
  int i;
  for (i=0; i<n; i++) {
    s_int32 resHi;
    u_int16 resLo, ut;
    ut = *dry++;
    resLo = (u_int16)(((u_int32)ut * dryGain) >> 16);
    resHi = ((s_int32)(*dry--) * dryGain);
    resHi += resLo;
    if (resHi >= 1024*32768L) {
      resHi = LONG_MAX;
    } else if (resHi <= -1024*32768L) {
      resHi = LONG_MIN;
    } else {
      resHi <<= 6;
    }
    *dry++ = (s_int16)resHi;
    *dry++ = Sat32To16((resHi>>16) + *wet++);
  }
}
#endif

void CombineDryWet(register s_int16 *dry, register u_int16 dryGain, register s_int16 *wet, register u_int16 stereoSamples, register u_int16 bits) {
  u_int16 n = 2*stereoSamples;
  int i;
  if (bits == 16) {
    CombineDryWet16(dry, dryGain, wet, 2*stereoSamples);
  } else {
    CombineDryWet32(dry, dryGain, wet, 2*stereoSamples);
  }
}


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

  case IOCTL_AUDIO_GET_REV_ROOM:
    {
      struct Room *myRoom = (struct Room *)argp;
      memcpyYX(myRoom, &room, sizeof(struct Room));
      myRoom->dryGain = myDryGain;
      myRoom->sampleRate = sampleRate;
    }
    return 0;
    break;

  case IOCTL_AUDIO_SET_REV_ROOM:
    {
      struct Room *myRoom = (struct Room *)argp;
      memcpyXY(&room, myRoom, sizeof(struct Room));
      myDryGain = room.dryGain;
      if (!downsample) room.dryGain = 0;
      SetSampleRate();
    }
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

  return fread(buf, 1, bytes>>1, audio_orig);
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

  ObtainMutex(myMutex+0);
  while (AttemptMutex(myMutex+1)) {
    ReleaseMutex(myMutex+0);
    Delay(1);
    ObtainMutex(myMutex+0);
  }
  ReleaseMutex(myMutex+1);

  {
    int sampleShift = (bits == 16) ? 1 : 2;
    int wordsPerSample = 1 << sampleShift;
    u_int16 samplesLeft = bytes>>(2+(bits>>5));
    u_int16 samplesToWrite;
    static u_int16 f = 0;

#if 0
    if (!(++f & 2047)) {
      printf("wps %d, byt %d, samp %d\n",
	     wordsPerSample, bytes, samplesToWrite);
    }
#endif

    while (samplesToWrite = MIN(samplesLeft, TMP_BUF_SAMPLES>>sampleShift)) {
      if (downsample) {
	u_int16 t = samplesToWrite;

	//	printf("%x:%x ", fDelayRP, fDelayWP);

	if (bits == 16) {
	  Deinterleave16To16Simple(tmpBuf[1], data, samplesToWrite);
	} else {
	  Deinterleave32To16Simple(tmpBuf[1], data, samplesToWrite);
	}

	down3L.Filter(&down3L, tmpBuf[1], samplesToWrite);
	t = down3R.Filter(&down3R, tmpBuf[1]+samplesToWrite, samplesToWrite);

	Interleave16To16(tmpBuf[0], tmpBuf[1], tmpBuf[1]+samplesToWrite, t);
	Reverb23(&room, tmpBuf[0], t);
	Deinterleave16To16(tmpBuf[1], tmpBuf[1]+TMP_BUF_SAMPLES/2, tmpBuf[0], t);
	up3L.Filter(&up3L, tmpBuf[1], t);
	t = up3R.Filter(&up3R, tmpBuf[1]+TMP_BUF_SAMPLES/2, t);

	if (bits == 16) {
	  Interleave16To16(tmpBuf[0], tmpBuf[1], tmpBuf[1]+TMP_BUF_SAMPLES/2, t);
	} else {
	  Interleave16To32(tmpBuf[0], tmpBuf[1], tmpBuf[1]+TMP_BUF_SAMPLES/2, t);
	}
	fwrite(tmpBuf[0], wordsPerSample, t, audio_orig);
      } else { /* !downsample */
	samplesToWrite = MIN(samplesToWrite, fDelayLen>>sampleShift);
	//	printf("%x:", fDelayRP);
	ringcpyYX(tmpBuf[0], MAKEMODLIN(1), fDelayP, fDelayModF, samplesToWrite*wordsPerSample);
	fDelayP = RING_XY_D(ringcpyXY(fDelayP, fDelayModF, data, MAKEMODLIN(1), samplesToWrite*wordsPerSample));
	if (bits != 16) Convert32ITo16I(tmpBuf[0], samplesToWrite);
	Reverb23(&room, tmpBuf[0], samplesToWrite);
	CombineDryWet(data, myDryGain, tmpBuf[0], samplesToWrite, bits);
	fwrite(data, wordsPerSample, samplesToWrite, audio_orig);
      }
      samplesLeft -= samplesToWrite;
      data += wordsPerSample*samplesToWrite;
    }
  }

  ReleaseMutex(myMutex+0);

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
  downsample = sampleRate > 48000;
  room.sampleRate = downsample ? sampleRate/3 : sampleRate;
  ObtainMutex(myMutex+1);
  ObtainMutex(myMutex+0);
  MyDesignRoom23(&room);
  ReleaseMutex(myMutex+0);
  ReleaseMutex(myMutex+1);
  return S_OK;
}
