#ifndef REVERB_INT_H
#define REVERB_INT_H

#include <vstypes.h>

#define REVERB_MIX_MAX_SHIFT 10
#define REVERB_MIX_MAX   (1<<REVERB_MIX_MAX_SHIFT)

#define MAX_DELAYS_PER_CHANNEL 8

#ifndef ASM

struct Delay {
  s_int16 *p;		/* 0:   Current pointer. */
#ifdef __VSDSP__
  u_int16 mod;		/* 1:   Pointer modifier. */
#else
  s_int16 *endP;	/*      End pointer. */
#endif
  s_int16 *startP;	/* 2:   Start pointer. */
  s_int16 gain[2];	/* 3-4: FIR multipliers, scale -32768=-1, 32767=1. */
  u_int16 len;		/* 5:   Delay length. */
};


struct RoomReverb {
  /* These fields prefilled by the caller of DesignRoom() */
  u_int16 firstReflectionMS; /* How many milliseconds before first reflection */
  u_int16 roomSizeCM;	/* Room size in cm */
  u_int16 reverbTimeMS;	/* Room reverb time in ms = -60 dB attenuation time */
  u_int16 softness;	/* Room softness, 0 = hard, 65535 = soft */
  u_int32 sampleRate;	/* sample rate used for making room design */
  s_int16 dryGain;	/* 1024 = 1 */
  s_int16 wetGain;	/* 1024 = 1 */
  s_int16 unused1;	/* To make partially compatible with Room23 */
  s_int16 unused2;	/* To make partially compatible with Room23 */
  u_int32 unused3;	/* To make partially compatible with Room23 */
  /* Rest of the fields filled in by the room designer */
  s_int16 delays;	/* How many delays per channel used */
  s_int16 wetGainAdjusted; /* Same scale as for wetGain */
  s_int16 guardBits;	/* Guard bits for calculation */
  s_int16 guardBitsBack;	/* Guard bits for calculation */
  struct Delay delay[2*MAX_DELAYS_PER_CHANNEL];
  s_int16 __mem_y *firstReflectionDelay;
};

struct Delay2 {
  u_int16 hasPrediction;
  u_int32 filePos;
  u_int32 currPos;
  u_int32 nextPos;
  u_int32 totalLen;
  u_int16 firstRound;
};

struct RoomReverb23 {
  /* These fields prefilled by the caller of DesignRoom23() */
  u_int16 firstReflectionMS; /* How many milliseconds before first reflection */
  u_int16 roomSizeCM;	/* Room size in cm */
  u_int16 reverbTimeMS;	/* Room reverb time in ms = -60 dB attenuation time */
  u_int16 softness;	/* Room softness, 0 = hard, 65535 = soft */
  u_int32 sampleRate;	/* sample rate used for making room design */
  s_int16 dryGain;	/* 1024 = 1 */
  s_int16 wetGain;	/* 1024 = 1 */
  s_int16 (*Read )(register       u_int16 *d, register u_int32 addr, register u_int16 words); /* Function to read from external memory */
  s_int16 (*Write)(register const u_int16 *d, register u_int32 addr, register u_int16 words); /* Function to write to external memory */
  u_int32 memSizeWords; /* External memory size */
  /* Rest of the fields filled in by the room designer */
  s_int16 delays;	/* How many delays per channel used */
  s_int16 wetGainAdjusted; /* Same scale as for wetGain */
  s_int16 guardBits;	/* Guard bits for calculation */
  s_int16 guardBitsBack;	/* Guard bits for calculation */
  struct Delay delay[2*MAX_DELAYS_PER_CHANNEL];
  struct Delay2 delay2[2*MAX_DELAYS_PER_CHANNEL];
};

void Reverb(register __i2 struct RoomReverb __mem_y *room,
	    register __i0 s_int16 *data, register __a0 u_int16 stereoSamples);

void Reverb23(register __i2 struct RoomReverb23 __mem_y *room,
	      register __i0 s_int16 *data, register __a0 u_int16 stereoSamples);

struct ReverbCoreParam {
  u_int16 stereoSamples;	/* 0 */
  s_int16 *data;		/* 1 */
  struct Delay __mem_y *delay;	/* 2 */
  u_int16 delaysPerChannelM1;	/* 3 */
  u_int16 mainMix;		/* 4 */
  u_int16 reverbMix;		/* 5 */
  u_int16 shiftUp;		/* 6 */
  u_int16 shiftBack;		/* 7 */
};

extern struct ReverbCoreParam reverbCoreParam;

void ReverbCore(register __i0 struct ReverbCoreParam *r);
int DesignRoom(register struct RoomReverb __mem_y *room);
void FreeRoom(register struct RoomReverb __mem_y *room,
	      register u_int16 alreadyHasMutex);

int DesignRoom23(register struct RoomReverb23 __mem_y *room);
void FreeRoom23(register struct RoomReverb23 __mem_y *room,
		register u_int16 alreadyHasMutex); /* Always set alreadyHasMutex = 0 */

void CopyShiftRight(register __i0 s_int16 *d, register __i1 s_int16 *s,
		    register __a0 s_int16 n, register __a1 s_int16 shiftRight);

s_int16 Conv32To16F(register __reg_c s_int32 x);

s_int32 Conv16FTo32(register __c0 s_int16 x);

s_int32 Multip16FWithFract16(register __c0 s_int16 x, register __c1 s_int16 fr);

#ifdef __VSDSP__
s_int16 ShiftLeftAndSat16(register __reg_c s_int32 x, register __d0 shiftLeft);
#endif

#ifndef __VSDSP__
__near unsigned short LongLog2(register __reg_a long x);
#endif

u_int16 Exp(register __reg_a s_int32 x);

#endif /* !ASM */

#endif /* !REVERB_INT_H */
