#ifndef XP_FIFORDWR_H
#define XP_FIFORDWR_H

#ifndef ASM

/** Reads n words from XP FIFO address addr to d. */
auto void xp_fiford(LCC_REGISTER u_int16 *d, register u_int16 addr,
		    register u_int16 n);
/** Writes n words from d to XP FIFO address addr. */
auto void xp_fifowr(LCC_REGISTER const u_int16 *d, register u_int16 addr,
		    register u_int16 n);
/** Writes x to XP FIFO address addr n times */
auto void xp_fifowrx(LCC_REGISTER u_int16 x, register u_int16 addr,
		     register u_int16 n);

/** Reads 8*n8x words from XP FIFO to d, leaves one empty cycle after each 8.
    Meant to be called by xp_fiford(). Returns d+8*n8x. */
auto u_int16 *XP_FifoRd8(register u_int16 __i0 *d, register __a0 u_int16 n8x);

/** Writes 8*n8x words from s to XP FIFO, leaves one empty cycle after each 8.
    Meant to be called by xp_fifowr(). Returns s+8*n8x. */
auto u_int16 *XP_FifoWr8(register const u_int16 __i0 *s,
			 register __a0 u_int16 n8x);

/** Writes x 8*n8x times to XP FIFO, leaves one empty cycle after each 8.
    Meant to be called by xp_fifowrx(). */
auto void XP_FifoWr8X(register u_int16 __b0 x, register __a0 u_int16 n8x);

#endif /* !ASM */

#endif /*  !XP_FIFORDWR_H */
