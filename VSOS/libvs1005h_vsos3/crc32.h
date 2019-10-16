#ifndef OGG_COMPATIBLE_CRC32_H
#define OGG_COMPATIBLE_CRC32_H
/**
   \file crc32.h Ogg compatible CRC-32 routines.
*/


#include <vstypes.h>

#ifndef ASM

/**
   Calculates a 32-bit Ogg compatible CRC.

   \param p Pointer to 16-bit data that is read in big-endian fashion.
   \param bytes Number of bytes to calculate CRC over. If \a bytes is odd,
	then last word's 8 MSb's are used for CRC calculation.
   \param crc Initial seed, or result of previous CRC calculation if
	doing the CRC calculation is more than one stage. If calculating
	Ogg CRC's set \a crc to 0.
*/
auto u_int32 CalcCrc32(register __reg_c u_int32 crc,
		       register __i0 const u_int16 *p,
		       register __a0 u_int16 bytes);

/**
   Calculates a 32-bit Ogg compatible CRC for one data byte. Calling this
   function as crc = CalcCrc32Byte(crc, byte) is equivalent to the following:
   { u_int16 p = byte; crc = CalcCrc32(&p, 1, crc); }

   \param byte The data byte to calculate the CRC32 over. In systems
	where a character is larger than 8 bits, bits outside of bit
	7:0 are ignored.
   \param crc Initial seed, or result of previous CRC calculation.
	For the first byte of calculating an Ogg CRC, set \a crc to 0.
*/
auto u_int32 CalcCrc32Byte(register __reg_c u_int32 crc,
			   register __b0 unsigned char byte);

/** Support table for Crc functions. */
extern const u_int32 __mem_y encVOggCrc[256];

#endif /* !ASM */

#endif /* !OGG_COMPATIBLE_CRC32_H */
