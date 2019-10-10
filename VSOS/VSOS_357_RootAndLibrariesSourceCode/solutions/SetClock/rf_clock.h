#ifndef RF_CLOCK_H
#define RF_CLOCK_H

#ifndef ASM

#include <vo_stdio.h>

void InitRf(void);
u_int16 SetRfFreq(register u_int32 fmband);
u_int16 CheckRfLock(register u_int16 printena);

#endif /* !ASM */

#endif /* !RF_CLOCK_H */
