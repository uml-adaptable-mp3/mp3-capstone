#ifndef LIB_DEC_DSD_H
#define LIB_DEC_DSD_H

#include <vstypes.h>
#include <apploader.h>
#include <codecdsd.h>

#define LibDecDsdCreate(lib) ((struct Codec *)(RunLoadedFunction(lib, ENTRY_1, 0)))

#endif /* !LIB_DEC_FLAC_H */
