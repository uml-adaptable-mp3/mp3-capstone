
/*************************************************
 *
 * VS1005 FM-radio/RDS test code
 * 
 *************************************************/


#include <vo_stdio.h>
#include <vsos.h>
#include <vs1005h.h>
#include <stdlib.h>
#include <apploader.h>
#include <clockspeed.h>
#include "rf_clock.h"

/// Configures FM hardware, sets DAC sample rate etc.
/// \todo Integrate this into the upcoming VSOS audio API.
void InitRf(void) {
	/* InitAudio() that was called earlier already set up many registers when AID_FM was selected. */

	// pll factor(12.288MHz) = 53127850.57   ~=  53127850 = 0x032A AAAA
	// pll factor(13.0MHz)   = 50218079.06   ~=  50218079 = 0x02FE 445F
	// pll factor(12.0MHz)   = 54402918.98   ~=  54402918 = 0x033E 1F66
	PERIP32(FMPLL_LO) =  652835027804160.0/clockSpeed.peripClkHz;
}


u_int32 fmBoundary[2] = {0,0}; /*86000L,104500L*/


u_int16 SetRfFreq(register u_int32 fmband) {
  u_int16 anacf3  = 0;
  u_int16 vcolck  = 0;
  u_int16 locks   = 0;
  u_int16 cntrlck[3];
  s_int16 cntr    = 0;
  s_int32 sdm     = 0;
  u_int32 fract   = 0;
  u_int32 vcomult = 0;
  u_int32 vcofreq = 0;
  u_int16 vco_div = 0;
  u_int16 fm_div  = 0;
  u_int16 fm_div_bits = 0;
  u_int16 clk = 12288;
  s_int32 fvco;
  u_int16 vcodiv;
  s_int32 sdmtmp  = 0;

#if 0
  printf("SetFMFreq(%ld) ", fmband);
#endif

  if (!fmBoundary[0]) {
    s_int16 i;
    fmBoundary[1] = 120000;

    for (i=0; i<2; i++) {
      u_int32 lo, hi;
      u_int32 startT, t;

      startT = i ? 105000 : 85000;
      fmBoundary[i] = startT - 40000;
      
      lo = hi = t = startT;
      while (SetRfFreq(t-=500)) {
	lo = t;
#if 0
	printf("##1 %ld\n", t);
#endif
      }
      t = startT;
#if 0
      fmBoundary[i] = startT + 40000;
      while (SetFMFreq(t+=500)) {
	hi = t;
#if 0
	printf("##2 %ld\n", t);
#endif
      }
      fmBoundary[i] = (lo+hi)/2;
#else
      fmBoundary[i] = lo+500;
#endif
      
#if 0
      printf("####3 %ld\n", fmBoundary[i]);
#endif
    }
  }

#if 0
  if (fmband < FM_LOW-2*FM_STEP || fmband > FM_HIGH+2*FM_STEP) {
    printf("ERROR: bad input frequency (%lu)\n", fmband);
    return 0;
  }
#endif

  PERIP(FM_CF) &= ~(FM_CF_PHCOMP); // disable phcomp
	
  // NOTE: assuming FM mode, all power etc. ON
  cntrlck[0] = 0;
  cntrlck[1] = 0;
  cntrlck[2] = 0;
  sdm = 0;

  if (fmband < fmBoundary[0]) {        // about 86000L
    fm_div = 24;
    fm_div_bits = 3;    // 1 and 3 should be same thing "(x)1"
  } else if (fmband > fmBoundary[1]) {   // about 103000L
    fm_div = 16;
    fm_div_bits = 0;
  } else {
    fm_div = 20;
    fm_div_bits = 2;
  }


  vcomult = (fmband * (long)fm_div) / clk;
	
  clk = (clk>>3);   // assuming xtal is x*1kHz so no rounding errors
  vcofreq =  fmband * (long)fm_div * (0x00008000L>>6); // scale to fit 32 bits
  if (vcomult > 144L) {
    fract    = (0xFFB80000L>>3);       // -144*0x00008000
    vco_div  = 0;
    vcodiv   = 36;
  } else {  // else if (vcomult > 120L) {
    fract    =  (0xFFC40000L>>3);      // -120*0x00008000
    vco_div  = 2;
    vcodiv   = 30;
  }

#if 0
  printf("  fm_div %d, vcodiv %d, vcomult %ld, ", fm_div, vcodiv, vcomult);
#endif
  
  fract = clk*fract + vcofreq;
	
  fract = fract*2;
  //  calculating: fract = 256 * fract/clk    // 4*8*8
  {
    u_int32 tmpfract, error, tmpfract2;
    tmpfract = fract/clk;    // 99669.33333
    error    = fract - (clk * tmpfract);
    error    = (error<<16);
    tmpfract2 = error / clk;
    tmpfract2 = (tmpfract2>>(16-8));
    fract = (tmpfract<<8) + tmpfract2;
  }
	
  sdm = (fract ^ 0x02000000) & 0x03FFFFFF;
  {
    u_int16 ccf = (u_int16)(fract>>21);
#if 0
    printf("sdm %08lx, ccf %d, ", sdm, ccf);
#endif
    /* Limit the CCD according to datasheet rules */
    if (ccf >= vcodiv-2 || ccf >= 31) {
#if 0
      printf("BAD CCF %d, vcodiv %d!\n", ccf, vcodiv);
#endif
      vcolck = 0;
      goto finally;
    }
  }
  
  // find working cntr area
  PERIP(FMCCF_HI) = 0;
  PERIP(FMCCF_LO) = 0;
  PERIP(FMCCF_HI)  = (u_int16)(sdm>>16);
  PERIP(FMCCF_LO)  = (u_int16) sdm;
	
	
  locks = 0; // no locks yet
  //
  // 0xF is the slowest, 0x0 is the fastest cntr freq area
  //
  anacf3 = PERIP(ANA_CF3) & ~((3*ANA_CF3_FMDIV) | (3*ANA_CF3_DIV) |
			      ANA_CF3_2GCNTR_MASK);
  anacf3 |= fm_div_bits*ANA_CF3_FMDIV | vco_div*ANA_CF3_DIV;
  PERIP(ANA_CF3) = anacf3;
  // Would be nice to have some kind of starting guess...
  for (cntr=15; cntr>=0; cntr--) {
    // FMdiv[13:12], div[11:10], cntr[3:0]
    anacf3 &= ~ANA_CF3_2GCNTR_MASK;
    anacf3 |= cntr*ANA_CF3_2GCNTR;
    PERIP(ANA_CF3) = anacf3;
    //Delay(1);//delay_n(100000);
    vcolck  = CheckRfLock(0);           // param: printena
#if 0
    printf("%d", vcolck);
#endif
    if (vcolck == 1) {
      cntrlck[locks] = cntr;
      if (++locks == 3) {
	break;
      }
      // printf() SARChanAvg(3, 8) // chan=3, cnt=8
    } else if (locks) { // 3:43
      // max possible is 3 locks, if two locks found and
      // then no lock => also break (there can not be any more locks)
      break;
    }
  }
#if 0
  printf("\n");
#endif
  if (locks == 0) {
    Delay(1);//delay_ms(100);  // add delay, SEEMS TO BE VERY IMPORTATN DELAY...
  }
  
  // decide which cntr area to use
  anacf3 &= ~ANA_CF3_2GCNTR_MASK;
  if (locks == 3) {
    anacf3 |= cntrlck[1];       // middle
  } else if (locks == 1) {
    anacf3 |= cntrlck[0];       // first and only one
  } else { // 2 locks
    // => try to find best one by checking locks around the band
    s_int16 j, k;
    u_int16 lckstat[2] = {0,0};

    
    // FIGURE BEST CNTR AREA
    // slower area => check upper side locks
    // check + n*100kHz for slower cntr area
    // printf("    SEARCH %x  ++\n", cntrlck[0]);
    for (j=0; j<2; j++) {
      vcolck = 1;
      for (k=1; k<=5 && vcolck; k++) {           // loop freq offset *100kHz
	anacf3 &= ~ANA_CF3_2GCNTR_MASK;
	anacf3 |= cntrlck[j];          // 1st
	PERIP(ANA_CF3)  = anacf3;
	sdmtmp = sdm + (j?-k:k) * 0x00055550L;
	//	printf("j%d k%d sdm %08lx, sdmtmp %08lx\n", j, k, sdm, sdmtmp);
	PERIP(FMCCF_HI) = 0;
	PERIP(FMCCF_LO) = 0;
	PERIP(FMCCF_HI) = (u_int16)(sdmtmp>>16);
	PERIP(FMCCF_LO) = (u_int16)sdmtmp;
	vcolck = CheckRfLock(0);           // param: printena
	lckstat[j] += vcolck;           // param: printena
      }
    }
#if 0
    printf("   lckstat: %x %x, ", lckstat[0], lckstat[1]);
#endif
    PERIP(FMCCF_HI) = 0;
    PERIP(FMCCF_LO) = 0;
    PERIP(FMCCF_HI) = (u_int16)(sdm>>16);
    PERIP(FMCCF_LO) = (u_int16)sdm;
    anacf3 &= ~ANA_CF3_2GCNTR_MASK;
    anacf3 |= cntrlck[lckstat[1]>lckstat[0]];
  } /* If 2 locks */
#if 0
  printf("  ANA_CF3 = %04x\n", anacf3);
#endif
  
  PERIP(ANA_CF3) = anacf3;
  if (locks == 0) {
    PERIP(ANA_CF3) |= ANA_CF3_2GCNTR_MASK;
  }
  
  //Delay(1); //delay_n(1000);
  vcolck  = CheckRfLock(0);                   // param: printena
  // {
  //   modu = ( sdm / 2097152L) + 16;
  //   fvco = (4*vcodiv + modu) * 12e-3;        // scale to GHz
  // }
 finally:
  return vcolck;
}



u_int16 CheckRfLock(register u_int16 printena) {
	u_int16 lockstat;
	PERIP(ANA_CF0) |= ANA_CF0_LCKCHK; // 0x0008;    // lock set high
	Delay(1); // actually only 20us needed
	
	PERIP(ANA_CF0) &= ~ANA_CF0_LCKCHK;
	
	Delay(1);// wait some time to see quality of vco lock
				// actually only 1us needed
	lockstat = (PERIP(ANA_CF0) >> ANA_CF0_LCKST_B) & 0x0001;
	if (printena) {
		printf("   RF vco lck: %d\n", lockstat);
		fflush (stdout);
	}
	return lockstat;              // '1' when locked
}






