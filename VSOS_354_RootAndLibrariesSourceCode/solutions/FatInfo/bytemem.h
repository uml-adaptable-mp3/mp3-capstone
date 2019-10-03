#ifndef BYTE_MEM_H
#define BYTE_MEM_H

#include <stdlib.h>

#ifndef ASM

auto u_int16 GetBE8(register __i0 u_int16 *p, register u_int16 __b0 byteOffset);
auto u_int16 GetLE16(register __i0 u_int16 *p, register u_int16 __b0 byteOffset);
auto u_int16 GetBE16(register __i0 u_int16 *p, register u_int16 __b0 byteOffset);
auto u_int32 GetLE32(register __i0 u_int16 *p, register u_int16 __b0 byteOffset);
auto u_int32 GetBE32(register __i0 u_int16 *p, register u_int16 __b0 byteOffset);
void SetBE8(register __i0 u_int16 *p, register u_int16 __a0 byteOffset, register u_int16 __b0 data);
void SetBE16(register __i0 u_int16 *p, register u_int16 __a0 byteOffset, register u_int16 __b0 data);
void SetLE16(register __i0 u_int16 *p, register u_int16 __a0 byteOffset, register u_int16 __b0 data);
void SetBE32(register __i0 u_int16 *p, register u_int16 __a0 byteOffset, register u_int32 __reg_b data);
void SetLE32(register __i0 u_int16 *p, register u_int16 __a0 byteOffset, register u_int32 __reg_b data);
auto u_int16 BitReverse8(register __d0 u_int16 x);
auto u_int16 BitReverse16(register __d0 u_int16 x);
auto u_int32 BitReverse32(register __reg_d u_int32 x);
u_int32 BitReverseN32(register __reg_d u_int32 x, register __c0 u_int16 n);
u_int16 BitReverseN16(register __d0 u_int16 x, register __c0 u_int16 n);

#endif /* !ASM */

#endif /* !BYTE_MEM_H */
