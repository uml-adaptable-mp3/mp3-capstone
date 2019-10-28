#ifndef LIB_COD_WAV_H
#define LIB_COD_WAV_H

#include <vstypes.h>
#include <apploader.h>
#include <codecvorbis.h>

#define LibCodWavCreate(lib) ((struct Codec *)(RunLoadedFunction(lib, ENTRY_1, 0)))

#endif /* !LIB_COD_WAV_H */
