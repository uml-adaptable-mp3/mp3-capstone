#ifndef VSBASS_H
#define VSBASS_H

#include <vstypes.h>

#define BASS_SAMPLE_BLOCK 64
auto void Bass(s_int16 *samples, register __a0 s_int16 n);
void SetBass(register __c0 u_int16 sFreq, register __c1 u_int16 low,
	     u_int16 compMaxG, u_int16 trebleG);

#endif
