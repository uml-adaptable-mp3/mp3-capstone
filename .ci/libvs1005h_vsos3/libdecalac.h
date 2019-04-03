#ifndef LIB_DEC_ALAC_H
#define LIB_DEC_ALAC_H

#include <vstypes.h>
#include <apploader.h>
#include <codecalac.h>

#define LibDecAlacCreate(lib) ((struct Codec *)(RunLoadedFunction(lib, ENTRY_1, 0)))

#endif /* !LIB_DEC_FLAC_H */
