#ifndef ENC_VORBIS_MISC_H
#define ENC_VORBIS_MISC_H

#ifndef ASM

#include <vstypes.h>
#include "codVGeneral.h"

s_int16 ILog32s(register __reg_a s_int32 v);
#ifdef __VSDSP__
auto void MemCpySampleT(register __i0 sample_t *d, register __i2 sample_t *s,
			register __a0 u_int16 n);
#else
#define MemCpySampleT(d, s, n) memcpy((d),(s),(n))
#endif
auto void EncVShiftUpAndCopyToSamp(register __i0 sample_t *d,
				   register __i2 s_int16 *s,
				   register __a1 s_int16 shiftUp,
				   register __a0 s_int16 n);
auto void EncVSampleClear(register __i0 sample_t *d,
			  register __a0 s_int16 n);
auto u_int32 SIntToFltPack(register __reg_a s_int32 n);
auto void EncVUpdateHiVol(register __i0 const s_int16 __mem_y *d,
			  register __a0 s_int16 n,
			  register __a1 u_int16 stereo);
u_int16 GetI6(void);
#endif /* !ASM */

#endif /* !defined ENC_VORBIS_MISC_H */
