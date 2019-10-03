#ifndef LIB_ENC_MP3_H
#define LIB_ENC_MP3_H

#include <vstypes.h>
#include <apploader.h>
#include <encodermp3.h>

#define LibEncMp3Create(lib, es, channels, sampleRate, quality) ( ((struct Encoder * (*)(register struct EncoderServices *, register u_int16, register u_int32, register u_int16))(*((u_int16 *)(lib)+2+(ENTRY_1))))((es), (channels), (sampleRate), (quality)) )

#endif /* !LIB_ENC_MP3_H */
