#ifndef __LIB_AGC_H__
#define __LIB_AGC_H__

#include <vsos.h>

#define LibInitSubsonic(lib, subsonicM, channels, subsonicG, sampleRate) ( ((void (*)(register __i0 struct SubsonicM *, register __a0 s_int16, register __i1 struct SubsonicG __mem_y *, register __reg_d u_int32))(*((u_int16 *)(lib)+2+(ENTRY_1))))((subsonicM), (channels), (subsonicG), (sampleRate)) )

#define LibSubsonic(lib, subsonicM, subsonicG, d, n) ( ((void (*)(register __i0 struct SubsonicM *, register __i1 struct SubsonicG __mem_y *, register __i2 s_int16 *, register __a0 u_int16))(*((u_int16 *)(lib)+2+(ENTRY_2))))((subsonicM), (subsonicG), (d), (n)) )

#define LibInitAgc(lib, agc, channels, agcConsts, maxGain, sampleRate) ( ((void (*)(register struct Agc *, register s_int16, register struct AgcConsts __mem_y *, register u_int16, register u_int32))(*((u_int16 *)(lib)+2+(ENTRY_3))))((agc), (channels), (agcConsts), (maxGain), (sampleRate)) )

#define LibAgc(lib, agc, agcConsts, d, n) ( ((void (*)(register __i0 struct Agc *, register __i1 __mem_y const struct AgcConsts *, register __i2 s_int16 *, register __a0 u_int16))(*((u_int16 *)(lib)+2+(ENTRY_4))))((agc), (agcConsts), (d), (n)) )

#endif /* !__LIB_AGC_H__ */
