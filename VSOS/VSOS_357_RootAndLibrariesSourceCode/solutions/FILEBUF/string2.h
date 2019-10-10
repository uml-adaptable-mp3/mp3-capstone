#ifndef _STRING2_H_
#define _STRING2_H_

#include <vstypes.h>
#include <stddef.h>

/** Copies bytes from one word buffer in Y to another in X memory. The bytes
    are packed in big-endian format, i.e. the first byte to the high bits of
    the word and the next byte to the low bits of the word.
    When using non-aligned start or end, the other half of the word retains
    its old value.  */
__near void MemCopyPackedBigEndianYX(register __i0 __near unsigned short *dst,
				   register __a0 unsigned short dstbyteidx,
				   register __i1 __near __mem_y unsigned short *src,
				   register __a1 unsigned short srcbyteidx,
				   register __b0 unsigned short byteSize);
/** Copies bytes from one word buffer in X to another in Y memory. The bytes
    are packed in big-endian format, i.e. the first byte to the high bits of
    the word and the next byte to the low bits of the word.
    When using non-aligned  start or end, the other half of the word retains
    its old value.  */
__near void MemCopyPackedBigEndianXY(register __i0 __near __mem_y unsigned short *dst,
				   register __a0 unsigned short dstbyteidx,
				   register __i1 __near unsigned short *src,
				   register __a1 unsigned short srcbyteidx,
				   register __b0 unsigned short byteSize);
/** Copies bytes from one word buffer in Y to another in Y memory. The bytes
    are packed in big-endian format, i.e. the first byte to the high bits of
    the word and the next byte to the low bits of the word. When using
    non-aligned start or end, the other half of the word retains its old
    value.  */

#endif /* _STRING2_H_ */
