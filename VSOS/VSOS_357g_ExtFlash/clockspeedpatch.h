#ifndef CLOCK_SPEED_PATCH
#define CLOCK_SPEED_PATCH

#include <vstypes.h>

#ifndef ASM

auto void DelayMicroSecPatch(u_int32 microSec);

void BusyDelay4x(register __reg_a u_int32 clocksPer8);

#endif /* !ASM */


#endif /* !CLOCK_SPEED_PATCH */
