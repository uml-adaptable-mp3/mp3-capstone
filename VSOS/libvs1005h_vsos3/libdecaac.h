#ifndef LIB_DEC_AAC_H
#define LIB_DEC_AAC_H

#include <vstypes.h>
#include <apploader.h>
#include <codecaac.h>

#define LibDecAacCreate(lib) ((struct Codec *)(RunLoadedFunction(lib, ENTRY_1, 0)))

#endif /* !LIB_DEC_AAC_H */
