#ifndef ENC_VORBIS_TABLES_H
#define ENC_VORBIS_TABLES_H

#include <vstypes.h>
#include "codVTables.h"

#ifndef ASM

#ifdef __VSDSP__
typedef __fract short ff_int16;
typedef __fract long ff_int32;
#else
typedef double ff_int16;
typedef double ff_int32;
#endif

struct FLOOR1_fromdB_LOOKUP_Inv {
  u_int16 val;
  s_int16 shift;
};

#define ONE_PER_SIZE 1024
#define DB_RATIO_SIZE 64

extern const struct FLOOR1_fromdB_LOOKUP_Inv encVFloorInv[FLOOR1_LOOKUP_SIZE];
extern const ff_int16 __mem_y encVOnePer[ONE_PER_SIZE];
extern const ff_int16 __mem_y encVDbRatio[DB_RATIO_SIZE];

#endif /* !ASM */

#endif /* !ENC_VORBIS_TABLES_H */
