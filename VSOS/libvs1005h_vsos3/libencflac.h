#ifndef LIB_ENC_FLAC_H
#define LIB_ENC_FLAC_H

#include <vstypes.h>
#include <apploader.h>
#include <encoderflac.h>

#define LibEncFlacCreate(lib, es, channels, sampleRate, quality) ( ((struct Encoder * (*)(register struct EncoderServices *, register u_int16, register u_int32, register u_int16))(*((u_int16 *)(lib)+2+(ENTRY_1))))((es), (channels), (sampleRate), (quality)) )

#endif /* !LIB_ENC_FLAC_H */
