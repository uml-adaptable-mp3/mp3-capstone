#ifndef LIB_DEC_AIFF_H
#define LIB_DEC_AIFF_H

#include <vstypes.h>
#include <apploader.h>

#define LibDecAiffCreate(lib) ((struct Codec *)(RunLoadedFunction(lib, ENTRY_1, 0)))

#endif /* !LIB_DEC_AIFF_H */
