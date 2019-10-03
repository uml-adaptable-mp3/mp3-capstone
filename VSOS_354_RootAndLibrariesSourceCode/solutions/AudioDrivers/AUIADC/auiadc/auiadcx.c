#include <vo_stdio.h>
#include <string.h>
#include <stdlib.h>
#include <aucommon.h>
#include <vs1005h.h>
#include <clockspeed.h>
#include "auiadc.h"






const u_int16 __mem_y adInts[AD_INTS] = {
  INTV_MAC0, INTV_MAC1, INTV_MAC2, INTV_I2S, INTV_SRC, INTV_SRC
};

const struct AudioIdPattern __mem_y audioIdPattern[] = {
  /* 0=I2S, 1=SPDIF */
  {AIF_STEREO,
   AIR_I2S, AD_INT_I2S,
   {{I2S_CF, I2S_CF_FS_BITS_MASK|I2S_CF_ENA, I2S_CF_FS48K32B},
    {I2S_CF, I2S_CF_FS_BITS_MASK|I2S_CF_ENA, I2S_CF_FS48K32B|I2S_CF_ENA}}},
  {AIF_STEREO,
   AIR_SPDIF, AD_INT_SPDIF,
   {{0}}},

  /* 2=DIA1, 3=DIA2, 4=DIA3 */
  {AIF_LEFT | AIF_CAN_USE_DEC6,
   AIR_MAC0, AD_INT_MAC0,
   {{0}}},
  {AIF_RIGHT | AIF_CAN_USE_DEC6,
   AIR_MAC0, AD_INT_MAC0,
   {{0}}},
  {AIF_CENTER | AIF_CAN_USE_DEC6,
   AIR_MAC2, AD_INT_MAC2,
   {{0}}},

  /* 5=FM */
  {AIF_STEREO | AIF_CAN_USE_DEC6 | AIF_NEEDS_DEC6,
   AIR_MAC0 | AIR_MAC1 | AIR_AD23, AD_INT_MAC0,
   {{AD_CF, AD_CF_DEC6SEL_MASK|AD_CF_AD23_FLP, AD_CF_DEC6SEL_FM},
    {ANA_CF3, ANA_CF3_GAIN2MASK|ANA_CF3_GAIN1MASK|ANA_CF3_2GCNTR_MASK, ANA_CF3_FMDIV24|(0x7<<ANA_CF3_GAIN2_B)|(0x7<<ANA_CF3_GAIN1_B)|(0x7<<ANA_CF3_2GCNTR_B)},
    {ANA_CF2, 0, ANA_CF2_LNA_ENA|ANA_CF2_2G_ENA|ANA_CF2_AMP1_ENA|ANA_CF2_AMP2_ENA},
    {ANA_CF0, ANA_CF0_M1LIN|ANA_CF0_M2LIN|ANA_CF0_M2MIC|ANA_CF0_M1MIC, ANA_CF0_M2FM|ANA_CF0_M1FM|ANA_CF0_LCKST},
    {FM_CF, 0, FM_CF_FM_ENA|FM_CF_ENABLE}}},

  /* 6=LINE1_1, 7=LINE2_1, 8=LINE3_1, 9=MIC1 */
  {AIF_LEFT | AIF_CAN_USE_DEC6,
   AIR_MAC0 | AIR_LINE1_1, AD_INT_MAC0,
   {{AD_CF, AD_CF_ADENA|AD_CF_ADFS_MASK, AD_CF_ADENA|AD_CF_ADFS_48K},
    {ANA_CF0, ANA_CF0_M1FM|ANA_CF0_M1LIN|ANA_CF0_M1MIC, ANA_CF0_M1LIN},
    {ANA_CF2, ANA_CF2_M1_ENA|ANA_CF2_AMP1_ENA, ANA_CF2_M1_ENA},
    {FM_CF,FM_CF_ENABLE|FM_CF_FM_ENA|FM_CF_UAD1, FM_CF_ENABLE}}},
  {AIF_LEFT | AIF_CAN_USE_DEC6,
   AIR_MAC0 | AIR_LINE3_1, AD_INT_MAC0,
   {{AD_CF, AD_CF_ADENA|AD_CF_ADFS_MASK, AD_CF_ADENA|AD_CF_ADFS_48K},
    {ANA_CF0, ANA_CF0_M1FM|ANA_CF0_M1LIN|ANA_CF0_M1MIC, ANA_CF0_M1FM|ANA_CF0_M1LIN},
    {ANA_CF2, ANA_CF2_M1_ENA|ANA_CF2_AMP1_ENA, ANA_CF2_M1_ENA},
    {FM_CF,FM_CF_ENABLE|FM_CF_FM_ENA|FM_CF_UAD1, FM_CF_ENABLE}}},
  {AIF_LEFT | AIF_CAN_USE_DEC6,
   AIR_MAC0 | AIR_LINE2_1, AD_INT_MAC0,
   {{AD_CF, AD_CF_ADENA|AD_CF_ADFS_MASK, AD_CF_ADENA|AD_CF_ADFS_48K},
    {ANA_CF0, ANA_CF0_M1FM|ANA_CF0_M1LIN|ANA_CF0_M1MIC, 0},
    {ANA_CF2, ANA_CF2_M1_ENA|ANA_CF2_AMP1_ENA, ANA_CF2_M1_ENA},
    {FM_CF,FM_CF_ENABLE|FM_CF_FM_ENA|FM_CF_UAD1, FM_CF_ENABLE}}},
  {AIF_LEFT | AIF_CAN_USE_DEC6,
   AIR_MAC0 | AIR_MIC1N | AIR_MIC1P, AD_INT_MAC0,
   {{AD_CF, AD_CF_ADENA|AD_CF_ADFS_MASK, AD_CF_ADENA|AD_CF_ADFS_48K},
    {ANA_CF0, ANA_CF0_M1FM|ANA_CF0_M1LIN|ANA_CF0_M1MIC, ANA_CF0_M1FM|ANA_CF0_M1MIC},
    {ANA_CF2, ANA_CF2_M1_ENA|ANA_CF2_AMP1_ENA, ANA_CF2_M1_ENA|ANA_CF2_AMP1_ENA},
    {ANA_CF3, ANA_CF3_GAIN1MASK, ANA_CF3_GAIN1_20DB},
    {FM_CF,FM_CF_ENABLE|FM_CF_FM_ENA|FM_CF_UAD1, FM_CF_ENABLE}}},

  /* 10=LINE1_2, 11=LINE2_2, 12=LINE3_2, 13=MIC2 */
  {AIF_RIGHT | AIF_CENTER_FLIPPED | AIF_CAN_USE_DEC6,
   AIR_MAC0 | AIR_LINE1_1 | AIR_AD23, AD_INT_MAC0,
   {{AD_CF&0x7FFF, AD_CF_ADENA|AD_CF_ADFS_MASK, AD_CF_ADENA|AD_CF_ADFS_48K},
    {AD_CF       , AD_CF_AD3ENA|AD_CF_AD3FS_MASK, AD_CF_AD3ENA|AD_CF_AD3FS_48K},
    {ANA_CF0, ANA_CF0_M2FM|ANA_CF0_M2LIN|ANA_CF0_M2MIC, ANA_CF0_M2LIN},
    {ANA_CF2, ANA_CF2_M2_ENA|ANA_CF2_M3_ENA|ANA_CF2_AMP2_ENA,
     ANA_CF2_M2_ENA|ANA_CF2_M3_ENA},
    {FM_CF&0x7FFF,FM_CF_ENABLE|FM_CF_FM_ENA|FM_CF_UAD3, FM_CF_ENABLE},
    {FM_CF       ,FM_CF_ENABLE|FM_CF_FM_ENA|FM_CF_UAD2, FM_CF_ENABLE}}},
  {AIF_RIGHT | AIF_CENTER_FLIPPED | AIF_CAN_USE_DEC6,
   AIR_MAC0 | AIR_LINE3_1 | AIR_AD23, AD_INT_MAC0,
   {{AD_CF&0x7FFF, AD_CF_ADENA|AD_CF_ADFS_MASK, AD_CF_ADENA|AD_CF_ADFS_48K},
    {AD_CF       , AD_CF_AD3ENA|AD_CF_AD3FS_MASK, AD_CF_AD3ENA|AD_CF_AD3FS_48K},
    {ANA_CF0, ANA_CF0_M2FM|ANA_CF0_M2LIN|ANA_CF0_M2MIC, ANA_CF0_M2FM|ANA_CF0_M2LIN},
    {ANA_CF2, ANA_CF2_M2_ENA|ANA_CF2_M3_ENA|ANA_CF2_AMP2_ENA,
     ANA_CF2_M2_ENA|ANA_CF2_M3_ENA},
    {FM_CF&0x7FFF,FM_CF_ENABLE|FM_CF_FM_ENA|FM_CF_UAD3, FM_CF_ENABLE},
    {FM_CF       ,FM_CF_ENABLE|FM_CF_FM_ENA|FM_CF_UAD2, FM_CF_ENABLE}}},
  {AIF_RIGHT | AIF_CENTER_FLIPPED | AIF_CAN_USE_DEC6,
   AIR_MAC0 | AIR_LINE2_1 | AIR_AD23, AD_INT_MAC0,
   {{AD_CF&0x7FFF, AD_CF_ADENA|AD_CF_ADFS_MASK, AD_CF_ADENA|AD_CF_ADFS_48K},
    {AD_CF       , AD_CF_AD3ENA|AD_CF_AD3FS_MASK, AD_CF_AD3ENA|AD_CF_AD3FS_48K},
    {ANA_CF0, ANA_CF0_M2FM|ANA_CF0_M2LIN|ANA_CF0_M2MIC, 0},
    {ANA_CF2, ANA_CF2_M2_ENA|ANA_CF2_M3_ENA|ANA_CF2_AMP2_ENA,
     ANA_CF2_M2_ENA|ANA_CF2_M3_ENA},
    {FM_CF&0x7FFF,FM_CF_ENABLE|FM_CF_FM_ENA|FM_CF_UAD3, FM_CF_ENABLE},
    {FM_CF       ,FM_CF_ENABLE|FM_CF_FM_ENA|FM_CF_UAD2, FM_CF_ENABLE}}},
  {AIF_RIGHT | AIF_CENTER_FLIPPED | AIF_CAN_USE_DEC6,
   AIR_MAC0 | AIR_MIC2N | AIR_MIC2P | AIR_AD23, AD_INT_MAC0,
   {{AD_CF, AD_CF_ADENA|AD_CF_ADFS_MASK, AD_CF_ADENA|AD_CF_ADFS_48K},
    {ANA_CF0, ANA_CF0_M2FM|ANA_CF0_M2LIN|ANA_CF0_M2MIC, ANA_CF0_M2FM|ANA_CF0_M2MIC},
    {ANA_CF2, ANA_CF2_M2_ENA|ANA_CF2_AMP2_ENA, ANA_CF2_M2_ENA|ANA_CF2_AMP2_ENA},
    {ANA_CF3, ANA_CF3_GAIN2MASK, ANA_CF3_GAIN2_20DB},
    {FM_CF&0x7FFF,FM_CF_ENABLE|FM_CF_FM_ENA|FM_CF_UAD3, FM_CF_ENABLE},
    {FM_CF,       FM_CF_ENABLE|FM_CF_FM_ENA|FM_CF_UAD2, FM_CF_ENABLE}}},

  /* 14=LINE1_3 */
  {AIF_CENTER | AIF_RIGHT_FLIPPED | AIF_CAN_USE_DEC6,
   AIR_MAC0 | AIR_LINE1_3 | AIR_AD23, AD_INT_MAC0,
   {{AD_CF&0x7FFF, AD_CF_ADENA|AD_CF_ADFS_MASK, AD_CF_ADENA|AD_CF_ADFS_48K},
    {AD_CF       , AD_CF_AD3ENA|AD_CF_AD3FS_MASK, AD_CF_AD3ENA|AD_CF_AD3FS_48K},
    {ANA_CF0, ANA_CF0_M2FM|ANA_CF0_M2LIN|ANA_CF0_M2MIC, ANA_CF0_M2LIN},
    {ANA_CF2, ANA_CF2_M2_ENA|ANA_CF2_M3_ENA|ANA_CF2_AMP2_ENA,
     ANA_CF2_M2_ENA|ANA_CF2_M3_ENA},
    {FM_CF&0x7FFF,FM_CF_ENABLE|FM_CF_FM_ENA|FM_CF_UAD3, FM_CF_ENABLE},
    {FM_CF       ,FM_CF_ENABLE|FM_CF_FM_ENA|FM_CF_UAD2, FM_CF_ENABLE}}},

  /* 15=DEC6 */
  {AIF_STEREO | AIF_IS_DEC6,
   AIR_MAC1, AD_INT_MAC1,
   {{AD_CF,AD_CF_DEC6SEL_MASK|AD_CF_DEC6ENA,AD_CF_DEC6ENA|AD_CF_DEC6SEL_STEREO}}},
};

const u_int16 __mem_y adInts[AD_INTS] = {
  INTV_MAC0, INTV_MAC1, INTV_MAC2, INTV_I2S, INTV_SRC, INTV_SRC
};

#define ADC_RATES 4

struct SetAdc {
  u_int32 nominalRate;
  u_int16 reg;
  u_int16 reg3;
} const __mem_y setAdc[ADC_RATES] = {
  { 24000, AD_CF_ADFS_24K,  AD_CF_AD3FS_24K},
  { 48000, AD_CF_ADFS_48K,  AD_CF_AD3FS_48K},
  { 96000, AD_CF_ADFS_96K,  AD_CF_AD3FS_96K},
  {192000, AD_CF_ADFS_192K, AD_CF_AD3FS_192K}
};



struct Adc adc = {
  0
};


auto void AudioCloseInput(void) {
  /* Shut down old interrupt, if exists. */
  {
    u_int16 intAddr =
      (adInts[adc.iChannel] >> 4)*(INT_ENABLE1_LP-INT_ENABLE0_LP);
    u_int16 intVec = adInts[adc.iChannel] & 15;
    
    PERIP(INT_ENABLE0_HP+intAddr) &= ~(1<<intVec);
  }
}

auto ioresult AudioSetInput(register u_int16 idPattern) {
  struct AudioIdPattern __mem_y *aip = audioIdPattern;
  struct AudioIdPattern __mem_y *aipLRD[3] = {0, 0, 0}; // Left, Right, Dec6
  int i;
  int isFm = 0;

  AudioCloseInput();

#if 0
  printf("*** AudioSetInput(id=%04x)\n",
	 idPattern);
#endif

  if (!idPattern) {
    idPattern = AID_LINE1_1|AID_LINE1_3;
  }

  for (i=0; i<16; i++) {
    if (idPattern & 1) {
      if (i==5) {
	isFm = 1;
      }
#if 0
      printf("i %2d, flags %04x, resources %04x\n",
	     i, aip->flags, aip->resources);
#endif
      if (aip->flags & AIF_IS_DEC6) {
	if (aipLRD[2])
	  return S_ERROR;
	aipLRD[2] = aip;
      } else {
	if (aip->flags & (AIF_LEFT | AIF_STEREO)) {
	  if (aipLRD[0])
	    return S_ERROR;
	  aipLRD[0] = aip;
	}
	if (aip->flags & (AIF_RIGHT | AIF_CENTER | AIF_STEREO)) {
	  if (aipLRD[1])
	    return S_ERROR;
	  aipLRD[1] = aip;
	}
      }
    }
    idPattern >>= 1;
    aip++;
  }
#if 0
  printf("## aip %d %d\n", aipLRD[0], aipLRD[1]);
#endif
  if (!aipLRD[0] || !aipLRD[1]) {
    /* Not two channels */
    return S_ERROR;
  }
#if 0
  printf("## aipIdx %d %d\n", aipLRD[0]-audioIdPattern, aipLRD[1]-audioIdPattern);
#endif

  if ((aipLRD[0]->resources & ((1<<AD_INTS)-1)) !=
      (aipLRD[1]->resources & ((1<<AD_INTS)-1)) ||
	aipLRD[0]->interrupt != aipLRD[1]->interrupt) {
#if 0
    printf("##0 err res %04x %04x %04x, int %d %d\n",
	   aipLRD[0]->resources, aipLRD[0]->resources, adc.resources,
	   aipLRD[0]->interrupt,  aipLRD[1]->interrupt);
#endif
    return S_ERROR;
  }

  if (aipLRD[1]->resources & AIR_AD23) {   // If requires specific AD23 flip
    if (aipLRD[1]->flags & AIF_RIGHT_FLIPPED) {
      PERIP(AD_CF) |= AD_CF_AD23_FLP;
    } else {
      PERIP(AD_CF) &= ~AD_CF_AD23_FLP;
    }
  }

  //  printf("oldRes %04x, ad23Flip %d, ", adc.resources, adc.ad23Flip);
  for (i=0; i<3; i++) {
    if (aipLRD[i]) {
      adc.iResources |= aipLRD[i]->resources;
      adc.iChannel = aipLRD[i]->interrupt;	// Note: Dec6 overrides others
    }
  }
  //  printf("newRes %04x\n", adc.resources);

#if 0
  printf("aipL %p, aipR %p, aipD %p, ad23F %d\n",
	 aipLRD[0], aipLRD[1], aipLRD[2], adc.ad23Flip);
#endif
  for (i=0; i<3; i++) {
    if (aipLRD[i]) {
      int j;
      u_int16 skip = 0;
      struct AudioIdReg __mem_y *reg = aipLRD[i]->startReg;
      for (j=0; j<AID_REGS; j++) {
	u_int16 addr = reg->addr;
	if (!skip && addr) {
	  if (!(addr & 0x8000U)) {
	    addr |= 0x8000U;
	    skip = 1;
	  }
	  if (!skip || (aipLRD[i]->resources & AIR_AD23)) {
	    PERIP(addr) &= ~(reg->mask);
	    PERIP(addr) |= reg->val;
#if 0
	  } else {
	    printf("skip 1\n");
#endif
	  }
	} else {
#if 0
	  printf("skip 2\n");
#endif
	  skip = 0;
	}
	reg++;
      }
    }
  }

  Disable();
  {
    u_int16 intAddr =
      (adInts[adc.iChannel] >> 4)*(INT_ENABLE1_LP-INT_ENABLE0_LP);
    u_int16 intVec = adInts[adc.iChannel] & 15;
    
    //    printf("Enable int addr %d, vec %d\n", intAddr, intVec);
    PERIP(INT_ENABLE0_LP+intAddr) &= ~(1<<intVec);
    PERIP(INT_ENABLE0_HP+intAddr) |= 1<<intVec;
  }
  if (isFm) {
    PERIP(AD_CF) &= ~AD_CF_DEC6SEL_MASK;
  }

  AudioSetRate(audioIn.sampleRate, audioIn.flags & AIN_32BIT);
  Enable();

  return S_OK;
}


/*
  Find best hardware settings for the given A/D samplerate.
*/
auto ioresult AudioSetRate(register s_int32 sampleRate,
			   register u_int16 hiBits) {
  int rateMult = 1;

  sampleRate &= ~0x7F000000; /* Mask away fractional sample rate bits 30:24 */

  if (!sampleRate) {
    sampleRate = 48000;
  }

  Disable();
  audioIn.wr = audioIn.rd = audioIn.buf;
  audioIn.flags &= ~AIN_32BIT;
  if (hiBits) {
    audioIn.flags |= AIN_32BIT;
  }
  Enable();



  switch(adc.iChannel) {
  case AD_INT_MAC1:
    rateMult = 6;
    /* Intentional fall-through */
  case AD_INT_MAC0:
  case AD_INT_MAC2:
    {
      u_int32 smallestError = 0x7FFFFFFF;
      u_int32 thisRate, bestRate = 0;
      int bestIdx = 0, i;


#if 0
      printf("SetAdc(%ld), perip %ld\n", rate, clockSpeed.peripClkHz);
#endif
      for (i=0; i<ADC_RATES; i++) {
	u_int32 err;
	thisRate = (u_int32)
	  ((double)setAdc[i].nominalRate * clockSpeed.peripClkHz *
	   (1.0 / 12288000.0) + 0.5);
	err = labs(thisRate - sampleRate*rateMult);
	if (err < smallestError) {
	  smallestError = err;
	  bestIdx = i;
	  bestRate = thisRate;
	}
      }

      audioIn.sampleRate = bestRate / rateMult;

      {
	u_int16 fmCf = PERIP(FM_CF) & ~FM_CF_ENABLE;
	u_int16 adCfSampleRateBits;
	/* Stop AD converter */
	PERIP(FM_CF) = fmCf;
	if (adc.iChannel == AD_INT_MAC2) {
	  adCfSampleRateBits = setAdc[bestIdx].reg3;
	} else {
	  adCfSampleRateBits = setAdc[bestIdx].reg;
	}
	/* Set new AD samplerate */
	PERIP(AD_CF) = (PERIP(AD_CF) & ~(AD_CF_ADFS_MASK)) | adCfSampleRateBits;
	/* Restart and synchronize the AD converter
	   (otherwise channels may get reversed) */
	PERIP(FM_CF) = fmCf | FM_CF_ENABLE;
      }
    }
    break;
  default:
    return S_ERROR;
    break;
  }
  return S_OK;
}


const struct AudioChanToIdPattern __mem_y audioChanToIdPattern[] = {

  /*  {"i2s",    AID_I2S},
      {"spdif",  AID_SPDIF},*/
  {"dia1",   AID_DIA1},
  {"dia2",   AID_DIA2},
  {"dia3",   AID_DIA3},

  {"fm",     AID_FM},

  {"line1_1",AID_LINE1_1},
  {"line2_1",AID_LINE2_1},
  {"line3_1",AID_LINE3_1},
  {"mic1",   AID_MIC1},

  {"line1_2",AID_LINE1_2},
  {"line2_2",AID_LINE2_2},
  {"line3_2",AID_LINE3_2},
  {"mic2",   AID_MIC2},
  {"line1_3",AID_LINE1_3},
  {"dec6",   AID_DEC6},

  {NULL}
};
