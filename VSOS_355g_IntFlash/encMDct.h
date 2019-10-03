#ifndef ENC_MP3_DCT_H
#define ENC_MP3_DCT_H

#include <vstypes.h>
#include "encMGeneral.h"

void EncMInitDct(struct EncoderMp3 *enc);
auto void EncMDct32(register __i0 s_int16 *in, register __i2 s_int16 *out);
auto void EncMDct18(register __i0 s_int16 *in, register __i2 s_int16 *out);
auto void EncMDct18Asm(register __i0 s_int16 *in, register __i2 s_int16 *out);
auto void EncMDct6(register __i0 s_int16 *in, register __i2 s_int16 *out);

#endif /* ! ENC_MP3_DCT_H */
