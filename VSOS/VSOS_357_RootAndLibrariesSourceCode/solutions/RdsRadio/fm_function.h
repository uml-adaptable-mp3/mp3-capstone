#ifndef FM_FUNCTION_H
#define FM_FUNCTION_H

#include <vo_stdio.h>
#include <vs1005h.h>
#include <string.h>
#include <stdlib.h>
#include <timers.h>
#include "fm_init.h"

#include "fmModel.h"

#define FMCHANS 16
#define SEARCH_OFFSET 100 // current channel avoidance in search next/prev, in kHz

u_int32 FmNext(register u_int32 oldFreq, register u_int16 backwards);
u_int32 FmTune(register u_int32 freq);

/* Calculate IQ signal strength for a channel.
   Returns an integer with unit 1/10 dB, e.g. 680 is 6 dB louder than 620.
   Note: Maximum non-clipping signal gives a result of 872 (87.2 dB).
   Recommendation is to switch to lower analog gain if result is >= 800. */
u_int16 FmCalcIQ(void);


auto void LongLongMac(register __i0 struct longlong *acc,
                      register __a s_int32 a, register __b s_int32 b);

auto double LongLongToDouble(register __i0 struct longlong *acc);


// Calculate IQ-signal amplitude/phase errors
// Own modification: also sets calculated value and enable correction
void FMIQErrSet(void);


void RunIQComp(void);
void CloseIQComp(void);
void FmIQRound(void);

extern u_int16 iqCompDisable;

#endif
