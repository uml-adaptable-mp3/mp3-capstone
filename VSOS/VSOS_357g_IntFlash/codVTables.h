#ifndef VORBIS_TABLES_H
#define VORBIS_TABLES_H

#include <vstypes.h>

#define FLOOR1_LOOKUP_SIZE 256
/* 6 is absolute maximum value */
#define LOOKUP_SHIFT 6

#ifndef ASM

extern const u_int16 __mem_y win64[], win128[], win256[];
extern const u_int16 __mem_y win512[], win1024[], win2048[];
extern const u_int16 __mem_y * const winPtr[];
extern const u_int32 FLOOR1_fromdB_LOOKUP_i[FLOOR1_LOOKUP_SIZE];

#endif

#endif
