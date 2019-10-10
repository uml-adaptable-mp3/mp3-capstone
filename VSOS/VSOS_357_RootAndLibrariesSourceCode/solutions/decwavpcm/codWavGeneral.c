/**
   \file codWavGeneral.c General WAV implementation

   This file contains common support WAV functions.
*/

#include <ctype.h>
#include <fir.h>
#include <ringbuf.h>
#include <stdlib.h>
#include "swap.h"
#include "codecwav.h"
#include "codWavGeneral.h"


#define MIN(a,b) (((a)<(b))?(a):(b))

#define PLAY_TIME_PER_SECOND 14




/**
   Reads a 32-bit word and interprets as big endian.

   \param cs Codec Services structure.
   \return Big endian interpreted value.
 */
u_int32 CodWavRead32(register struct CodecServices *cs, register u_int16 *err) {
  register u_int32 res = 0;
  register int i;
  u_int16 d = 0;
  if (cs->fileLeft < 4) {
    *err = 1;
    return 0;
  }
  for (i=0; i<4; i++) {
    if (cs->Read(cs, &d, 1, 1) != 1) {
      *err = 1;
      return 0;
    }
    res = (res << 8) | d;
  }
  return res;
}


/**
   Reads a 32-bit word and interprets as little endian.

   \param cs Codec Services structure.
   \return Little endian interpreted value.
 */
u_int32 CodWavRead32I(register struct CodecServices *cs, register u_int16 *err){
  return Swap32(CodWavRead32(cs, err));
}

#ifndef __VSDSP__
unsigned short QsortLog2(short n) {
  int bits=0;
  int n2 = abs(n) - (n<0);
  while (n2) {
    bits++;
    n2 >>= 1;
  }
  return n;
}
#endif

void CodWavOutput(register struct CodecWav *cw,
		  register struct CodecServices *cs,
		  register s_int16 *data, register s_int16 n) {
  s_int32 phase = cw->fastForwardPhase;
  s_int16 skip = (n<0) ? 2 : 1;
  int absN = abs(n);
  int absNCh = absN*cs->channels;
  int fadeShift = QsortLog2(absNCh)-1;
  int fadeSamples = 0;

  cs->sampleRate = cw->fileSampleRate;
  cs->playTimeSeconds = cw->realPlayTimeSeconds;
  cs->playTimeSamples = cw->realPlayTimeSamples;
  
  if (fadeShift > 0) {
    if (fadeShift > 6)
      fadeShift = 6;
    fadeSamples = (1<<fadeShift)-1;
  }

#if 0
  printf("fadeSh = %d, samp %d\n", fadeShift, fadeSamples);
  if (absN < 16 || kuuk) {
    printf("n=%d\n", n);
    kuuk=1;
  }
#endif

  if (cw->fadeIn) {
    s_int16 *d2 = data;
    s_int16 i;
    for (i=1; i<=fadeSamples; i++) {
      *d2 = (s_int16)((s_int32)(*d2) * i >> fadeShift);
      d2 += skip;
    }
    cw->fadeIn = 0;
  }
 
  if (phase >= 0 || cs->fastForward <= 1) {
    if (cs->fastForward > 1) {
      phase += absN;
      if (phase > cs->sampleRate/PLAY_TIME_PER_SECOND) {
	s_int16 *d2 = data + skip*(absNCh-fadeSamples) + skip-1;
	s_int16 i;
	phase = -phase*(cs->fastForward-1);
	for (i=fadeSamples; i>0; i--) {
	  *d2 = (s_int16)((s_int32)(*d2) * i >> fadeShift);
	  d2 += skip;
	}
      }
    }
    cs->Output(cs, data, n);
  } else {
    phase += absN;
    if (phase >= 0) {
      cw->fadeIn = 1;
    }
  }
  cw->fastForwardPhase = phase;
}
