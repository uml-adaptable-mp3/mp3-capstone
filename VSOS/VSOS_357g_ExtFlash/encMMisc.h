#ifndef ENC_MP3_MISC_H
#define ENC_MP3_MISC_H

#include <vstypes.h>
#include "encMGeneral.h"

#ifndef M_PI
#define M_PI 3.1415926535
#endif

#define SIGNAL_MULTIPLIER_BITS 8
#define SIGNAL_MULTIPLIER (1<<SIGNAL_MULTIPLIER_BITS)

s_int16 EncMCanShiftUpVal32(register __reg_b u_int32 d);
s_int16 EncMCanShiftUpVal16(register __a1 u_int16 d);
auto s_int16 EncMCanShiftUp16(register __i0 s_int16 *d,
			      register __a1 s_int16 n);
auto void EncMShiftUp16(register __i0 s_int16 *d,
			register __a0 s_int16 n, register __a1 s_int16 sh);
auto void EncMAliasReduction(register __i0 s_int16 *x);
auto void EncMCopyStep(register __i0 const s_int16 *s,
		       register __i1 s_int16 sStep,
		       register __i2 s_int16 *d,
		       register __i3 s_int16 dStep,
		       register __a0 s_int16 n);
auto u_int32 EncMAbsSum(register __i0 const s_int16 *d,
			register __a0 s_int16 n);
auto u_int32 EncMBigValCount1(register __i0 const s_int16 *d,
			      register __b0 s_int16 n);

#define max(a,b) (((a)>(b))?(a):(b))
#define min(a,b) (((a)<(b))?(a):(b))

extern const u_int16 __mem_y encMPow34[256];

#if 0
s_int16 DToS16(double d);
u_int16 DToU16(double d);
#endif

extern const u_int16 encMScfsiBand[2][4];
extern const u_int16 __mem_y encMButrCSCAI[16];
extern const s_int16 __mem_y encDiv256[64];

#endif /* ! ENC_MP3_MISC_H */

