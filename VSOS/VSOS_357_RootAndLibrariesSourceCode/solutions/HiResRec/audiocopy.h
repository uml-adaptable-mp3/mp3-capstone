#ifndef AUDIOCOPY_H
#define AUDIOCOPY_H

#include <vstypes.h>

#ifndef ASM

/* Finds maximum absolute value in a 16-bit stereo interleaved vector.
   Reads old values in max[0] and max[1] for left and right channel,
   respectively, and updates them. */
void FindAudioLevels2Ch16I(register __i0 const s_int16 *d, register __i2 u_int16 __mem_y *max, register __a0 s_int16 stereoSamples);
void FindAudioLevels4Ch16I(register __i0 const s_int16 *d, register __i2 u_int16 __mem_y *max, register __a0 s_int16 stereoSamples);

/* Finds maximum absolute value in a 32-bit stereo interleaved vector.
   Reads old values in max[0] and max[1] for left and right channel,
   respectively, and updates them. */
void FindAudioLevels2Ch32I(register __i0 const s_int32 *d, register __i2 u_int32 __mem_y *max, register __a0 s_int16 stereoSamples);
void FindAudioLevels4Ch32I(register __i0 const s_int32 *d, register __i2 u_int32 __mem_y *max, register __a0 s_int16 stereoSamples);

/* Applies soft volume to vector that is n 32-bit words long.
   Volume scale is such that min 0U = gain 0.0, and max 32768U = gain 1.0. */
void ApplySoftVol32(register __i0 s_int32 *d, register __a0 u_int16 volume, register __a1 s_int16 n);

/* Applies soft volume to vector that is n 32-bit words long, then shifts
   it by a given amount.
   Volume scale is such that min 0U = gain 0.0, and max 32768U = gain 1.0.
   After soft volume the result is shifted left (=up) by shiftLeft bits.
   shiftLeft range is from -39 to 39. The result is rounded and saturated. */
void ApplySoftVol32Shift(register __i0 s_int32 *d, register __a0 u_int16 volume, register __a1 s_int16 shiftLeft, register __d0 s_int16 n);



/* Converts 16-bit interleaved stereo vector from VSDSP byte order
   to little-endian byte order.
   Source and destination may be the same, otherwise they may not overlap. */
void Convert16BitVSDSPTo16BitLE(register __i3 s_int16 *d, register __i0 const s_int16 *s, register __d0 u_int16 stereoSamples);

/* Converts 16-bit interleaved stereo vector from little-endian byte order
   to VSDSP byte order.
   Source and destination may be the same, otherwise they may not overlap. */
void Convert16BitLETo16BitVSDSP(register __i3 s_int16 *d, register __i0 const s_int16 *s, register __d0 u_int16 stereoSamples);

/* Converts 16-bit interleaved stereo vector in VSDSP byte order
   to 32-bit little-endian byte order.
   Source and destination may be the same, otherwise they may not overlap. */
void Convert16BitVSDSPTo32BitLE(register __i3 s_int32 *d, register __i0 const s_int16 *s, register __d0 u_int16 stereoSamples);

/* Converts 16-bit interleaved stereo vector in little endian byte order
   to 32-bit VSDSP byte order.
   Source and destination may be the same, otherwise they may not overlap. */
void Convert16BitLETo32BitVSDSP(register __i3 s_int32 *d, register __i0 const s_int16 *s, register __d0 u_int16 stereoSamples);

/* Converts 32-bit interleaved stereo vector in VSDSP byte order
   to 16-bit little-endian byte order.
   Source and destination may be the same, otherwise they may not overlap. */
void Convert32BitVSDSPTo16BitLE(register __i3 s_int16 *d, register __i0 const s_int32 *s, register __d0 u_int16 stereoSamples);

/* Converts 32-bit interleaved stereo vector in little endian byte order
   to 16-bit VSDSP byte order.
   Source and destination may be the same, otherwise they may not overlap. */
void Convert32BitLETo16BitVSDSP(register __i3 s_int16 *d, register __i0 const s_int32 *s, register __d0 u_int16 stereoSamples);

/* Converts 32-bit interleaved stereo vector in VSDSP byte order
   to 24-bit little-endian byte order.
   Source and destination may be the same, otherwise they may not overlap. */
void Convert32BitVSDSPTo24BitLE(register __i3 void *d, register __i0 const s_int32 *s, register __d0 u_int16 stereoSamples);

/* Converts 24-bit interleaved stereo vector in little endian byte order
   to 32-bit VSDSP byte order.
   Source and destination may be the same, otherwise they may not overlap. */
void Convert24BitLETo32BitVSDSP(register __i3 s_int32 *d, register __i0 const void *s, register __d0 u_int16 stereoSamples);



/* Converts 16-bit interleaved stereo vector from VSDSP byte order
   to little-endian byte order.
   Words, samp = 1: 4433 ddcc -> 3344 ccdd */
void Convert16BitVSDSPTo16BitLEInPlace(register __i3 s_int16 *d, register __d0 u_int16 stereoSamples);

/* Converts 16-bit interleaved stereo vector from little-endian byte order
   to VSDSP byte order.
   Words, samp = 1: 3344 ccdd -> 4433 ddcc */
void Convert16BitLETo16BitVSDSPInPlace(register __i3 s_int16 *d, register __d0 u_int16 stereoSamples);

/* Converts 16-bit interleaved stereo vector in VSDSP byte order
   to 32-bit little-endian byte order. The vector must have enough
   space for the 32-bit result vector.
   Words, samp = 1: 4433 ddcc -> 0000 3344 0000 ccdd */
void Convert16BitVSDSPTo32BitLEInPlace(register __i3 void *d, register __d0 u_int16 stereoSamples);

/* Converts 16-bit interleaved stereo vector in little endian byte order
   to 32-bit VSDSP byte order. The vector must have enough
   space for the 32-bit result vector.
   Words, samp = 1: 3344 ccdd -> 0000 4433 0000 ddcc */
void Convert16BitLETo32BitVSDSPInPlace(register __i3 void *d, register __d0 u_int16 stereoSamples);

/* Converts 32-bit interleaved stereo vector in VSDSP byte order
   to 16-bit little-endian byte order.
   Words, samp = 1: 2211 4433 bbaa ddcc -> 3344 ccdd */
void Convert32BitVSDSPTo16BitLEInPlace(register __i3 void *d, register __d0 u_int16 stereoSamples);

/* Converts 32-bit interleaved stereo vector in little endian byte order
   to 16-bit VSDSP byte order.
   Words, samp = 1: 1122 3344 aabb ccdd -> 4433 ddcc */
void Convert32BitLETo16BitVSDSPInPlace(register __i3 void *d, register __d0 u_int16 stereoSamples);

/* Converts 32-bit interleaved stereo vector in VSDSP byte order
   to 24-bit little-endian byte order. 
   Words, samp = 1: 4433 2211 ddcc bbaa -> 2233 44bb ccdd */
void Convert32BitVSDSPTo24BitLEInPlace(register __i3 void *d, register __d0 u_int16 stereoSamples);

/* Converts 24-bit interleaved stereo vector in little endian byte order
   to 32-bit VSDSP byte order. The vector must have enough
   space for the 32-bit result vector.
   Words, samp = 1: 2233 44bb ccdd -> 2200 4433 bb00 ddcc */
void Convert24BitLETo32BitVSDSPInPlace(register __i3 void *d, register __d0 u_int16 stereoSamples);



/*
  Note! For the Interleave and Deinterleave functions:
   - 1-4  Left  channel 16-bit words
   - a-d  Right channel 16-bit words
   -   x  Don't care
   -   0  Zero fill
*/

/* samp = 4, s = 1a2b3c4d -> d = 1234abcd */
/* Each char above is one 16-bit word. */
void Deinterleave16To16Simple(register __i1 s_int16 *d, register __i0 const s_int16 *s, register __d0 stereoSamples);

/* samp = 4, s = 1a2b3c4d -> d = 010203040a0b0c0d */
/* Each char above is one 16-bit word. */
void Deinterleave16To32Simple(register __i1 s_int32 *d, register __i0 const s_int16 *s, register __d0 stereoSamples);

/* samp = 4, s = x1xax2xbx3xcx4xd -> d = 1234abcd */
/* Each char above is one 16-bit word. */
void Deinterleave32To16Simple(register __i1 s_int16 *d, register __i0 const s_int32 *s, register __d0 stereoSamples);

/* samp = 4, s = 11aA22bB33cC44dD -> d = 11223344aAbBcCdD */
/* Each char above is one 16-bit word. */
void Deinterleave32To32Simple(register __i1 s_int32 *d, register __i0 const s_int32 *s, register __d0 stereoSamples);

/* samp = 4, s = 1234abcd -> d = 1a2b3c4d */
/* Each char above is one 16-bit word. */
void Interleave16To16Simple(register __i1 s_int16 *d, register __i0 const s_int16 *s, register __d0 stereoSamples);

/* samp = 4, s = 1234abcd -> d = 010203040a0b0c0d */
/* Each char above is one 16-bit word. */
void Interleave16To32Simple(register __i1 s_int32 *d, register __i0 const s_int16 *s, register __d0 stereoSamples);

/* samp = 4, s = x1x2x3x4xaxbxcxd -> d = 1a2b3c4d */
/* Each char above is one 16-bit word. */
void Interleave32To16Simple(register __i1 s_int16 *d, register __i0 const s_int32 *s, register __d0 stereoSamples);

/* samp = 4, s = 11223344aAbBcCdD -> d = 11aA22bB33cC44dD */
/* Each char above is one 16-bit word. */
void Interleave32To32Simple(register __i1 s_int32 *d, register __i0 const s_int32 *s, register __d0 stereoSamples);


/* samp = 4, s = 1a2b3c4d -> d1 = 1234, d2 = abcd */
/* Each char above is one 16-bit word. */
void Deinterleave16To16(register __i1 s_int16 *d1, register __i2 s_int16 *d2, register __i0 const s_int16 *s, register __d0 stereoSamples);

/* samp = 4, s = 1a2b3c4d -> d1 = 01020304, d2 = 0a0b0c0d */
/* Each char above is one 16-bit word. */
void Deinterleave16To32(register __i1 s_int32 *d1, register __i2 s_int32 *d2, register __i0 const s_int16 *s, register __d0 stereoSamples);

/* samp = 4, s = x1xax2xbx3xcx4xd -> d1 = 1234, d2 = abcd */
/* Each char above is one 16-bit word. */
void Deinterleave32To16(register __i1 s_int16 *d1, register __i2 s_int16 *d2, register __i0 const s_int32 *s, register __d0 stereoSamples);

/* samp = 4, s = 11aA22bB33cC44dD -> d1 = 11223344, d2 = aAbBcCdD */
/* Each char above is one 16-bit word. */
void Deinterleave32To32(register __i1 s_int32 *d1, register __i2 s_int32 *d2, register __i0 const s_int32 *s, register __d0 stereoSamples);

/* samp = 4, s1 = 1234, s2 = abcd -> d = 1a2b3c4d */
/* Each char above is one 16-bit word. */
void Interleave16To16(register __i1 s_int16 *d, register __i0 const s_int16 *s1, register __i2 const s_int16 *s2, register __d0 stereoSamples);

/* samp = 4, s1 = 1234, s2 = abcd -> d = 010203040a0b0c0d */
/* Each char above is one 16-bit word. */
void Interleave16To32(register __i1 s_int32 *d, register __i0 const s_int16 *s1, register __i2 const s_int16 *s2, register __d0 stereoSamples);

/* samp = 4, s1 = x1x2x3x4, s2 = xaxbxcxd -> d = 1a2b3c4d */
/* Each char above is one 16-bit word. */
void Interleave32To16(register __i1 s_int16 *d, register __i0 const s_int32 *s1, register __i2 const s_int32 *s2, register __d0 stereoSamples);

/* samp = 4, s1 = 11223344, s2 = aAbBcCdD -> d = 11aA22bB33cC44dD */
/* Each char above is one 16-bit word. */
void Interleave32To32(register __i1 s_int32 *d, register __i0 const s_int32 *s1, register __i2 const s_int32 *s2, register __d0 stereoSamples);



/* Convert interleaved 32-bit to 16-bit data.
   (samp=2) x 1 x a x 2 x b -> 1 a 2 b */
void Convert32ITo16IInPlace(register __i1 s_int16 *d, register __d0 stereoSamples);
/* Convert interleaved 16-bit to 32-bit data.
   (samp=2) 1 a 2 b -> 0 1 0 a 0 2 0 b */
void Convert16ITo32IInPlace(register __i1 s_int16 *d, register __d0 stereoSamples);

/* 1 2 3 4 -> 1+3 2+4 */
void Matrix4ChTo2Ch16Bit(register __i0 s_int16 *d, register __i2 s_int16 *s, register __d0 s_int16 stereoSamples);

/* 1l 1h 2l 2h 3l 3h 4l 4h -> 1+3l 1+3h 2+4l 2+4h */
void Matrix4ChTo2Ch32Bit(register __i0 s_int32 *d, register __i2 s_int32 *s, register __d0 s_int16 stereoSamples);

#endif /* !ASM */

#endif /* AUDIOCOPY_H */
