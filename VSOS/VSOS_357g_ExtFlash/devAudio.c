/**
   \file devAudio.c Audio device driver for VSOS.
   \author Henrik Herranen
*/

#include <stdlib.h>
#include <string.h>
#include <audio.h>
#include <timers.h>
#include <sysmemory.h>
#include <clockspeed.h>
#include "vsos.h"
#include "devAudio.h"
#include "vo_fat.h"	
#include "vs1005g.h"
#include "mmcCommands.h"
#include "devboard.h"
//#include "forbid_stdout.h"
#include "vo_stdio.h"
//#include "uart1005.h"
#include "audiofs.h"
#include "ringbuf.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

struct Adc adc;
__align s_int16 srcBuf[SRC_BUFFER_SIZE];
struct Src src;
void AdcInterruptVector(void);	 ///< For hooking. Never call this from C
void Dec6InterruptVector(void);	 ///< For hooking. Never call this from C
void SrcInterruptVector(void);	 ///< For hooking. Never call this from C
void SPDifNoVolInterruptVector(void); ///< For hooking. Never call this from C
void SPDifVolInterruptVector(void); ///< For hooking. Never call this from C


#pragma msg 30 off //VO_FILE * params trigger warnings
const DEVICE devAudioDefaults = {	
	0, //u_int16   flags; ///< VSOS Flags
	DevAudioIdentify, //const char* (*Identify)(void *obj, char *buf, u_int16 bufsize);	 
	DevAudioCreate, //ioresult (*Create)(DEVICE *dev, char *name, struct filesystem_descriptor *fs);
	(ioresult(*)(DEVICE*))CommonOkResultFunction, //ioresult (*Delete)(VO_FILE *f);
	DevAudioIoctl, //ioresult (*Ioctl)(VO_FILE *f, s_int16 request, char *argp); //Start, Stop, Flush, Check Media
	// Stream operations
	DevAudioRead, //	u_int16  (*Read) (register __i0 VO_FILE *f, void *buf, u_int16 destinationIndex, u_int16 bytes); ///< Read bytes to *buf, at byte index
	DevAudioWrite, // (*Write)(register __i0 VO_FILE *f, void *buf, u_int16 sourceIndex, u_int16 bytes); ///< Write bytes from *buf at byte index
	// Block device operations
	//DevAudioBlockRead, //ioresult (*BlockRead)(VO_FILE *f, u_int32 firstBlock, u_int16 blocks, u_int16 *data);
	//DevAudioBlockWrite, //ioresult (*BlockWrite)(VO_FILE *f, u_int32 firstBlock, u_int16 blocks, u_int16 *data);	
	//FILESYSTEM *fs; ///< pointer to the filesystem driver for this device
	//u_int16  deviceInstance; ///< identifier to detect change of SD card etc
	//u_int16  hardwareInfo[6]; ///< Device driver's info of this hardware
	//u_int16  deviceInfo[16]; ///< Filesystem driver's info of this device
};
#pragma msg 30 on

#define ADC_RATES 4


u_int16 const halfDB[] = {
  32768, 30929, 29193, 27554, 26008, 24548,
  23170, 21870, 20643, 19484, 18390, 17358
};

/**
   This function makes the assumption that double signal is 6 dB, which
   isn't entirely correct (2x signal is approximately 6.02 dB). Nevertheless,
   it's close enough for any non-scientific applications.
 */
auto u_int16 DB12ToLin(register __b0 u_int16 n) {
  if (n <= 191) {
    return halfDB[n % 12] >> (n/12);
  }
  return 0;
}



const struct AudioChanToIdPattern __y audioChanToIdPattern[] = {

  {"i2s",    AID_I2S},
  {"spdif",  AID_SPDIF},
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


const struct AudioIdPattern __y audioIdPattern[] = {
  /* I2S & SPDIF */
  {AIF_STEREO,
   AIR_I2S, AD_INT_I2S,
   {{0}}},
  {AIF_STEREO,
   AIR_SPDIF, AD_INT_SPDIF,
   {{0}}},

  /* DIA1-3 */
  {AIF_LEFT | AIF_CAN_USE_DEC6,
   AIR_MAC0, AD_INT_MAC0,
   {{0}}},
  {AIF_RIGHT | AIF_CAN_USE_DEC6,
   AIR_MAC0, AD_INT_MAC0,
   {{0}}},
  {AIF_CENTER | AIF_CAN_USE_DEC6,
   AIR_MAC2, AD_INT_MAC2,
   {{0}}},

  /* FM */
  {AIF_STEREO | AIF_CAN_USE_DEC6 | AIF_NEEDS_DEC6,
   AIR_MAC0 | AIR_MAC1, AD_INT_MAC0,
   {{0}}},

  /* LINE1_1 - LINE3_1 + MIC1 */
  {AIF_LEFT | AIF_CAN_USE_DEC6,
   AIR_MAC0 | AIR_LINE1_1, AD_INT_MAC0,
   {{AD_CF, AD_CF_ADENA|AD_CF_ADFS_MASK, AD_CF_ADENA|AD_CF_ADFS_48K},
    {ANA_CF0, ANA_CF0_M1FM|ANA_CF0_M1LIN|ANA_CF0_M1MIC, ANA_CF0_M1LIN},
    {ANA_CF2, ANA_CF2_M1_ENA|ANA_CF2_AMP1_ENA, ANA_CF2_M1_ENA},
    {FM_CF,FM_CF_ENABLE|FM_CF_FM_ENA|FM_CF_UAD1, FM_CF_ENABLE}}},
  {AIF_LEFT | AIF_CAN_USE_DEC6,
   AIR_MAC0 | AIR_LINE2_1, AD_INT_MAC0,
   {{0}}},
  {AIF_LEFT | AIF_CAN_USE_DEC6,
   AIR_MAC0 | AIR_LINE3_1, AD_INT_MAC0,
   {{0}}},
  {AIF_LEFT | AIF_CAN_USE_DEC6,
   AIR_MAC0 | AIR_MIC1N | AIR_MIC1P, AD_INT_MAC0,
   {{AD_CF, AD_CF_ADENA|AD_CF_ADFS_MASK, AD_CF_ADENA|AD_CF_ADFS_48K},
    {ANA_CF0, ANA_CF0_M1FM|ANA_CF0_M1LIN|ANA_CF0_M1MIC, ANA_CF0_M1FM|ANA_CF0_M1MIC},
    {ANA_CF2, ANA_CF2_M1_ENA|ANA_CF2_AMP1_ENA, ANA_CF2_M1_ENA|ANA_CF2_AMP1_ENA},
    {ANA_CF3, ANA_CF3_GAIN1MASK, ANA_CF3_GAIN1_20DB},
    {FM_CF,FM_CF_ENABLE|FM_CF_FM_ENA|FM_CF_UAD1, FM_CF_ENABLE}}},

  /* LINE1_2 - LINE3_2 + MIC2 */
  {AIF_RIGHT | AIF_CENTER_FLIPPED | AIF_CAN_USE_DEC6,
   AIR_MAC0 | AIR_LINE1_1 | AIR_AD23, AD_INT_MAC0,
   {{0}}},
  {AIF_RIGHT | AIF_CENTER_FLIPPED | AIF_CAN_USE_DEC6,
   AIR_MAC0 | AIR_LINE2_1 | AIR_AD23, AD_INT_MAC0,
   {{0}}},
  {AIF_RIGHT | AIF_CENTER_FLIPPED | AIF_CAN_USE_DEC6,
   AIR_MAC0 | AIR_LINE3_1 | AIR_AD23, AD_INT_MAC0,
   {{0}}},
  {AIF_RIGHT | AIF_CENTER_FLIPPED | AIF_CAN_USE_DEC6,
   AIR_MAC0 | AIR_MIC2N | AIR_MIC2P | AIR_AD23, AD_INT_MAC0,
   {{AD_CF, AD_CF_ADENA|AD_CF_ADFS_MASK, AD_CF_ADENA|AD_CF_ADFS_48K},
    {ANA_CF0, ANA_CF0_M2FM|ANA_CF0_M2LIN|ANA_CF0_M2MIC, ANA_CF0_M2FM|ANA_CF0_M2MIC},
    {ANA_CF2, ANA_CF2_M2_ENA|ANA_CF2_AMP2_ENA, ANA_CF2_M2_ENA|ANA_CF2_AMP2_ENA},
    {ANA_CF3, ANA_CF3_GAIN2MASK, ANA_CF3_GAIN2_20DB},
    {FM_CF&0x7FFF,FM_CF_ENABLE|FM_CF_FM_ENA|FM_CF_UAD3, FM_CF_ENABLE},
    {FM_CF,       FM_CF_ENABLE|FM_CF_FM_ENA|FM_CF_UAD2, FM_CF_ENABLE}}},

  /* LINE1_3 */
  {AIF_CENTER | AIF_RIGHT_FLIPPED | AIF_CAN_USE_DEC6,
   AIR_MAC0 | AIR_LINE1_3 | AIR_AD23, AD_INT_MAC0,
   {{AD_CF&0x7FFF, AD_CF_ADENA|AD_CF_ADFS_MASK, AD_CF_ADENA|AD_CF_ADFS_48K},
    {AD_CF       , AD_CF_AD3ENA|AD_CF_AD3FS_MASK, AD_CF_AD3ENA|AD_CF_AD3FS_48K},
    {ANA_CF0, ANA_CF0_M2FM|ANA_CF0_M2LIN|ANA_CF0_M2MIC, ANA_CF0_M2LIN},
    {ANA_CF2, ANA_CF2_M2_ENA|ANA_CF2_M3_ENA|ANA_CF2_AMP2_ENA,
     ANA_CF2_M2_ENA|ANA_CF2_M3_ENA},
    {FM_CF&0x7FFF,FM_CF_ENABLE|FM_CF_FM_ENA|FM_CF_UAD3, FM_CF_ENABLE},
    {FM_CF       ,FM_CF_ENABLE|FM_CF_FM_ENA|FM_CF_UAD2, FM_CF_ENABLE}}},

  /* DEC6 */
  {AIF_STEREO | AIF_IS_DEC6,
   AIR_MAC1, AD_INT_MAC1,
   {{AD_CF,AD_CF_DEC6SEL_MASK|AD_CF_DEC6ENA,AD_CF_DEC6ENA|AD_CF_DEC6SEL_STEREO}}},
};

const u_int16 __y adInts[AD_INTS] = {
  INTV_MAC0, INTV_MAC1, INTV_MAC2, INTV_I2S, INTV_SRC, INTV_SRC
};

struct SetAdc {
  u_int32 nominalRate;
  u_int16 reg;
  u_int16 reg3;
} const __y setAdc[ADC_RATES] = {
  { 24000, AD_CF_ADFS_24K,  AD_CF_AD3FS_24K},
  { 48000, AD_CF_ADFS_48K,  AD_CF_AD3FS_48K},
  { 96000, AD_CF_ADFS_96K,  AD_CF_AD3FS_96K},
  {192000, AD_CF_ADFS_192K, AD_CF_AD3FS_192K}
};



auto ioresult DevAudioCloseInput(register VO_FILE *fp);

ioresult DevAudioSetIBuffer(VO_FILE *fp, register u_int16 bufSize){ 
  AudioFileInfo *fi = (AudioFileInfo *)fp->fileInfo;
  struct AdcChannel *ac = adc.adc+fi->iChannel;

  if (bufSize & (bufSize-1)) {
    /* Buffer size is not power of two */
    return S_ERROR;
  }

  Disable();
  if (ac->buf) {
    FreeMemY(ac->buf, ac->bufSize);
    ac->buf = NULL;
  }
  if (!(ac->buf = AllocMemY(bufSize, bufSize))) {
    DevAudioCloseInput(fp);
    return S_ERROR;
  }
  ac->bufSize = bufSize;
  ac->modifier = MAKEMODF(bufSize);
  ac->wr = ac->rd = (s_int16 __y *)(ac->buf);
  
  Enable();
  return S_OK;
}

/*
  Find best hardware settings for the given A/D samplerate.
*/
ioresult DevAudioSetIRate(VO_FILE *fp, register u_int32 rate) {
  AudioFileInfo *fi = (AudioFileInfo *)fp->fileInfo;
  struct AdcChannel *ac = adc.adc+fi->iChannel;
  int rateMult = 1;

  //  printf("### SetIRate(fp, %ld)\n", rate);

  switch(fi->iChannel) {
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
	err = labs(thisRate - rate*rateMult);
	if (err < smallestError) {
	  smallestError = err;
	  bestIdx = i;
	  bestRate = thisRate;
	}
      }

      ac->samplerate = bestRate / rateMult;

      if (fi->iChannel == AD_INT_MAC2) {
	PERIP(AD_CF) &= ~(3*AD_CF_AD3FS);
	PERIP(AD_CF) |= setAdc[bestIdx].reg3;
      } else {
	PERIP(AD_CF) &= ~(3*AD_CF_ADFS);
	PERIP(AD_CF) |= setAdc[bestIdx].reg;
      }
    }
    break;
  case AD_INT_I2S:
    return S_ERROR;
    break;
  case AD_INT_SRC:
    return S_ERROR;
    break;
  case AD_INT_SPDIF:
    return S_ERROR;
    break;
  default:
    return S_ERROR;
    break;
  }
  return S_OK;
}


ioresult DevAudioSetOutputs(DEVICE *dev, register u_int16 out) {
#if 0
  AudioFileInfo *fi = (AudioFileInfo *)fp->fileInfo;
  DEVICE *dev = fp->dev;
#endif
  devAudioHwInfo *hw = (devAudioHwInfo *)dev->hardwareInfo;

  if (!(out & AOR_DAC)) {
    return S_ERROR;
  }
  if (out & AOR_I2S) {
    PERIP(I2S_CF) = I2S_CF_FS96K32B | I2S_CF_ENA |
      I2S_CF_ENAMCK | I2S_CF_MASTER;
    PERIP(GPIO1_MODE) |= GPIO1_I2S_DI | GPIO1_I2S_DO | GPIO1_I2S_BITCLK |
			 GPIO1_I2S_FRAME | GPIO1_I2S_MCLK;
  } else {
    PERIP(I2S_CF) = 0;
    PERIP(GPIO1_MODE) &= ~(GPIO1_I2S_DI | GPIO1_I2S_DO | GPIO1_I2S_BITCLK |
			   GPIO1_I2S_FRAME | GPIO1_I2S_MCLK);
  }
  if (out & AOR_SPDIF) {
    if (!(hw->oResources & AOR_SPDIF)) {
      /* Has to be created */
      PERIP(GPIO0_MODE) |= GPIO0_SPDIF_OUT;

      /* Turns USB compatible clock off if in use. */
      SetClockSpeed(clockSpeed.cpuClkHz);

      PERIP(SRC_CF) = SRC_CF_ENA +
	SRC_CF_FS*((u_int16)(clockSpeed.peripClkHz/(2*48000))-1);

      PERIP(SP_TX_CF) = clockSpeed.cpuClkHz/6144000*SP_TX_CF_CLKDIV |
	SP_TX_CF_SND | SP_TX_CF_IE;

      PERIP(SP_TX_CHST0) = 0;
      PERIP(SP_TX_CHST1) = (SP_TX_CHST1_CH * 0b0011) +
	(SP_TX_CHST1_FS * 0b0000);
      PERIP(SP_TX_CHST2) = SP_TX_CHST2_TX_ENA | SP_TX_CHST2_CH2_WRDLM;
      memset(srcBuf, 0, sizeof(srcBuf));
      src.rd = srcBuf;
      src.wr = srcBuf+SRC_BUFFER_SIZE/2;

      PERIP(INT_ENABLE1_HP) |= INTF1_STX | INTF1_SRC;
      PERIP(INT_ENABLE1_LP) &= ~(INTF1_STX | INTF1_SRC);
    }
  } else { /* !(out & AOR_SPDIF) */
    /* Has to be unconnected */
    PERIP(INT_ENABLE1_HP) &= ~(INTF1_STX | INTF1_SRC);
    PERIP(SRC_CF) = 0;
    PERIP(SP_TX_CF) = 0;
  }
  return S_OK;
}



const char* DevAudioIdentify(register __i0 void *obj, char *buf,
			     u_int16 bufsize) {
  return "Audio";
}

	
ioresult DevAudioCreate(register __i0 DEVICE *dev, void *name,
			u_int16 extraInfo) {
  devAudioHwInfo *hw;
  memset(&adc, 0, sizeof(adc));
  memset(&src, 0, sizeof(src));
  src.modifier = MAKEMODF(SRC_BUFFER_SIZE);
  src.spdifVolHalfDb = 4;
  PERIP(INT_ENABLE1_HP) &= ~(INTF1_STX | INTF1_SRC);

  hw = (devAudioHwInfo *)dev->hardwareInfo;
  //  printf ("##!!##!! DevAudioCreate(%x,%d)",name,extraInfo);
  memcpy(dev, &devAudioDefaults, sizeof(*dev));
  hw->outputSampleRate = 48000;
  SetRate(hw->outputSampleRate);
  if (extraInfo) {
    hw->outputSampleRate = extraInfo * 5;
  }
  dev->deviceInstance = __nextDeviceInstance++;

  WriteIMem((u_int16 *)(0x20 + INTV_MAC0), ReadIMem(AdcInterruptVector));
  WriteIMem((u_int16 *)(0x20 + INTV_MAC1), ReadIMem(Dec6InterruptVector));
  WriteIMem((u_int16 *)(0x20 + INTV_SRC), ReadIMem(SrcInterruptVector));
  WriteIMem((u_int16 *)(0x20 + INTV_STX), ReadIMem(SPDifNoVolInterruptVector));

  PERIP(ANA_CF2) |= ANA_CF2_HIGH_REF | ANA_CF2_REF_ENA;

  return DevAudioIoctl(dev, IOCTL_RESTART, 0);
}

ioresult VoAudioOpenFile(register __i0 VO_FILE *f, const char *name,
			 char *mode);


ioresult DevAudioOpenFile(register __i0 VO_FILE *f, const char *name,
			  const char *mode) {
  ioresult res;
  char *c;
  devAudioHwInfo *hw = (devAudioHwInfo *)f->dev->hardwareInfo;	
#if 0
  printf("DevAudioOpenFile(\"%s\", %s)\n", name, mode);
#endif
  f->flags = __MASK_PRESENT | __MASK_OPEN | __MASK_READABLE | __MASK_WRITABLE;
  DevAudioIoctl((void *)f, IOCTL_RESTART, 0);
  return VoAudioOpenFile(f, name, mode);
}

auto u_int16 DevAudioAdcFill(register struct AdcChannel *ac) {
  s_int16 fr = ac->wr - ac->rd;

  if (fr < 0)
    fr += ac->bufSize;

  return fr;
}

extern u_int32 huuhaa;

u_int16 DevAudioRead(register __i0 VO_FILE *f, void *buf,
		     u_int16 destIndex,
		     u_int16 bytes){ ///< Read bytes to *buf, at byte index
  AudioFileInfo *fi = (AudioFileInfo *)f->fileInfo;
  struct AdcChannel *ac = adc.adc+fi->iChannel;
  u_int16 *d = buf;
  u_int16 left = bytes >> 1;


#if 0
  printf("DevAudioRead(%p, %p, sI %d, b %d)\n",
	 f, buf, destIndex, bytes);
#endif

  if (!(f->flags & __MASK_FILE) || (destIndex & 1)) {
    return 0;
  }

  d += destIndex >> 1;

  while (left) {
    u_int16 toRead = MIN(left, 16);
    while (DevAudioAdcFill(ac) < toRead) {
      Delay(1);
    }
#if 0
    printf("Gonna RING(%p, 1, %p, %04x, %d)\n",
	   d, ac->rd, ac->modifier, toRead);
#endif
    ac->rd = RING_YX_S(ringcpyYX(d, 1, ac->rd,
					ac->modifier, toRead));
    d += toRead;
    left -= toRead;
  }

#if 0
  printf("End of DevAudioRead()\n");
#endif

  return bytes & ~1;
}

u_int16 DevAudioWrite(register __i0 VO_FILE *f, void *buf,
		      u_int16 sourceIndex,
		      u_int16 bytes) { ///< Write bytes from *buf at byte index
  //  AudioFileInfo *fi = (AudioFileInfo *)f->fileInfo;
  u_int16 *d = buf;
  u_int16 left = bytes >> 2;

  if (!(f->flags & __MASK_FILE) || (sourceIndex & 1)) {
    return 0;
  }

  d += sourceIndex >> 1;

  while (left) {
    u_int16 toWrite = MIN(left, 16);
    while (AudioBufFree() < toWrite*2) {
      Delay(1);
    }
    AudioOutputSamples(d, toWrite);
    d += toWrite*2;
    left -= toWrite;
  }
  return bytes & ~3;
}


#if 0
{
  u_int16 t = *((u_int16 *)argp);
  PERIP(INT_ENABLEL0) &= ~(INTF_MAC0 | INTF_MAC1);
  PERIP(INT_ENABLEH0) &= ~(INTF_MAC0 | INTF_MAC1);
  if (t & 0xC000U) {
    u_int16 src = ((t >> 14) - 1) << AD_CF_DEC6SEL_B;
    PERIP(INT_ENABLEH0) |= INTF_MAC1;
    PERIP(AD_CF) |= AD_CF_DEC6ENA | src;
  } else {
    PERIP(INT_ENABLEH0) |= INTF_MAC0;
    PERIP(AD_CF) &= ~(AD_CF_DEC6ENA | AD_CF_DEC6SEL_MASK);
  }
}
#endif




auto ioresult DevAudioOpenInput(register VO_FILE *fp,
				register u_int16 idPattern,
				void __y *dBuf,
				u_int16 bufSize) {
  DEVICE *dev;
  devAudioHwInfo *hw;
  struct AudioIdPattern __y *aip = audioIdPattern;
  struct AudioIdPattern __y *aipLRD[3] = {0, 0, 0}; // Left, Right, Dec6
  int i;
  AudioFileInfo *fi = (AudioFileInfo *)fp->fileInfo;

#if 0
  printf("\n*** DevAudioOpenInput(fp=%p, id=%04x, dB=%p, bSiz=%d)\n",
	 fp, idPattern, dBuf, bufSize);
#endif

  if (!dBuf && !bufSize) {
    bufSize = 32;
  }

  dev = fp->dev;
  hw = (devAudioHwInfo *)dev->hardwareInfo;

  if (!idPattern) {
    idPattern = AID_LINE1_1|AID_LINE1_3;
  }

  for (i=0; i<16; i++) {
    if (idPattern & 1) {
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
  if (!aipLRD[0] || !aipLRD[1]) {
    /* Mono channel */
    SysError("#1: Not implemented yet!\n");
    while (1)
      ;
  }


  if ((aipLRD[0]->resources & ((1<<AD_INTS)-1)) !=
      (aipLRD[1]->resources & ((1<<AD_INTS)-1)) ||
      ((aipLRD[0]->resources|aipLRD[1]->resources)&adc.resources&~AIR_AD23) ||
      aipLRD[0]->interrupt != aipLRD[1]->interrupt) {
#if 0
    printf("##0 err res %04x %04x %04x, int %d %d\n",
	   aipLRD[0]->resources, aipLRD[0]->resources, adc.resources,
	   aipLRD[0]->interrupt,  aipLRD[1]->interrupt);
#endif
    return S_ERROR;
  }

  if (aipLRD[1]->resources & AIR_AD23) {	// If requires specific AD23 flip
    if (adc.ad23Flip) {		// If AD23 flip already in use
      //      printf("##0, %d\n", aipLRD[1]->flags & AIF_RIGHT);
      if (aipLRD[1]->flags & AIF_RIGHT) { // ... check if it is in our wanted 
	if (PERIP(AD_CF) & AD_CF_AD23_FLP) {	// ... position
	  //	  printf("##1ERR\n");
	  return S_ERROR;
	}
      } else {
	if (!(PERIP(AD_CF) & AD_CF_AD23_FLP)) {
	  //	  printf("##2ERR\n");
	  return S_ERROR;
	}
      }
    } else {				// Not yet in use, set as we'd like
      //      printf("##2 %d %04x\n", aipLRD[1]->flags & AIF_RIGHT_FLIPPED, aipLRD[1]->flags);
      if (aipLRD[1]->flags & AIF_RIGHT_FLIPPED) {
	PERIP(AD_CF) |= AD_CF_AD23_FLP;
      } else {
	PERIP(AD_CF) &= ~AD_CF_AD23_FLP;
      }
    }
    adc.ad23Flip++;			// AD23 flip Used once more
  }

  //  printf("oldRes %04x, ad23Flip %d, ", adc.resources, adc.ad23Flip);
  for (i=0; i<3; i++) {
    if (aipLRD[i]) {
      fi->iResources |= aipLRD[i]->resources;
      fi->iChannel = aipLRD[i]->interrupt;	// Note: Dec6 overrides others
    }
    adc.resources |= fi->iResources;
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
      struct AudioIdReg __y *reg = aipLRD[i]->startReg;
      for (j=0; j<AID_REGS; j++) {
	u_int16 addr = reg->addr;
	if (!skip && addr) {
	  if (!(addr & 0x8000U)) {
	    addr |= 0x8000U;
	    skip = 1;
	  }
	  if (!skip || (aipLRD[i]->resources & AIR_AD23)) {
#if 0
	    printf("ij %d:%d, skp %d, a %04x, m %04x, v %04x\n",
		   i, j, skip, addr, reg->mask, reg->val);
#endif
	    //	  printf("-- %04x", PERIP(addr));
	    PERIP(addr) &= ~(reg->mask);
	    //	  printf(" %04x", PERIP(addr));
	    PERIP(addr) |= reg->val;
	    //	  printf(" %04x\n", PERIP(addr));
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

  if (bufSize & (bufSize-1)) {
    /* Buffer size is not power of two */
    return S_ERROR;
  }

	//printf("#1 dBuf %p\n", dBuf);
   if (!dBuf) {
  	//printf("#2 bufSize %d\n", bufSize);
    if (!(dBuf = AllocMemY(bufSize, bufSize))) {
      return S_ERROR;
    }
  }
#if 0
  printf("d[0] %p bufSize %d\n", dBuf, bufSize);
#endif
  memsetY(dBuf, 0, bufSize);
      
  /* Activate A/D converter */
#if 0
  PERIP(AD_CF) = AD_CF_ADFS_48K | AD_CF_ADENA | AD_CF_AD23_FLP;
  hw->inputSampleRate = DevAudioSetIRate(fp, 48000);
#endif

  Disable();
#if 0
  printf("Interrupt #%d = hw int %d, ad23F %d\n",
	 fi->iChannel, adInts[fi->iChannel], adc.ad23Flip);
#endif
  {
    u_int16 intAddr =
      (adInts[fi->iChannel] >> 4)*(INT_ENABLE1_LP-INT_ENABLE0_LP);
    u_int16 intVec = adInts[fi->iChannel] & 15;
    
    //    printf("Enable int addr %d, vec %d\n", intAddr, intVec);
    PERIP(INT_ENABLE0_LP+intAddr) &= ~(1<<intVec);
    PERIP(INT_ENABLE0_HP+intAddr) |= 1<<intVec;
  }

  {
    struct AdcChannel *ac = adc.adc+fi->iChannel;
    memset(ac, 0, sizeof(*ac));
    ac->modifier = MAKEMODF(bufSize);
    ac->samplerate = 48000;
    if (fi->iChannel == AD_INT_MAC1)
      ac->samplerate /= 6;
    ac->wr = ac->rd = ac->buf = dBuf;
    ac->bufSize = bufSize;
  }
  Enable();

  fp->flags |= __MASK_READABLE;

  return S_OK;
}


auto ioresult DevAudioCloseInput(register VO_FILE *fp) {
  DEVICE *dev;
  devAudioHwInfo *hw;
  u_int16 size;
  u_int16 i;
  AudioFileInfo *fi = (AudioFileInfo *)fp->fileInfo;
  struct AdcChannel *ac = adc.adc+fi->iChannel;

#if 0
  printf("## CloseInput(), ad23Flip %d, iRes %04x\n",
	 adc.ad23Flip, fi->iResources);
#endif

  dev = fp->dev;
  hw = (devAudioHwInfo *)dev->hardwareInfo;

  /* Deactivate A/D converter */
#if 0
  PERIP(AD_CF) = 0;
  PERIP(FM_CF) = 0;
#endif
#if 0
  PERIP(ANA_CF2) &= ~(ANA_CF2_M1_ENA | ANA_CF2_M2_ENA);
#endif

  {
    u_int16 intAddr =
      (adInts[fi->iChannel] >> 4)*(INT_ENABLE1_LP-INT_ENABLE0_LP);
    u_int16 intVec = adInts[fi->iChannel] & 15;
    
    //    printf("Disable int addr %d, vec %d\n", intAddr, intVec);
    PERIP(INT_ENABLE0_HP+intAddr) &= ~(1<<intVec);
  }

  if (ac->buf) {
    FreeMemY(ac->buf, ac->bufSize);
    ac->buf = NULL;
  }

  if (fi->iResources & AIR_AD23) {
    adc.ad23Flip--;
  }
  adc.resources &= ~fi->iResources;
  fi->iResources = 0;

  fp->flags &= ~(__MASK_READABLE);

  return S_OK;
}


 
ioresult DevAudioIoctl(register __i0 void *self, s_int16 request,
		       char *argp) {
  DEVICE *dev;
  VO_FILE *f = self;
  devAudioHwInfo *hw;
  AudioFileInfo *fi = NULL;
  u_int16 i, n, cmd;
  struct AdcChannel *ac = NULL;

  if ((f->flags & __MASK_FILE)) {
    dev = f->dev;
    fi = (AudioFileInfo *)f->fileInfo;
    ac = adc.adc+fi->iChannel;
  } else {
    if (request != IOCTL_RESTART) {
      return S_ERROR;
    }
    f = NULL;
    dev = self;
  }
  hw = (devAudioHwInfo *)dev->hardwareInfo;
#if 0
  printf("DevAudioIoctl(f=0x%x,dev=0x%x,req=%d)\n",
	 f, dev, request);
#endif

  switch (request) {
  case IOCTL_START_FRAME:			
    break;
		
  case IOCTL_END_FRAME:
    break;

  case IOCTL_RESTART:
    PERIP(ANA_CF1) |= (ANA_CF1_DA_ENA | ANA_CF1_DRV_ENA);
    dev->flags = __MASK_PRESENT |
      __MASK_OPEN | __MASK_CHARACTER_DEVICE |
      __MASK_READABLE | __MASK_WRITABLE;
    dev->fs = &AudioFileSystem;
    SetRate(hw->outputSampleRate);
    DevAudioSetOutputs(dev, AOR_DAC);
    break;

#if 0
  case IOCTL_OPEN_DEVICE_FILE:
    hw->openCounter++;
    printf("IOCTL_OPEN, Self %p, openCounter %d\n", self, hw->openCounter);
    break;
  case IOCTL_CLOSE_DEVICE_FILE:
    hw->openCounter--;
    printf("IOCTL_CLOSE, Self %p, openCounter %d\n", self, hw->openCounter);
    break;			
#endif

  case IOCTL_AUDIO_GET_ORATE:
    *((u_int32 *)argp) = hw->outputSampleRate;
    break;

  case IOCTL_AUDIO_SET_ORATE:
    //  case IOCTL_AUDIO_SET_ORATE_BLOCK:
    hw->outputSampleRate = MAX(100L, *((u_int32 *)argp));
    SetRate(hw->outputSampleRate);
    break;

  case IOCTL_AUDIO_GET_IRATE:
    {
      u_int32 t = ac->samplerate;
      *((u_int32 *)argp) = t;
    }
    break;

  case IOCTL_AUDIO_SET_IRATE:
    return DevAudioSetIRate(f, *((u_int32 *)argp));
    break;

  case IOCTL_AUDIO_OPEN_INPUT:
    return DevAudioOpenInput(f, (u_int16)argp, NULL, 0);
    break;

  case IOCTL_AUDIO_CLOSE_INPUT:
    return DevAudioCloseInput(f);
    break;

  case IOCTL_AUDIO_SELECT_INPUT:
    {
      s_int16 __y *buf;
      s_int16 bufSize = ac->bufSize;
      buf = ac->buf;
      ac->buf = NULL;
      DevAudioCloseInput(f);
      //      printf("Closed, gonna open with %04x\n", (u_int16)argp);
      return DevAudioOpenInput(f, (u_int16)argp, buf, bufSize);
    }
    break;

  case IOCTL_AUDIO_GET_INPUT_BUFFER_SIZE:
    return ac->bufSize;
    break;

  case IOCTL_AUDIO_SET_INPUT_BUFFER_SIZE:
    return DevAudioSetIBuffer(f, (u_int16)argp);
    break;

  case IOCTL_AUDIO_GET_INPUT_BUFFER_FILL:
    return DevAudioAdcFill(ac);
    break;

  case IOCTL_AUDIO_GET_OUTPUT_BUFFER_SIZE:
    return YRAM_START+YRAM_SIZE-AUDIO_START;
    break;

  case IOCTL_AUDIO_GET_OUTPUT_BUFFER_FREE:
    return AudioBufFree();
    break;

  case IOCTL_AUDIO_SET_OUTPUTS:
    return DevAudioSetOutputs(dev, (u_int16)argp);
    break;

  case IOCTL_AUDIO_GET_OUTPUTS:
    return hw->oResources;
    break;

  case IOCTL_AUDIO_GET_SPDIF_VOLUME:
    return src.spdifVolHalfDb;
    break;

  case IOCTL_AUDIO_SET_SPDIF_VOLUME:
    src.spdifVolHalfDb = (u_int16)argp;
    // Take linear value, then add 2 dB (multiply with 1.2589 = 41251)
    // This is done to compensate for the -2.1 dB attenuation of the two
    // digital SRCs that are in the signal path.
    src.spdifVolLin = (u_int32)DB12ToLin(src.spdifVolHalfDb) * 41251U >> 15;
    if (src.spdifVolHalfDb == 4) {
      WriteIMem((u_int16 *)(0x20 + INTV_STX),
		ReadIMem(SPDifNoVolInterruptVector));
    } else {
      WriteIMem((u_int16 *)(0x20 + INTV_STX),
		ReadIMem(SPDifVolInterruptVector));
    }
    break;

  default:
    return S_ERROR;
    break;
  }			
  return S_OK;
}
