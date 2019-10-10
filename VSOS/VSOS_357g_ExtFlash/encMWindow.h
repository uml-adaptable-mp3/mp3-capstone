#ifndef ENC_MP3_WINDOW_H
#define ENC_MP3_WINDOW_H

#include <vstypes.h>

auto void EncMInitWindow(void);
auto void EncMWindow(const register __i0 s_int16 *in,
		     register __i2 s_int16 *out,
		     register __c0 s_int16 winStartIndex,
		     register __c1 s_int16 n);
auto void EncMHybridWindow(register __i0 s_int16 *d,
			   const register __i2 __mem_y u_int16 *win);
auto void EncMFoldWindow(const register __i0 s_int16 *s,
			 register __i2 s_int32 *d,
			 register __i3 s_int16 dMod,
			 register __b0 s_int16 mult,
			 register __c0 s_int16 n);

extern u_int16 __mem_y encMHybridWinFuncI[4][36];

#endif /* ! ENC_MP3_WINDOW_H */

