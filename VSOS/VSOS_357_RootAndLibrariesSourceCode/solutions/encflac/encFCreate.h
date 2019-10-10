/**
   \file encFCreate.h Flac Encoder.
*/

#ifndef ENCODER_FLAC_CREATE_H
#define ENCODER_FLAC_CREATE_H

#include <vstypes.h>
#include <encoder.h>
#include <filter.h>

#ifndef ASM
/**
   Create and allocate space for encoder.

   \return A Flac Encoder structure.
*/
struct Encoder *EncFlacCreate(struct EncoderServices *es,
			      u_int16 channels, u_int32 sampleRate,
			      u_int16 quality);
#endif /* !ASM */

#endif /* ENCODER_FLAC_H */
