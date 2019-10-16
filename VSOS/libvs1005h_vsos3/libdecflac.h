#ifndef LIB_DEC_FLAC_H
#define LIB_DEC_FLAC_H

#include <vstypes.h>
#include <apploader.h>
#include <codecflac.h>

#define LibDecFlacCreate(lib) ((struct Codec *)(RunLoadedFunction(lib, ENTRY_1, 0)))

#endif /* !LIB_DEC_FLAC_H */
