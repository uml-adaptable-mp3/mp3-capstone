/**
   \file saturate.h Saturation functions.
*/

#ifndef SATURATE_H
#define SATURATE_H

#ifndef ASM

#include <vstypes.h>

/**
   Saturate 32-bit number to 16 bits.

   \param n 32-bit number to saturate.
   \return Saturated number.
*/
auto s_int16 Sat32To16(register __reg_a s_int32 n);
auto s_int16 Sat32To15(register __reg_a s_int32 n);
auto s_int16 Sat16To15(register __a0 s_int16 n);

#endif /* !ASM */

#endif /* !SATURATE_H */
