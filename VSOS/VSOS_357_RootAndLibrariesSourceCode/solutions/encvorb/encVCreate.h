/**
   \file encVCreate.h Vorbis 1.0 Encoder.
*/

#ifndef ENCODER_VORBIS_CREATE_H
#define ENCODER_VORBIS_CREATE_H

#include <vstypes.h>
#include <encoder.h>
#include <filter.h>

#ifndef ASM
/**
   Create and allocate space for encoder.

   \return A Vorbis Encoder structure.
*/
struct Encoder *EncVorbisCreate(struct EncoderServices *es,
				u_int16 channels, u_int32 sampleRate,
				u_int16 quality);
#endif /* !ASM */

#endif /* ENCODER_VORBIS_H */
