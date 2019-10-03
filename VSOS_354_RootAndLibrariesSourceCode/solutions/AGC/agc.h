#ifndef __SUBSONIC_AND_AGC_H__
#define __SUBSONIC_AND_AGC_H__

#include <vstypes.h>

struct SubsonicM {
  s_int16 mem0;
  s_int32 mem1;
};

struct SubsonicG {
  s_int16 b0;
  s_int16 b1;
  s_int16 a1;
};

void InitSubsonic(register __i0 struct SubsonicM *subsonicM,
		  register __a0 s_int16 channels,
		  register __i1 struct SubsonicG __mem_y *subsonicG,
		  register __reg_d u_int32 sampleRate);

auto s_int16 Subsonic(register __i0 struct SubsonicM *mem,
		      const register __i1 struct SubsonicG __mem_y *g,
		      register __a0 s_int16 d);

void SubsonicN(register __i0 struct SubsonicM *mem,
	       const register __i1 struct SubsonicG __mem_y *g,
	       register __i2 s_int16 *d,
	       register __a0 u_int16 n);

struct Agc {
  s_int16 gain;   /* word  0 */
  u_int32 lpRes;  /* words 1, 2 */
  s_int32 offset; /* words 3, 4*/
};

/* agcSpeed may vary from 1 (fastest) through 7 (slowest).
   Adding one to agcSpeed doubles the attack time. Decay
   time is not affected. */
#define DEFAULT_AGC_ATTACK_DELAY 2

struct AgcConsts {
  u_int16 mr0;	// Should be MR_SAT
  u_int16 upShift; // Should be 5
  s_int16 decaySlowness1; // Should be -agcDecaySlowness
  u_int16 refLevel; // Should be 1024
  s_int16 decaySlowness2; // Should be -agcDecaySlowness-4
  u_int16 maxGain; // Default: 4096 = 4x
};

auto s_int16 AgcKeepDC(register __i0 struct Agc *agc,
		       register __i1 __mem_y const struct AgcConsts *consts,
		       register __a0 s_int16 sig);

void AgcN(register __i0 struct Agc *agc,
	  register __i1 __mem_y const struct AgcConsts *consts,
	  register __i2 s_int16 *d,
	  register __a0 u_int16 n);

void InitAgc(register struct Agc *agc, register s_int16 channels,
	     register struct AgcConsts __mem_y *agcConsts, register u_int16 maxGain,
	     register u_int32 sampleRate);	     

#define SetAgc(agcConsts, val) ((agcConsts)->maxGain = val)

#endif /*!__SUBSONIC_AND_AGC_H__*/
