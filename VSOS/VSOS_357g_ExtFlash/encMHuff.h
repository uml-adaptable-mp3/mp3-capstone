#ifndef ENCM_HUFF
#define ENCM_HUFF

#include <vstypes.h>
#include "encMGeneral.h"

#define HUFF2_TABS 30
#define HUFF4_TABS 2

struct HuffEnc {
  u_int16 maxVal;
  u_int16 maxValWithLinBits;
  u_int16 tabBits;
  u_int16 linBits;
  const s_int16 __mem_y *decodeTab;
  /** bitsAndCode is compressed as follows: for typical cases
      5 MSb's are the number of bits and 11 LSb's are the code.
      However, if bits = 21, then replace bits with 17 and 12
      LSb's are the code. This compression is just enough to fit
      in all MP3 compression tables. */
  const u_int16 __mem_y *bitsAndCode;
};

extern const struct HuffEnc __mem_y huffEnc2[HUFF2_TABS];
extern const struct HuffEnc __mem_y huffEnc4[HUFF4_TABS];

auto s_int16 EncMCountHuff2(register __i0 const struct EncoderMp3 *enc,
			    register __i1 const struct HuffEnc __mem_y *he,
			    register __i2 const s_int16 *x,
			    register __a0 s_int16 n);
auto s_int16 EncMCountHuff2Asm(register __i0 const s_int16 *d,
			       register __i2 const u_int16 __mem_y *bitsAndCode,
			       register __b0 s_int16 tabBits,
			       register __a1 s_int16 n);
auto s_int16 EncMCountHuff4(register __i0 const struct EncoderMp3 *enc,
			    register __i1 const struct HuffEnc __mem_y *he,
			    register __i2 const s_int16 *x,
			    register __a0 s_int16 n);
auto void EncMHuff2(register __i0 struct EncoderMp3 *enc,
		    register __i1 const struct HuffEnc __mem_y *he,
		    register __i2 s_int16 x[2]);
auto void EncMHuff2N(register __i0 struct EncoderMp3 *enc,
		     register __i1 const struct HuffEnc __mem_y *he,
		     register __i2 s_int16 *x,
		     register __a0 s_int16 n);
auto void EncMHuff4(register __i0 struct EncoderMp3 *enc,
		    register __i1 const struct HuffEnc __mem_y *he,
		    register __i2 s_int16 x[4]);

#endif
