#ifndef LIB_DEC_MP3_H
#define LIB_DEC_MP3_H

#include <vstypes.h>
#include <apploader.h>
#include <codecmpg.h>

#define LibDecMp3Create(lib) ((struct Codec *)(RunLoadedFunction(lib, ENTRY_1, 0)))

#endif /* !LIB_DEC_WAV_H */
