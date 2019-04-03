#ifndef RING_BUF_H
#define RING_BUF_H

#include <vstypes.h>

#ifndef ASM

/** Copies data from X to X memory. Either buffer can be a linear buffer, a
    ring buffer, or a bit-reverse buffer.
    \param d Destination start address.
    \param dMod VSDSP modifier for destination address.
    \param s Source start address.
    \param sMod VSDSP modifier for source address.
    \param n Number of words to copy.
    \return New \a d in bits 31:16, and new \a s in bits 15:0. To
	extract these bits, use macros RING_XX_D() and RING_XX_S(),
	respectively.
    \note If \a d or \a s is a ring buffer or bit-reverse
	buffer, the base address of the buffer needs to be a power of two
	at least as large or larger than the buffer. To get such alignment
	for a buffer, use the keyword __align .
    \note To get a modifier register for \a dMod or \a sMod, either
	use macros MAKEMODLIN(), MAKEMOD(), MAKEMODF(), MAKEMODR(), or
	MAKEMOD64(), or the all-encompassing function GetRingModifier() .
*/
auto u_int32 ringcpyXX(register __i0 u_int16 *d,
		       register __i1 u_int16 dMod,
		       register __i2 const u_int16 *s,
		       register __i3 u_int16 sMod,
		       register __b0 u_int16 n);
#define ringcpy(a,b,c,d,e) (ringcpyXX((a),(b),(c),(d),(e)))

/** Copies data from X to Y memory. Either buffer can be a linear buffer, a
    ring buffer, or a bit-reverse buffer.
    \param d Destination start address.
    \param dMod VSDSP modifier for destination address.
    \param s Source start address.
    \param sMod VSDSP modifier for source address.
    \param n Number of words to copy.
    \return New \a d in bits 31:16, and new \a s in bits 15:0. To
	extract these bits, use macros RING_XY_D() and RING_XY_S(),
	respectively.
    \note If \a d or \a s is a ring buffer or bit-reverse
	buffer, the base address of the buffer needs to be a power of two
	at least as large or larger than the buffer. To get such alignment
	for a buffer, use the keyword __align .
    \note To get a modifier register for \a dMod or \a sMod, either
	use macros MAKEMODLIN(), MAKEMOD(), MAKEMODF(), MAKEMODR(), or
	MAKEMOD64(), or the all-encompassing function GetRingModifier() .
*/
auto u_int32 ringcpyXY(register __i0 u_int16 __mem_y *d,
		       register __i1 u_int16 dMod,
		       register __i2 const u_int16 *s,
		       register __i3 u_int16 sMod,
		       register __b0 u_int16 n);

/** Copies data from Y to X memory. Either buffer can be a linear buffer, a
    ring buffer, or a bit-reverse buffer.
    \param d Destination start address.
    \param dMod VSDSP modifier for destination address.
    \param s Source start address.
    \param sMod VSDSP modifier for source address.
    \param n Number of words to copy.
    \return New \a d in bits 31:16, and new \a s in bits 15:0. To
	extract these bits, use macros RING_YX_D() and RING_YX_S(),
	respectively.
    \note If \a d or \a s is a ring buffer or bit-reverse
	buffer, the base address of the buffer needs to be a power of two
	at least as large or larger than the buffer. To get such alignment
	for a buffer, use the keyword __align .
    \note To get a modifier register for \a dMod or \a sMod, either
	use macros MAKEMODLIN(), MAKEMOD(), MAKEMODF(), MAKEMODR(), or
	MAKEMOD64(), or the all-encompassing function GetRingModifier() .
*/
auto u_int32 ringcpyYX(register __i0 u_int16 *d,
		       register __i1 u_int16 dMod,
		       register __i2 const u_int16 __mem_y *s,
		       register __i3 u_int16 sMod,
		       register __b0 u_int16 n);

/** Copies data from Y to Y memory. Either buffer can be a linear buffer, a
    ring buffer, or a bit-reverse buffer.
    \param d Destination start address.
    \param dMod VSDSP modifier for destination address.
    \param s Source start address.
    \param sMod VSDSP modifier for source address.
    \param n Number of words to copy.
    \return New \a d in bits 31:16, and new \a s in bits 15:0. To
	extract these bits, use macros RING_YY_D() and RING_YY_S(),
	respectively.
    \note If \a d or \a s is a ring buffer or bit-reverse
	buffer, the base address of the buffer needs to be a power of two
	at least as large or larger than the buffer. To get such alignment
	for a buffer, use the keyword __align .
    \note To get a modifier register for \a dMod or \a sMod, either
	use macros MAKEMODLIN(), MAKEMOD(), MAKEMODF(), MAKEMODR(), or
	MAKEMOD64(), or the all-encompassing function GetRingModifier() .
*/
auto u_int32 ringcpyYY(register __i0 u_int16 __mem_y *d,
		       register __i1 u_int16 dMod,
		       register __i2 const u_int16 __mem_y *s,
		       register __i3 u_int16 sMod,
		       register __b0 u_int16 n);


/** Modifies the X pointer \a p with modifier \a mod */
u_int16         *ringmodX(register __i0 u_int16     *xp,
		      register __i1 u_int16 mod);
/** Modifies the Y pointer \a p with modifier \a mod */
u_int16 __mem_y *ringmodY(register __i0 u_int16 __mem_y *yp,
		      register __i1 u_int16 mod);

/** Creates a linear modifier register with \a step = -8192..8191 */
#define MAKEMODLIN(stp) (stp)
/** Creates a modifier register with \a step = -128..127, \a bufsize 64,128,...,4096 */
#define MAKEMOD64(stp, bufsz) (0x4000|(((bufsz)/64-1)&31)|(((stp)&255)<<6))
/** Creates a modifier register with \a step = -64..63, \a bufsz = 1..64. */
#define MAKEMOD(stp, bufsz) (0x2000|(((stp)&127)<<6)|(((bufsz)-1)&63))
/** Creates a forwards modifier register, \a bufsz = 1..8192. */
#define MAKEMODF(bufsz)     (0x8000|(((bufsz)-1)&8191))
/** Creates a backwards modifier register, \a bufsz = 1..8192. */
#define MAKEMODB(bufsz)     (0xA000|(((bufsz)-1)&8191))

/**
   Creates a VSDSP4 compatible modifier register for a linear or ring buffer.
   \param step Modification step size in words.
   \param bufSize The size of the ring buffer in words. If zero, then a linear
	modification (without a ring buffer) is created.
   \return Modifier value, or zero if creating a modification
	register with the given constraints is not possible.
   \note There are hardware-related constraints for the values of buffer and
	step sizes. Only the following arguments are valid:
	For \a bufSize = 0 (linear modification), \a step = -8192...8191
		(MAKEMODLIN()).
	For \a step = -1,1, \a bufSize = -8192..8191
		(MAKEMODF(), MAKEMODB()).
	For \a bufSize = 1...63, \a step = -64...63
		(MAKEMOD()).
	For \a bufSize = 64,128,192,256,...4096, \a step = -128...127
		(MAKEMOD64()).
   \note If \a step = \a bufSize = 0, the trivial return value would be 0.
	However, as zero is used as an error code, the function will in this
	case return 0x2000, which is a modification 0 for a 64-word buffer,
	creating the same effect as a linear 0 modifier.
 */
auto u_int16 GetRingModifier(register s_int16 step, register u_int16 bufSize);

#define GET_MODIFIER_ERROR 0

/** Returns new destination pointer from result of ringcpyYY */
#define RING_XX_D(a) ((u_int16         *)((a)>>16))
#define RING_XX_S(a) ((u_int16         *)((a)&0xFFFFU))
#define RING_XY_D(a) ((u_int16 __mem_y *)((a)>>16))
#define RING_XY_S(a) ((u_int16         *)((a)&0xFFFFU))
#define RING_YX_D(a) ((u_int16         *)((a)>>16))
#define RING_YX_S(a) ((u_int16 __mem_y *)((a)&0xFFFFU))
#define RING_YY_D(a) ((u_int16 __mem_y *)((a)>>16))
#define RING_YY_S(a) ((u_int16 __mem_y *)((a)&0xFFFFU))

#else /* ASM */

#include <ringbufasm.h>

#endif /* ASM */


#endif /* !RING_BUF_H */
