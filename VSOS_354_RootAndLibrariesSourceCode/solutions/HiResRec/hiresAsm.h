#ifndef HIRES_RECORDER_ASM_H
#define HIRES_RECORDER_ASM_H

#include <vstypes.h>

#ifndef ASM

void FindAudioLevels16(register __i0 s_int16 *d,
		       register __i2 u_int16 __mem_y *max,
		       register __a0 s_int16 stereoSamples);
void FindAudioLevels32(register __i0 s_int32 *d,
		       register __i2 u_int32 __mem_y *max,
		       register __a0 s_int16 stereoSamples);

#endif

#endif /* HIRES_RECORDER_ASM_H */
