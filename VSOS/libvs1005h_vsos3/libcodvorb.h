#ifndef LIB_COD_VORB_H
#define LIB_COD_VORB_H

#include <vstypes.h>
#include <apploader.h>
#include <codecvorbis.h>

#define LibCodVorbCreate(lib) ((struct Codec *)(RunLoadedFunction(lib, ENTRY_1, 0)))

#endif /* !LIB_CODEC_VORBIS_H */
