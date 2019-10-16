/**
   \file swap.h Byte swap functions.
*/

#ifndef SWAP_H
#define SWAP_H

#ifndef ASM

#include <vstypes.h>

/**
   Swap byte order in a 16-bit word. 0x1234 will be swapped to 0x3412.

   \param n Number to have its bytes switched.
   \return Swapped number.
*/
auto u_int16 Swap16(register __b0 u_int16 n);

/**
   Swap between big-endian and little-endian byte order for a 32-bit word.
   0x12345678 will be swapped to 0x78563412.

   \param n Number to have its bytes switched.
   \return Swapped number.
*/
auto u_int32 Swap32(register __reg_d u_int32 n);

/**
   Swap between little-endian and mixed-endian byte order for a 32-bit word.
   0x12345678 will be swapped to 0x34127856.

   \param n Number to have its bytes switched.
   \return Swapped number.
*/
auto u_int32 Swap32Mix(register __reg_d u_int32 n);

/**
   Swap 16-bit words inside a 32-bit word.
   0x12345678 will be swapped to 0x56781234.
*/
u_int32 Swap32Words(register __reg_a u_int32 n);

#endif /* !ASM */

#endif /* !SWAP_H */
