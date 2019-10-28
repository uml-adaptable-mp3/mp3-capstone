#ifndef INTERLEAVE_H
#define INTERLEAVE_H

#ifndef ASM

/*
  Note:
   - Left  channel samples are nominated 1-4
   - Right channel samples are nominated a-d
   - x = don't care
*/

/* (samp=4) s = 1 a 2 b 3 c 4 d -> d = 1 2 3 4 a b c d */
void Deinterleave16To16Simple(register __i1 s_int16 *d, register __i0 const s_int16 *s, register __d0 stereoSamples);

/* (samp=4) s = 1 a 2 b 3 c 4 d -> d = 0 1 0 2 0 3 0 4 0 a 0 b 0 c 0 d */
void Deinterleave16To32Simple(register __i1 s_int32 *d, register __i0 const s_int16 *s, register __d0 stereoSamples);

/* (samp=4) s = x 1 x a x 2 x b x 3 x c x 4 x d -> d = 1 2 3 4 a b c d */
void Deinterleave32To16Simple(register __i1 s_int16 *d, register __i0 const s_int32 *s, register __d0 stereoSamples);

/* (samp=4) s = 1 1 a a 2 2 b b 3 3 c c 4 4 d d -> d = 1 1 2 2 3 3 4 4 a a b b c c d d */
void Deinterleave32To32Simple(register __i1 s_int32 *d, register __i0 const s_int32 *s, register __d0 stereoSamples);

/* (samp=4) s = 1 2 3 4 a b c d -> d = 1 a 2 b 3 c 4 d */
void Interleave16To16Simple(register __i1 s_int16 *d, register __i0 const s_int16 *s, register __d0 stereoSamples);

/* (samp=4) s = 1 2 3 4 a b c d -> d = 0 1 0 2 0 3 0 4 0 a 0 b 0 c 0 d */
void Interleave16To32Simple(register __i1 s_int32 *d, register __i0 const s_int16 *s, register __d0 stereoSamples);

/* (samp=4) s = x 1 x 2 x 3 x 4 x a x b x c x d -> d = 1 a 2 b 3 c 4 d */
void Interleave32To16Simple(register __i1 s_int16 *d, register __i0 const s_int32 *s, register __d0 stereoSamples);

/* (samp=4) s = 1 1 2 2 3 3 4 4 a A b B c C d D -> d = 1 1 a A 2 2 b B 3 3 c C 4 4 d D */
void Interleave32To32Simple(register __i1 s_int32 *d, register __i0 const s_int32 *s, register __d0 stereoSamples);


/* The following functions are the same as previous except you can provide a pointer for each uninterleaved data area.
   None of the buffers may interleave. */

/* (samp=4) s = 1 a 2 b 3 c 4 d -> d1 = 1 2 3 4, d2 = a b c d */
void Deinterleave16To16(register __i1 s_int16 *d1, register __i2 s_int16 *d2, register __i0 const s_int16 *s, register __d0 stereoSamples);

/* (samp=4) s = 1 a 2 b 3 c 4 d -> d1 = 0 1 0 2 0 3 0 4, d2 = 0 a 0 b 0 c 0 d */
void Deinterleave16To32(register __i1 s_int32 *d1, register __i2 s_int32 *d2, register __i0 const s_int16 *s, register __d0 stereoSamples);

/* (samp=4) s = x 1 x a x 2 x b x 3 x c x 4 x d -> d1 = 1 2 3 4, d2 = a b c d */
void Deinterleave32To16(register __i1 s_int16 *d1, register __i2 s_int16 *d2, register __i0 const s_int32 *s, register __d0 stereoSamples);

/* (samp=4) s = 1 1 a a 2 2 b b 3 3 c c 4 4 d d -> d1 = 1 1 2 2 3 3 4 4, d2 = a a b b c c d d */
void Deinterleave32To32(register __i1 s_int32 *d1, register __i2 s_int32 *d2, register __i0 const s_int32 *s, register __d0 stereoSamples);

/* (samp=4) s1 = 1 2 3 4, s2 = a b c d -> d = 1 a 2 b 3 c 4 d */
void Interleave16To16(register __i1 s_int16 *d, register __i0 const s_int16 *s1, register __i2 const s_int16 *s2, register __d0 stereoSamples);

/* (samp=4) s1 = 1 2 3 4, s2 = a b c d -> d = 0 1 0 2 0 3 0 4 0 a 0 b 0 c 0 d */
void Interleave16To32(register __i1 s_int32 *d, register __i0 const s_int16 *s1, register __i2 const s_int16 *s2, register __d0 stereoSamples);

/* (samp=4) s1 = x 1 x 2 x 3 x 4, s2 = x a x b x c x d -> d = 1 a 2 b 3 c 4 d */
void Interleave32To16(register __i1 s_int16 *d, register __i0 const s_int32 *s1, register __i2 const s_int32 *s2, register __d0 stereoSamples);

/* (samp=4) s1 = 1 1 2 2 3 3 4 4, s2 = a A b B c C d D -> d = 1 1 a A 2 2 b B 3 3 c C 4 4 d D */
void Interleave32To32(register __i1 s_int32 *d, register __i0 const s_int32 *s1, register __i2 const s_int32 *s2, register __d0 stereoSamples);

/* Convert interleaved 32-bit to 16-bit data.
   (samp=2) x 1 x a x 2 x b -> 1 a 2 b */
void Convert32ITo16I(register __i1 s_int16 *d, register __d0 stereoSamples);
/* Convert interleaved 16-bit to 32-bit data.
   (samp=2) 1 a 2 b -> 0 1 0 a 0 2 0 b */
void Convert16ITo32I(register __i1 s_int16 *d, register __d0 stereoSamples);
#endif

#endif /* !INTERLEAVE_H */
