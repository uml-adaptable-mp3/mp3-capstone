/**
   \file codWavLinear.c PCM Wav Codec. This codec plays PCM RIFF WAV files
   in mono or stereo.
*/

#include <vstypes.h>
#include <filter.h>
#include <stdlib.h>
#include <dsplib.h>
#include <string.h>
#include "swap.h"
#include "codecwav.h"
#include "codWavGeneral.h"
#include "codWavLoop.h"
#include "codWavLinear.h"



#ifdef __VSDSP__
s_int32 ShiftLeftAndSat32(register __reg_d s_int32 val,
			  register __b0 s_int16 shLeft);
#else
s_int32 ShiftLeftAndSat32(register __reg_d s_int32 val,
			  register __b0 s_int16 shLeft) {
  if (shLeft > 0) {
    if ((u_int32)labs(val) >= (0x80000000UL>>shLeft)) {
      if (val > 0) {
	return 0x7FFFFFFF;
      } else {
	return -0x80000000;
      }
    }
    return val << shLeft;
  } else {
    return val >> -shLeft;
  }
}
#endif

#ifndef __VSDSP__
void IEEEto32(register __i2 void *from, register __i3 s_int32 *to,
              register __a0 s_int16 samples) {
  int i;
  u_int16 *from2 = from;
  u_int16 *to2 = to;
  /* 32-bit floating point representation */
  for (i=0; i<samples; i++) {
    s_int32 t;
    s_int16 ex;
    /* LLMM MMHH */
    t = (*from2 << 8) | (*from2 >> 8);
    from2++;
    t |= ((u_int32)((*from2 << 8) | (*from2 >> 8))) << 16;
    from2++;
    ex = ((t >> 23) & 255);
    t |= 0x00800000UL; /*1.fraction*/
    /* printf(" %08lx ex %d %ld\n", t, ex, t & 0x00ffffffUL); */
    if (t & 0x80000000UL) {
      t = -(t & 0x00ffffffUL);
    } else {
      t &= 0x00ffffffUL;
    }
#define FL_SCALE 8
    if (ex != 0) {
      t = ShiftLeftAndSat32(t, ex - (127-16 + FL_SCALE));
    } else {
      t = 0;
    }
    *to2++ = (u_int16)t;
    *to2++ = (u_int16)(t>>16);
  }
}
#endif


s_int16 CodWavCheckJump1(register struct CodecWav *cw,
			 register struct CodecServices *cs,
			 register u_int16 bytesPerSample,
			 register u_int16 maxSamples,
			 register const char **errorString) {
  return 0;
}



void CodWavCheckJump2(register struct CodecWav *cw,
		      register struct CodecServices *cs,
		      register u_int16 bytesPerSample) {
}




/**
   Decodes an 8-bit, 16-bit, 24-bit, or 32-bit linear or 32-bit floating point
   mono or stereo PCM RIFF WAV file data chunk.

   \param cw Wav Codec.
   \param cs Codec Services. During playback the codec updatable fields of
	cs are updated.
   \param bytes The number of bytes for the data format.
   \param errorString A destination where to write error messages to.
	This is updated only if the function returns something else
	than ceOk.
   \return Error code or ceOk if everything went well..
*/
enum CodecError CodWavDecodeLinear(register struct CodecWav *cw,
				   register struct CodecServices *cs,
				   u_int32 bytes, const char **errorString) {
  s_int16 __mem_y maxSamples = MAX_FILTER_SAMPLES >> (cw->bitsPerSample>16);
  int bytesPerSample = cw->bitsPerSample/8;
  u_int32 samplesLeft;

  cs->playTimeTotal = bytes / (cs->avgBitRate/8);
  if (cs->channels < 1 || cs->channels > 2) {
    *errorString = "RIFF WAV has wrong number of channels";
    return ceFormatNotSupported;
  }

  cw->samples = samplesLeft = bytes / (cs->channels * bytesPerSample);

  while (samplesLeft && !cs->cancel) {
    s_int16 i;
    u_int16 bytesToRead;
    u_int32 gotoSeconds, gotoSamples;
    s_int16 toHandle, toHandleMultiCh;

    gotoSeconds = 0xFFFFFFFFUL;
    gotoSamples = 0xFFFFFFFFUL;

    toHandle = MIN(samplesLeft, maxSamples);

    if (cs->Seek) {
      struct WavLoop *loop = (struct WavLoop *)cs->loop;
      if (cs->goTo != 0xFFFFU) {
	gotoSeconds = cs->goTo;
	gotoSamples = 0;
	cs->goTo = 0xFFFFU;
      }

      if (loop && loop->currentLoop < loop->numberOfLoops) {
	u_int32 loopEndSample = loop->endSeconds*cw->fileSampleRate +
	  loop->endSamples;
	u_int32 samplesAfterThisFrame =
	  cw->realPlayTimeSeconds*cw->fileSampleRate +
	  cw->realPlayTimeSamples + toHandle;
	if (samplesAfterThisFrame > loopEndSample || toHandle == samplesLeft) {
	  if (++loop->currentLoop < loop->numberOfLoops) {
	    u_int32 samplesToPlayInThisFrame =
	      toHandle - (samplesAfterThisFrame - loopEndSample);
	    if (samplesToPlayInThisFrame < toHandle) {
	      toHandle = samplesToPlayInThisFrame;
	    }
	    gotoSeconds = loop->startSeconds;
	    gotoSamples = loop->startSamples;
	  }
	}
      }
    }
    toHandleMultiCh = toHandle * cs->channels;
    bytesToRead = toHandleMultiCh * bytesPerSample;
    if (cs->Read(cs, cw->buf, 0, bytesToRead) != bytesToRead) {
      *errorString = "Read error";
      return ceUnexpectedFileEnd;
    }

    /* Process samples */
    if (cw->bitsPerSample == 16 || cw->bitsPerSample == 32) {
      u_int16 *bpd = cw->buf;

      if (cw->coding == wcLinear) {
	u_int16 words = toHandleMultiCh*bytesPerSample>>1;
	for (i=0; i<words; i++) {
	  *bpd = Swap16(*bpd);
	  bpd++;
	}
      } else {
	IEEEto32(bpd, bpd, toHandleMultiCh);
      }
    } else if (cw->bitsPerSample == 8) {
      u_int16 *bpd = cw->buf;
      u_int16 *bps = cw->buf+MAX_FILTER_SAMPLES;
      s_int16 samplesPer2 = (toHandleMultiCh+1)>>1;

      memmove(bps, bpd, MAX_FILTER_SAMPLES*sizeof(*bps));

      /* May copy one sample too much, but that's ok: we won't play it. */
      for (i=0; i<samplesPer2; i++) {
	*bpd++ = (*bps & 0xFF00) ^ 0x8000U;
	*bpd++ = (*bps++ << 8  ) ^ 0x8000U;
      }
    } else if (cw->bitsPerSample == 24) {
      s_int16 i;
      u_int16 *bpd = cw->buf;
      u_int16 *bps = cw->buf+MAX_FILTER_SAMPLES/2;

      memmove(bps, bpd, MAX_FILTER_SAMPLES*6/4*sizeof(*bps));

      for (i=0; i<toHandleMultiCh; i+=2) {
	*bpd++ = *bps & 0xFF00U;
	*bpd = *bps++ & 0x00FFU;
	*bpd++ |= *bps & 0xFF00U;
	*bpd++ = Swap16(*bps++ & 0x00FFU);
	*bpd++ = Swap16(*bps++);
      }
    } else {
      *errorString = "Bit depth not supported for linear RIFF WAV";
      return ceFormatNotSupported;
    }

    cw->realPlayTimeSamples += toHandle;
    if (cw->realPlayTimeSamples > cw->fileSampleRate) {
      cw->realPlayTimeSamples -= cw->fileSampleRate;
      cw->realPlayTimeSeconds++;
    }

    CodWavOutput(cw, cs, (s_int16 *)cw->buf,
		 (bytesPerSample > 2) ? -toHandle : toHandle);

    samplesLeft -= toHandle;

    if (gotoSeconds != 0xFFFFFFFFU) {
      u_int32 realDestSample = gotoSeconds * cw->fileSampleRate + gotoSamples;
      if (realDestSample > cw->samples)
	realDestSample = cw->samples;

      cw->realPlayTimeSeconds = gotoSeconds;
      cw->realPlayTimeSamples = gotoSamples;

      cs->Seek(cs, realDestSample*cs->channels*bytesPerSample+cw->dataStartPos,
	       SEEK_SET);
      samplesLeft = cw->samples-realDestSample;
    }

  }
  return ceOk;
}
