/**
   \file auospda.c Audio S/PDIF automatic driver for VSOS
   \author Henrik Herranen, VLSI Solution Oy
*/

#include <vo_stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <audio.h>
#include <timers.h>
#include <vsos.h>
#include <sysmemory.h>
#include <ringbuf.h>
#include <aucommon.h>
#include <clockspeed.h>
#include <imem.h>
#include "auspdif.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#if 1
  /* The default sample rate may be either 96 or 48 kHz. In any case,
     it must be at least as high as the highest sample rate of the
     playback system. */
#  define DEFAULT_SAMPLE_RATE 96000
#else
#  define DEFAULT_SAMPLE_RATE 48000
#endif

void SrcInterruptVector(void);
void SrcVolInterruptVector(void);
void StxInterruptVector(void);

#define AUDIO_OUT_BUF_SIZE 16
__align s_int16 __mem_y audioOutBuf[AUDIO_OUT_BUF_SIZE]; /* Note: 32-bit data */

struct AudioOutSPDif audioOutSPDif;


ioresult AudioClose(void) {
  PERIP(SP_LDATA) = 0;
  PERIP(SP_LDATA_LSB) = 0;
  PERIP(SP_RDATA) = 0;
  PERIP(SP_RDATA_LSB) = 0;
  PERIP(INT_ENABLE1_HP) &= ~(INTF1_SRC | INTF1_STX);
  PERIP(SRC_CF) = 0;
  PERIP(SP_TX_CF) = 0;
  PERIP(SP_TX_CHST2) = 0;
}

auto ioresult AudioSetRate(register s_int32 sampleRate) {
  s_int16 multiplier;
  sampleRate &= ~0x7F000000; /* Mask away fractional sample rate bits 30:24 */
  if (lockSampleRate) {
    sampleRate = lockSampleRate;
  }
  if (sampleRate >= 50000) {
    sampleRate = 96000;
    multiplier = 2;
  } else {
    sampleRate = 48000;
    multiplier = 1;
  }

  /* Has to be created */
  PERIP(GPIO0_MODE) |= GPIO0_SPDIF_OUT;

  /* Set up S/PDIF */
  PERIP(SRC_CF) = SRC_CF_ENA +
    SRC_CF_FS*((u_int16)(clockSpeed.peripClkHz/(multiplier*(2*48000L)))-1);

  PERIP(SP_TX_CF) = clockSpeed.cpuClkHz/(multiplier*6144000)*SP_TX_CF_CLKDIV |
    SP_TX_CF_SND | SP_TX_CF_IE;

#if 0
  printf("m %d, SRC_CF %04x, SP_TX_CF %04x\n",
	 multiplier, PERIP(SRC_CF), PERIP(SP_TX_CF));
#endif

  PERIP(SP_TX_CHST0) = 0;
  PERIP(SP_TX_CHST1) = 2*SP_TX_CHST1_CLKA | (SP_TX_CHST1_CH * 0b0011) |
    (SP_TX_CHST1_FS * 0b0000);
  PERIP(SP_TX_CHST2) = SP_TX_CHST2_TX_ENA | SP_TX_CHST2_CH2_WRDLM |
    4*SP_TX_CHST2_CH2_WRDL;

  /* Set audioOut structure */
  audioOutSPDif.ao.sampleRate = sampleRate;
  audioOutSPDif.ao.flags |= AIN_32BIT;

  return S_OK;
}

auto ioresult AudioSetVolume(register u_int16 volume) {
  ioresult res = S_OK;

  audioOutSPDif.volVal = volume;
  if (volume == 256) {
    WriteIMem(0x20 + INTV_SRC, ReadIMem((u_int16)SrcInterruptVector));  
    audioOutSPDif.volMult[0] = audioOutSPDif.volMult[1] = 4096;
  } else {
    WriteIMem(0x20 + INTV_SRC, ReadIMem((u_int16)SrcVolInterruptVector));  
    if (volume <= 256-48) {
      audioOutSPDif.volMult[0] = audioOutSPDif.volMult[1] = 65535;
    } else if (volume >= 511) {
      audioOutSPDif.volMult[0] = audioOutSPDif.volMult[1] = 0;
    } else {
      static const u_int16 __mem_y volTab[12] =
	{61858, 58386, 55109, 52016, 49097, 46341,
	 43740, 41285, 38968, 36781, 34716, 32768};
      u_int16 tmpVol = volume-(256-48+1);
      audioOutSPDif.volMult[0] = audioOutSPDif.volMult[1] =
	volTab[tmpVol%12] >> (tmpVol/12);
    }
  }

  return res;
}


IOCTL_RESULT AudioIoctl(register __i0 VO_FILE *self, s_int16 request,
			IOCTL_ARGUMENT argp) {
  ioresult res = S_OK;

  switch (request) {
#if 0
  case IOCTL_START_FRAME:			
    break;
		
  case IOCTL_END_FRAME:
    break;
#endif

  case IOCTL_RESTART:
    Disable();
    audioFile.flags = __MASK_PRESENT | __MASK_OPEN | __MASK_CHARACTER_DEVICE |
      __MASK_FILE /* | __MASK_READABLE | __MASK_WRITABLE */;
    PERIP(INT_ENABLE1_LP) &= ~(INTF1_SRC | INTF1_STX);
    PERIP(INT_ENABLE1_HP) &= ~(INTF1_SRC | INTF1_STX);
    WriteIMem(0x20 + INTV_STX, ReadIMem((u_int16)StxInterruptVector));  
    AudioSetRate(DEFAULT_SAMPLE_RATE);
    AudioSetVolume(0x100);
    audioOutSPDif.ao.buf = audioOutBuf;
    audioOutSPDif.ao.bufSize = AUDIO_OUT_BUF_SIZE;
    audioOutSPDif.ao.modifier = MAKEMODF(AUDIO_OUT_BUF_SIZE);
    audioOutSPDif.ao.wr = audioOutSPDif.ao.rd =
      (__mem_y s_int16 *)(audioOutSPDif.ao.buf);

    PERIP(INT_ENABLE1_HP) |= (INTF1_SRC | INTF1_STX);

    Enable();
    break;

  case IOCTL_AUDIO_GET_ORATE:
    *((u_int32 *)argp) = audioOutSPDif.ao.sampleRate;
    break;

  case IOCTL_AUDIO_SET_ORATE:
  case IOCTL_AUDIO_SET_RATE_AND_BITS:
    res = AudioSetRate(labs(*((s_int32 *)argp)));
    break;

  case IOCTL_AUDIO_GET_VOLUME:
    res = audioOutSPDif.volVal;
    break;

  case IOCTL_AUDIO_SET_VOLUME:
    res = AudioSetVolume((int)argp);
    break;

  default:
    res = S_ERROR;
    break;
  }

  if (ioctl_orig) {
    /*    stdaudioout->op->Ioctl = ioctl_orig;*/
    res = ioctl_orig(stdaudioout, request, argp);
    /*    stdaudioout->op->Ioctl = AudioIoctl;*/
  }

#if 0
  printf("  SRC_CF %04x, SP_TX_CF %04x\n",
	 PERIP(SRC_CF), PERIP(SP_TX_CF));
#endif
#if 0
  printf("  res %04x\n", res);
#endif

  return res;
}


char *Identify(register __i0 void *self, char *buf, u_int16 bufSize) {
  strncpy(buf, "AUOSPDA", bufSize);
  buf[bufSize-1] = '\0';
  return buf;
}

FILEOPS audioFileOps = {
  NULL, /* Open() */
  NULL, /* Close() */
  AudioIoctl, /* Ioctl() */
  CommonOkResultFunction, /* Read() */
  CommonOkResultFunction /* Write() */
};

SIMPLE_FILE audioFile = {
  __MASK_PRESENT|__MASK_OPEN|/*__MASK_WRITABLE|__MASK_READABLE|*/__MASK_FILE, /* Flags */
  Identify, /* Identify() */
  &audioFileOps,
  0, /* pos */
  0, /* ungetc_buffer */
  NULL, /* dev */
};
