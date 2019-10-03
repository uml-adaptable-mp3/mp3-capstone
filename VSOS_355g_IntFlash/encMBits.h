#ifndef ENCM_BITS_H
#define ENCM_BITS_H

#include <vstypes.h>
#include "encMGeneral.h"

auto void EncMInitBits(register __i0 struct EncoderMp3 *enc);
auto s_int16 EncMOutBits16(register __i0 struct EncoderMp3 *e,
			   register __a0 u_int16 d, register __a1 s_int16 n);
auto register s_int16 EncMOutBits32(register __i0 struct EncoderMp3 *e,
				    register __reg_a u_int32 d,
				    register __b0 s_int16 n);
auto s_int16 EncMOutBit(register __i0 struct EncoderMp3 *e,
	       register __a0 u_int16 d);
auto s_int16 EncMAlignBitsToByte(register __i0 struct EncoderMp3 *e);
auto u_int16 EncMGetBitPtr(register __i0 struct EncoderMp3 *e);
auto s_int16 EncMInsertBits(register __i0 struct EncoderMp3 *e,
			    register __a0 u_int16 d,
			    register __a1 s_int16 n,
			    register __b0 s_int16 pos);
auto s_int16 EncMSetBitBytePtr(register __i0 struct EncoderMp3 *e,
			       register __a0 s_int16 bytePos);
auto s_int16 EncMMoveBitBytes(register __i0 struct EncoderMp3 *e,
			      register __a0 s_int16 d,
			      register __a1 s_int16 s,
			      register __b0 s_int16 n);
auto s_int16 EncMFlushBits(register __i0 struct EncoderMp3 *e);
auto s_int16 EncMWriteBitBytes(register __i0 struct EncoderMp3 *e,
			       register __a0 s_int16 d,
			       register __a1 s_int16 n);


#endif
