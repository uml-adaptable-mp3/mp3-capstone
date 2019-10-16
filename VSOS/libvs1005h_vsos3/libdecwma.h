#ifndef LIB_DEC_WMA_H
#define LIB_DEC_WMA_H

#include <vstypes.h>
#include <apploader.h>
#include <codecwma.h>

#define LibDecWmaCreate(lib) ((struct Codec *)(RunLoadedFunction(lib, ENTRY_1, 0)))

#endif /* !LIB_DEC_WMA_H */
