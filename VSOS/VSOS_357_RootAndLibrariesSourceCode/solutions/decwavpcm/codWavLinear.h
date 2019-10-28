/**
   \file codWavLinear.h PCM Wav Codec.
*/

#ifndef COD_WAV_LINEAR_H
#define COD_WAV_LINEAR_H

#include <vstypes.h>
#include <codec.h>
#include "codWavGeneral.h"

#ifndef ASM

enum CodecError CodWavDecodeLinear(register struct CodecWav *cw,
				   register struct CodecServices *cs,
				   u_int32 bytes, const char **errorString);

#endif

#endif
