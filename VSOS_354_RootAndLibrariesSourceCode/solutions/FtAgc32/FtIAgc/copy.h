#ifndef ICOPY_H
#define ICOPY_H

#include <vstypes.h>

#ifndef ASM

/** Copies data from 16-bit interleaved buffer to 32-bit consecutive buffer
    and scale.
*/
auto void Copy16iTo32Scale(register __i0 s_int32 *d,
			   register __i2 s_int16 *s,
			   register __a0 s_int16 n,
			   register __c0 s_int16 safetyBits);
/** Copies data from 32-bit consecutive buffer to 16-bit interleaved buffer
    and scale. Adds offset to result.
*/
auto void Copy32To16iScale(register __i0 s_int16 *d,
			   register __i2 s_int32 *s,
			   register __a0 s_int16 n,
			   register __d1 s_int16 addOffset,
			   register __a1 s_int16 safetyBits);

/** Scale up 32-bit audio data in-place. If shiftLeft is negative, scale down.
*/
auto void ScaleUp32(register __i0 s_int32 *d,
		    register __a0 s_int16 n,
		    register __c0 s_int16 shiftLeft);


/** Copies data from 32-bit interleaved buffer to 32-bit consecutive buffer
    and scale.
*/
auto void Copy32iTo32Scale(register __i0 s_int32 *d,
			   register __i2 s_int32 *s,
			   register __a0 s_int16 n,
			   register __c0 s_int16 safetyBits);
/** Copies data from 32-bit consecutive buffer to 32-bit interleaved buffer
    and scale. Adds offset to result.
*/
auto void Copy32To32iScale(register __i0 s_int32 *d,
			   register __i2 s_int32 *s,
			   register __a0 s_int16 n,
			   register __c0 s_int16 safetyBits);


#endif /* !ASM */



#endif /* !ICOPY_H */
