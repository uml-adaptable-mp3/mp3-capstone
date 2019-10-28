#ifndef LIB_DEC_VORB_H
#define LIB_DEC_VORB_H

#include <vstypes.h>
#include <apploader.h>
#include <codecvorbis.h>

#define LibDecVorbCreate(lib) ((struct Codec *)(RunLoadedFunction(lib, ENTRY_1, 0)))

#endif /* !LIB_DEC_VORBIS_H */
