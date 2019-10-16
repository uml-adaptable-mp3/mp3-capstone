#ifndef LIB_DEC_WAV_H
#define LIB_DEC_WAV_H

#include <vstypes.h>
#include <apploader.h>
#include <codecvorbis.h>

#define LibDecWavCreate(lib) ((struct Codec *)(RunLoadedFunction(lib, ENTRY_1, 0)))

#endif /* !LIB_DEC_WAV_H */
