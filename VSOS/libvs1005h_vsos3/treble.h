#ifndef VSTREBLE_H
#define VSTREBLE_H

#include <vstypes.h>

#define TREBLE_FREQS 7
#define TREBLE_COEFFS 9
#define TREBLE_MID_COEFF (TREBLE_COEFFS>>1)

#ifndef ASM

/* sFreq = sample Frequency, f = cut-off frequency in Hz,
   G is in 1.5 dB steps, 0 = -12.0 dB, 1 = -10.5 dB, etc. */
auto void Treble(register __i0 s_int16 *samples, register __a0 s_int16 n);
auto void SetTreble(u_int16 sFreq, u_int16 f, s_int16 trebleG, u_int16 gain);

#endif

#endif
