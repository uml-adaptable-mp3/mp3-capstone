#include <vo_stdio.h>
#include <math.h>
#include <exec.h>
#include <imem.h>
#include "fm_function.h"

// Function sweeps the FM frequencies, calculates IQ 
// values and returns the number of found channels.
// fstart  =start frequency
// dir     =search direction up/down (1=up)
// chanptr =pointer to found channel frequencies
// chanmax =maximum number of channels to be searched 

#if 0
#define DEBUG_PRINT
#endif


#define SCAN_STATUS 1

#if SCAN_STATUS
//#include <uimessages.h>
#include "fmMessages.h"

#include "fmModel.h"

extern u_int16 rdsDebug;

extern void (*fmCallbackFunction)(s_int16 index, u_int16 message, u_int32 value);


#endif


#define FM_TUNE_LOOPS 4096
/* Tune FM frequency by examining audio DC offset. */
u_int32 FmTune(register u_int32 freq) {
  u_int32 newFreq;
  s_int16 i;
  s_int16 oldDiff, newDiff = 0x7FFF;

  newFreq = freq;

  do {
    s_int32 sum = 0.0;

    if (rdsDebug) {
      printf("Tune %6ld ", newFreq);
    }
  
    for (i=-2048/FM_AUDIO_BUFSIZE; i<FM_TUNE_LOOPS/FM_AUDIO_BUFSIZE; i++) {
      s_int16 j, *p = fmAudioBuf;

      fread(fmAudioBuf, sizeof(s_int16), FM_AUDIO_BUFSIZE, stdaudioin);
#if 0
      fwrite(fmAudioBuf, sizeof(s_int16), FM_AUDIO_BUFSIZE, stdaudioout);
#endif
      if (i >= 0) {
	for (j=0; j<FM_AUDIO_BUFSIZE; j++) {
	  sum += *p++;
	}
      }
    }
    
    if (rdsDebug) {
      printf("mean %8.2f -> ",
	     sum*(0.5/FM_TUNE_LOOPS));
    }
    /* Set to 0 kHz intentional offset */
    newFreq -= floor(sum*(0.5/FM_TUNE_LOOPS/88.0)+0.5) + 0;
    if (rdsDebug) {
      printf("%6ld. ", newFreq);
    }

    if (freq != newFreq) {
      SetFMFreq(newFreq);
    }

    oldDiff = newDiff;
    newDiff = (u_int16)labs(freq-newFreq);
    if (rdsDebug) {
      printf("  oDiff %04x, nDiff %04x\n", oldDiff, newDiff);
    }
    freq = newFreq;
  } while (newDiff && newDiff < oldDiff);

  return newFreq;
}


#define FM_NEXT_SIGNAL_STRENGTH_ABSOLUTE_LIMIT 400 /* In 1/10th of a decibel */
#define FM_NEXT_THRESHOLD 40 /* How high a signal level peak there must be, in 1/10th of a decibel */
u_int32 FmNext(register u_int32 oldFreq, register u_int16 backwards) {
  u_int16 fiveLast[5] = {32767, 32767, 32767, 32767, 32767};
  u_int16 stepsLeft = (FM_HIGH-FM_LOW)/FM_STEP+4;

  /* Remove any potentially remaining channel tuning:
     round old frequency to closest FM_STEP. */
  oldFreq += FM_STEP/2;
  oldFreq = oldFreq - oldFreq%FM_STEP;

  do {
    u_int16 okFreq = 0;
    oldFreq += backwards ? -FM_STEP : FM_STEP;
    if (oldFreq < FM_LOW - 2*FM_STEP) {
      oldFreq = FM_HIGH + 2*FM_STEP;
      memset(fiveLast, 32767, 5);
    } else if (oldFreq > FM_HIGH + 2*FM_STEP) {
      oldFreq = FM_LOW - 2*FM_STEP;
      memset(fiveLast, 32767, 5);
    }
    SetFMFreq(oldFreq);
    memmove(fiveLast, fiveLast+1, 4*sizeof(fiveLast[0]));
    // DC-removal filter/vco needs some time to settle
    Delay(50);
    fiveLast[4] = FmCalcIQ();
    if ((fiveLast[2] > fiveLast[3]+FM_NEXT_THRESHOLD ||
	 fiveLast[2] > fiveLast[4]+FM_NEXT_THRESHOLD) && 
	(fiveLast[2] > fiveLast[1]+FM_NEXT_THRESHOLD ||
	 fiveLast[2] > fiveLast[0]+FM_NEXT_THRESHOLD) &&
	fiveLast[2] > FM_NEXT_SIGNAL_STRENGTH_ABSOLUTE_LIMIT) {
      if (fiveLast[2] >= fiveLast[1] && fiveLast[2] >= fiveLast[3]) {
	okFreq = 2;
      }
    }

    if (rdsDebug) {
      printf("f %7.2f MHz, five = %6u %6u %6u %6u %6u %c\n",
	     oldFreq*(1.0/1000.0),
	     fiveLast[0], fiveLast[1], fiveLast[2], fiveLast[3], fiveLast[4],
	     okFreq ? '0'+okFreq : ' ');
    }

    if (okFreq) {
#if 0
      printf("Returning %ld %d %d %d\n", oldFreq, okFreq, backwards, FM_STEP);
#endif
      oldFreq += (s_int32)okFreq*(backwards ? FM_STEP : -FM_STEP);
#if 0
      printf("Returning %ld\n", oldFreq);
#endif
      SetFMFreq(oldFreq);
      return oldFreq;
    }
  } while (--stepsLeft);

  return 0;
}



/* Calculate IQ signal strength for a channel.
   Returns an integer with unit 1/10 dB, e.g. 680 is 6 dB louder than 620.
   Note: Maximum non-clipping signal gives a result of 872 (87.2 dB).
   Recommendation is to switch to lower analog gain if result is >= 800. */
#define CALC_IQ_SAMPLES 4096
u_int16 FmCalcIQ(void) {
  u_int32 acc_hi=0, acc_lo=0;
  double dAcc;
  s_int16 res;
  int i;
  u_int16 fmIntHi, fmIntLo;

  Forbid();
  Disable();
  fmIntHi = PERIP(INT_ENABLE1_HP) & ~INTF1_FM;
  fmIntLo = PERIP(INT_ENABLE1_LP) & ~INTF1_FM;
  PERIP(INT_ENABLE1_HP) = fmIntHi;
  PERIP(INT_ENABLE1_LP) = fmIntLo;
  Enable();

  for (i=0; i<CALC_IQ_SAMPLES; i++) {
    u_int32 t32;
    s_int16 ii, qq;
    // 187.5/192kHz interrupt flag
    while (!(PERIP(INT_ORIGIN1) & INTF1_FM))
      ;
    PERIP(INT_ORIGIN1) = INTF1_FM;

    // Get DC-removed IQ samples
    ii = PERIP(FM_I_RDC);
    qq = PERIP(FM_Q_RDC);
    t32 = acc_lo + (s_int32)ii*ii + (s_int32)qq*qq;
    if (t32 < acc_lo) {
      acc_hi++;
    }
    acc_lo = t32;
  }

  Disable();
  PERIP(INT_ENABLE1_HP) |= fmIntHi & INTF1_FM;
  PERIP(INT_ENABLE1_LP) |= fmIntLo & INTF1_FM;
  Enable();
  Permit();

  dAcc = acc_hi*(65536.0*65536.0);
  dAcc += acc_lo;
  dAcc /= 2.0*CALC_IQ_SAMPLES;
  dAcc = 100.0*log(dAcc)/log(10.0); /* Return scale is in 1/10th dB */

  res = (s_int16)dAcc;
  if (res < 0) {
    res = 0;
  }

#if 0
  printf("dAcc %7.2f, acc_hi %08lx, acc_lo %08lx, res %6d\n",
	 dAcc, acc_hi, acc_lo, res);
#endif

  return (u_int16)res;
}




extern u_int16 endPhComp;


u_int32 i_amp;
u_int32 q_amp;
u_int32 i_ph;
u_int32 q_ph;

u_int32 i_amp_avg;
u_int32 q_amp_avg;
u_int32 i_ph_avg;
u_int32 q_ph_avg;

u_int16 aRound;
u_int16 bRound;


// Calculate IQ-signal amplitude/phase errors
// Own modification: also sets calculated value and enable correction

#define PH_COMP_ROUNDA_BITS 13
#define PH_COMP_ROUNDA_SAMPLES (1<<PH_COMP_ROUNDA_BITS)
#define PH_COMP_ROUNDB_BITS  7
#define PH_COMP_ROUNDB_SAMPLES (1<<PH_COMP_ROUNDB_BITS)


void FmIntC(void) {
  // obtain samples and calculate
  register s_int16 i_smp = (s_int16)PERIP(FM_I_RDC);
  register s_int16 q_smp = (s_int16)PERIP(FM_Q_RDC);
  // IQ-amplitude error
  i_amp += labs((s_int32)i_smp);
  q_amp += labs((s_int32)q_smp);
  // IQ-phase error (45 degree turn)
  i_ph  += labs((s_int32)i_smp-(s_int32)q_smp);
  q_ph  += labs((s_int32)i_smp+(s_int32)q_smp);

  if (++aRound >= PH_COMP_ROUNDA_SAMPLES) {
    i_amp_avg += i_amp >> PH_COMP_ROUNDA_BITS;
    q_amp_avg += q_amp >> PH_COMP_ROUNDA_BITS;
    i_ph_avg  += i_ph >> PH_COMP_ROUNDA_BITS;
    q_ph_avg  += q_ph >> PH_COMP_ROUNDA_BITS;

    i_amp = 0;
    q_amp = 0;
    i_ph  = 0;
    q_ph  = 0;

    aRound = 0;
		
    if (++bRound >= PH_COMP_ROUNDB_SAMPLES) {
      // generate result
      endPhComp = 1;
      // end interrupt for now
      PERIP(INT_ENABLE1_LP) &= ~(INTF1_FM);
    }
  }
}

extern u_int16 FmIntVector;
u_int16 iqCompDisable = 0;

void RunIQComp(void) {
  // disable phcomp
  PERIP(FM_CF) &= ~FM_CF_PHCOMP;

  if (!iqCompDisable) {
    // initalize
    endPhComp = 0;
    i_amp = 0;
    q_amp = 0;
    i_ph  = 0;
    q_ph  = 0;
	
    i_amp_avg = 0;
    q_amp_avg = 0;
    i_ph_avg  = 0;
    q_ph_avg  = 0;
	
    aRound = 0;
    bRound = 0;

    /* Shut down RDS because there isn't enough CPU */
    PERIP(INT_ENABLE0_LP) &= ~INTF_MAC0;
    PERIP(INT_ENABLE0_HP) &= ~INTF_MAC0;
    
    // Set interrupt active
	
    WriteIMem(0x20+INTV_FM, ReadIMem((u_int16)(&FmIntVector)));
    PERIP(INT_ENABLE1_LP) |= INTF1_FM;
  }
}

void CloseIQComp(void) {
	double amp_err = 0;
	double ph_err = 0;
	u_int16 q_fix = 0;
	u_int16 i_fix = 0;
	
	/* Restart RDS */
	PERIP(INT_ENABLE0_LP) |= INTF_MAC0;
	PERIP(INT_ENABLE0_HP) |= INTF_MAC0;

	if(endPhComp == 0) {
		// This can result from shutting down busy algorithm run
		// end interrupt for now
		PERIP(INT_ENABLE1_LP) &= ~(INTF1_FM);
		return;
	}
	endPhComp = 0;
	amp_err = (double)i_amp_avg/(double)q_amp_avg;
	ph_err  = (double)i_ph_avg/(double)q_ph_avg;
	// Set amplitude/phase error compensation registers
	// v2: (x+384)/512
	q_fix = (u_int16)(amp_err*512-384);    // amp error to Q-scale
	i_fix = (u_int16)((1/ph_err)*512-384); // ph error to I-scale
	
	// sanity check, do not try to fix error > 10%
	if ((q_fix < 77) || (q_fix > 205)) {
//		printf("Amplitude error exceeds limits, using defaults\n");
		q_fix = 120;
	}
	if ((i_fix < 77) || (i_fix > 205)) {
//		printf("Phase error exceeds limits, using defaults\n");
		i_fix = 136;
	}
	//	printf("INT: Amp fix: %i Phase fix: %i\n", q_fix, i_fix);
	
	PERIP(FM_CF)     |= FM_CF_PHCOMP;  // enable phcomp
	// 	PERIP(FM_CTRL)     &= ~(1<<7); // disable phcomp
	PERIP(FM_PHSCL) = (i_fix<<8) | q_fix;
	bRound = 0;
	aRound = 0;
	if (rdsDebug) {
	  printf("PH_COMP done, Amp fix: %i Phase fix: %i\n", q_fix, i_fix);
	}
#ifdef DEBUG_PRINT
	printf("PH_COMP done, Amp fix: %i Phase fix: %i\n", q_fix, i_fix);
#endif
}
