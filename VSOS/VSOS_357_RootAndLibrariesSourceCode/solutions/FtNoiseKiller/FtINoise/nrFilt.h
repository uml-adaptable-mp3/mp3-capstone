#ifndef NR_FILT_H
#define NR_FILT_H

#define NR_LEN_OFFSET       0
#define NR_BUFLEN_OFFSET    1
#define NR_MEM_OFFSET       2
#define NR_MEMCENTER_OFFSET 3
#define NR_COEFF_OFFSET     4
#define NR_SLOWMOD_OFFSET   5
#define NR_FASTMOD_OFFSET   6

/* Noise killer must always be called with
   NOISE_KILLER_HANDLE_SAMPLES stereo samples. */
#define NOISE_KILLER_HANDLE_SAMPLES 64


#ifndef ASM

struct AliasHBFilter {
  u_int16 len; /* Actual calculation points. */
  s_int16 bufLen; /* Size of buffer */
  s_int16 __mem_y *mem;	/* Must be aligned */
  s_int16 __mem_y *memCenter; /* Half-band filter center point. */
  s_int16 *coeff;
  u_int16 slowMod;
  u_int16 fastMod;
  s_int16 __mem_y *memBase; /* Only needed by C model. */
  u_int16 fastModStep;
};

void SplitAudio16Bands(register __i0 s_int16 *d, register __d1 s_int16 n);
void NoiseKiller(register __i0 s_int16 *d, register __d1 s_int16 n);
void CombineAudio16Bands(register __i0 s_int16 *d, register __d1 s_int16 n);
void DelayMono113(register __i0 s_int16 *d, register __d1 s_int16 n);

void RunAliasHBFilter(register __i0 struct AliasHBFilter *flt,
		      register __i1 s_int16 *d, 
		      register __i2 s_int16 *hiOut,
		      register __d0 s_int16 n);

void LRtoMS(register __i0 s_int16 *d, register __d0 s_int16 stereoSamples);
void MStoLR(register __i0 s_int16 *d, register __d0 s_int16 stereoSamples);

void InitNoiseKiller(void);
void SetNoiseKiller(register __d1 s_int16 dB);
s_int16 GetNoiseKiller(void);
void RunFmNoiseKiller(register __i0 s_int16 *d, register __c0 s_int16 stereoSamples);

extern struct AliasHBFilter aliasHBFilter[15];

#endif /* !ASM */

#endif /* !NR_FILT_H */
