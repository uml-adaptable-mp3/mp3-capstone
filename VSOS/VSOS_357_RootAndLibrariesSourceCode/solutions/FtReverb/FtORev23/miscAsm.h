#ifndef MISC_ASM_H
#define MISC_ASM_H

#ifndef ASM

void CombineDryWet16(register __i0 s_int16 *dry, register __d0 u_int16 dryGain, register __i1 s_int16 *wet, register __d1 u_int16 n);
void CombineDryWet32(register __i0 s_int16 *dry, register __d0 u_int16 dryGain, register __i1 s_int16 *wet, register __d1 u_int16 n);

#endif /* !ASM */

#endif /* !MISC_ASM_H */
