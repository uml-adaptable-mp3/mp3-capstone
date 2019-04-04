#ifndef BYTE_MANIPULATION_H
#define BYTE_MANIPULATION_H

#ifndef ASM

#include <stdlib.h>

/** Get 8-bite value from given byte offset. */
auto u_int16 GetBE8(register __i0 u_int16 *p, register u_int16 __b0 byteOffset);

/** Get 16-bit value from given byte offset interpreted as little-endian. */
auto u_int16 GetLE16(register __i0 u_int16 *p, register u_int16 __b0 byteOffset);

/** Get 16-bit value from given byte offset interpreted as big-endian. */
auto u_int16 GetBE16(register __i0 u_int16 *p, register u_int16 __b0 byteOffset);

/** Get 32-bit value from given byte offset interpreted as little-endian. */
auto u_int32 GetLE32(register __i0 u_int16 *p, register u_int16 __b0 byteOffset);

/** Get 16-bit value from given byte offset interpreted as little-endian. */
auto u_int32 GetBE32(register __i0 u_int16 *p, register u_int16 __b0 byteOffset);

/** Set 8-bite value at given byte offset. */
void SetBE8(register __i0 u_int16 *p, register u_int16 __a0 byteOffset, register u_int16 __b0 data);

/** Set 16-bite value interpreted as big-endian at given byte offset. */
void SetBE16(register __i0 u_int16 *p, register u_int16 __a0 byteOffset, register u_int16 __b0 data);

/** Set 16-bite value interpreted as little-endian at given byte offset. */
void SetLE16(register __i0 u_int16 *p, register u_int16 __a0 byteOffset, register u_int16 __b0 data);

/** Set 32-bite value interpreted as big-endian at given byte offset. */
void SetBE32(register __i0 u_int16 *p, register u_int16 __a0 byteOffset, register u_int32 __reg_b data);

/** Set 32-bite value interpreted as little-endian at given byte offset. */
void SetLE32(register __i0 u_int16 *p, register u_int16 __a0 byteOffset, register u_int32 __reg_b data);

/** Set three bytes to Cylinder / Head / Sector data */
void SetCHS(register __i0 u_int16 *p, register u_int16 __a0 byteOffset, register u_int32 __reg_b data);

/** Bit-reverse an 8-bit value */
auto u_int16 BitReverse8(register __d0 u_int16 x);

/** Bit-reverse a 16-bit value */
auto u_int16 BitReverse16(register __d0 u_int16 x);

/** Bit-reverse a 32-bit value */
auto u_int32 BitReverse32(register __reg_d u_int32 x);

/** Bit-reverse n bits where n <= 32 */
u_int32 BitReverseN32(register __reg_d u_int32 x, register __c0 u_int16 n);

/** Bit-reverse n bits where n <= 16 */
u_int16 BitReverseN16(register __d0 u_int16 x, register __c0 u_int16 n);

#endif /* !ASM */

#endif /* !BYTE_MANIPULATION_H */
