#ifndef XP_FIFORDWR_H
#define XP_FIFORDWR_H

#ifndef ASM

/** Reads 8*n8x words from XP FIFO to d, leaves one empty cycle after each 8.
    Meant to be called by XP_FifoRd(). */
auto u_int16 *XP_FifoRd8(register u_int16 __i0 *d, register __a0 u_int16 n8x);

/** Writes 8*n8x words from s to XP FIFO, leaves one empty cycle after each 8.
    Meant to be called by XP_FifoWr(). */
auto u_int16 *XP_FifoWr8(register const u_int16 __i0 *s,
		     register __a0 u_int16 n8x);

#endif /* !ASM */

#endif /*  !XP_FIFORDWR_H */
