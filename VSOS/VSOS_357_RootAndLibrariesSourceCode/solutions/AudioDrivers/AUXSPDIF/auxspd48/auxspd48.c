/**
   \file audiofs.c Audio Filesystem driver for VsOS
   \author Henrik Herranen, VLSI Solution Oy
*/

#include <vo_stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <vstypes.h>
#include <vs1005h.h>
#include <timers.h>
#include <vsos.h>
#include <sysmemory.h>
#include <ringbuf.h>
#include <apploader.h>
#include <clockspeed.h>
#include <imem.h>
#include "auxspd48.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

struct AudioIn audioIn = {
  NULL
};

struct AudioOut audioOut = {
  NULL
};

struct AuiSpdRegisters auiSpdRegisters[1];
u_int16 newSamples = 0;
void AuiSpdCyclicFunc(register struct CyclycNode *cyclicNode) {
  static u_int16 lastSampleCounter = 0;
  static s_int16 phase = -1;
  u_int16 newSampleCounter = (u_int16)audioIn.sampleCounter;
  
  static s_int16 diffOffset = 0;
  s_int16 newDiff = (u_int16)audioOut.sampleCounter -
    (u_int16)audioIn.sampleCounter + diffOffset;

  newSamples = newSampleCounter-lastSampleCounter;

#if 0
#if 1
  printf("phase = %d, ", phase);
#endif
#if 0
  printf("sc %ld - lsc %ld = %ld, ",
         newSampleCounter, lastSampleCounter, newSamples);
#endif
#if 1
  printf("dif %d, CCF %08lx ",
	 newDiff, PERIP32(FMCCF_LO));
#endif
#if 0
  printf("%ld %ld", audioIn.sampleCounter, audioOut.sampleCounter);
#endif
  printf("\n");
#endif

  if (newSamples < 20000/10 || phase < 0) {
    /* Switch between 56.448 / 61.44 MHz for 44100 / 48000 kHz reception */
    s_int16 multiplier;
    phase = (phase + 1) & 1;
    multiplier = (phase & 1) ? 1/* 96 kHz */ : 2/* 48 kHz */;
    PERIP(SP_RX_CLKDIV) = 2*5*multiplier; 
    /*
      The following formula has been adapted from:
        PERIP(SP_TX_CF) =
           clockSpeed.cpuClkHz/(multiplier*6144000)*SP_TX_CF_CLKDIV |
           SP_TX_CF_SND | SP_TX_CF_IE;
    */
    PERIP(SP_TX_CF) = multiplier*(5*SP_TX_CF_CLKDIV) |
      SP_TX_CF_SND | SP_TX_CF_IE;
    PERIP(SP_TX_CHST2) = 0;
    PERIP32(FMCCF_LO) = auiSpdRegisters[0].fmCcf;
    PERIP32(ANA_CF3) = PERIP32(ANA_CF3) &
      ~(ANA_CF3_DIV_MASK|ANA_CF3_FMDIV_MASK|ANA_CF3_2GCNTR_MASK) |
      auiSpdRegisters[0].anaCf3;
  } else if (!PERIP(SP_TX_CHST2)) {
    PERIP(SP_TX_CHST2) = SP_TX_CHST2_TX_ENA | SP_TX_CHST2_CH2_WRDLM |
      4*SP_TX_CHST2_CH2_WRDL;
    diffOffset = (u_int16)audioIn.sampleCounter - (u_int16)audioOut.sampleCounter;
  } else {
    /* Adjust clock speed very very slightly to follow sample rate of source. */
    s_int32 nd2 = newDiff;
    s_int32 ccf = (auiSpdRegisters[0].fmCcf << 6) - 50000*nd2;
    PERIP32(FMCCF_LO) = ccf >> 6;
  }

  lastSampleCounter = newSampleCounter;
}

auto u_int16 AudioInFill(void) {
  s_int16 fi = audioIn.wr - audioIn.rd;
  if (fi < 0)
    fi += audioIn.bufSize;
  return fi & ~3;
}

auto u_int16 AudioOutFree(void) {
  s_int16 fi = audioOut.wr - audioOut.rd;
  s_int16 fr;
  if (fi < 0)
    fi += audioOut.bufSize;
  fr = audioOut.bufSize - fi - 1;
  if (fr < 0)
    return 0;
  return fr & ~3;
}

auto ioresult AudioSetHiBits(register u_int16 hiBits) {
  Disable();
  audioIn.wr = audioIn.rd = audioIn.buf;
  audioIn.flags = 0;
  audioOut.wr = audioOut.rd = audioOut.buf;
  audioOut.flags = 0;
  if (hiBits) {
    audioIn.flags = AIN_32BIT;
    audioOut.flags = AIN_32BIT;
  }
  Enable();

  return 0;
}



ioresult AudioClose(void) {
  /* RX-specific */
  PERIP(SP_RX_CF) = 0;

  /* TX-specific */
  PERIP(SP_LDATA) = 0;
  PERIP(SP_LDATA_LSB) = 0;
  PERIP(SP_RDATA) = 0;
  PERIP(SP_RDATA_LSB) = 0;
  PERIP(INT_ENABLE1_HP) &= ~(INTF1_SRX | INTF1_STX); /* RX & TX interrupts */
  PERIP(SP_TX_CF) = 0;
  PERIP(SP_TX_CHST2) = 0;

  if (audioIn.bufSize) {
    FreeMemY(audioIn.buf, audioIn.bufSize);
    audioIn.bufSize = 0;
  }
  if (audioOut.bufSize) {
    FreeMemY(audioOut.buf, audioOut.bufSize);
    audioOut.bufSize = 0;
  }
}


ioresult AllocateAudioBuffer(register struct AudioIn *a,
			     register u_int16 bufSize) {
  ioresult res = S_OK;
  Disable();
  if (a->bufSize) {
    FreeMemY(a->buf, a->bufSize);
  }
  if (!(a->buf = AllocMemY(bufSize, bufSize))) {
    static u_int16 __mem_y panicBuffer[4];
    a->buf = panicBuffer;
    a->bufSize = 0;
    a->modifier = MAKEMODF(4);
    res = S_ERROR;
  } else {
    memsetY(a->buf, 0, bufSize);
    a->bufSize = bufSize;
    a->modifier = MAKEMODF(bufSize);
  }
  a->wr = a->rd = (__mem_y s_int16 *)(a->buf);
  Enable();
  return res;
}

IOCTL_RESULT AudioIoctl(register __i0 VO_FILE *self, s_int16 request,
			IOCTL_ARGUMENT argp) {
  ioresult res = S_OK;

#if 0
  printf("AUXSPD48 Ioctl(%p, %d, %p)\n", self, request, argp);
#endif

  switch (request) {
#if 0
  case IOCTL_START_FRAME:			
    break;
		
  case IOCTL_END_FRAME:
    break;
#endif

  case IOCTL_RESTART:
    PERIP(SP_RX_CF) = 0;
    PERIP(SP_TX_CF) = 0;
    PERIP(INT_ENABLE1_LP) &= ~(INTF1_SRX|INTF1_STX);
    PERIP(INT_ENABLE1_HP) &= ~(INTF1_SRX|INTF1_STX);
    /* Set to Perip Mode */
    PERIP(GPIO0_MODE) |= GPIO0_SPDIF_IN | GPIO0_SPDIF_OUT;
    WriteIMem(0x20 + INTV_SRX, ReadIMem((u_int16)SrxInterruptVector));
    WriteIMem(0x20 + INTV_STX, ReadIMem((u_int16)StxInterruptVector));
    PERIP(SP_TX_CHST0) = 0;
    PERIP(SP_TX_CHST1) = 2*SP_TX_CHST1_CLKA | (SP_TX_CHST1_CH * 0b0011) |
      (SP_TX_CHST1_FS * 0b0000);
    PERIP(SP_TX_CHST2) = 0;
    AudioSetHiBits(0);
    PERIP(SP_RX_CLKDIV) = 4*5; /* 48 kHz: 5x clock * (12.288MHz/)4. */
    PERIP(SP_RX_CF) = SP_RX_CF_EN|SP_RX_CF_INT_ENA;
    if (AllocateAudioBuffer(&audioIn, 512)) {
      res = S_ERROR;
    }
    if (AllocateAudioBuffer(&audioOut, 512)) {
      res = S_ERROR;
    }
    break;

  case IOCTL_AUDIO_GET_ORATE:
  case IOCTL_AUDIO_GET_IRATE:
    *((u_int32 *)argp) = (s_int32)newSamples*10;
    break;

  case IOCTL_AUDIO_GET_BITS:
    return (audioIn.flags & (AIN_32BIT)) ? 32 : 16;
    break;

  case IOCTL_AUDIO_SET_BITS:
    res = AudioSetHiBits((u_int16)argp > 16);
    break;

  case IOCTL_AUDIO_GET_INPUT_BUFFER_SIZE:
    return audioIn.bufSize;
    break;

  case IOCTL_AUDIO_SET_INPUT_BUFFER_SIZE:
    if (AllocateAudioBuffer(&audioIn, (u_int16)argp)) {
      res = S_ERROR;
    }
    break;

  case IOCTL_AUDIO_GET_INPUT_BUFFER_FILL:
    return AudioInFill();
    break;

  case IOCTL_AUDIO_GET_OUTPUT_BUFFER_SIZE:
    return audioOut.bufSize;
    break;

  case IOCTL_AUDIO_SET_OUTPUT_BUFFER_SIZE:
    if (AllocateAudioBuffer(&audioOut, (u_int16)argp)) {
      res = S_ERROR;
    }
    break;

  case IOCTL_AUDIO_GET_OUTPUT_BUFFER_FREE:
    return AudioOutFree();
    break;

  case IOCTL_AUDIO_GET_SAMPLE_COUNTER:
    Disable();
    *((u_int32 *)argp) = audioIn.sampleCounter;
    Enable();
    break;

  case IOCTL_AUDIO_GET_OVERFLOWS:
    Disable();
    *((u_int32 *)argp) = audioIn.overflow;
    Enable();
    break;

  case IOCTL_AUDIO_GET_UNDERFLOWS:
    Disable();
    *((u_int32 *)argp) = audioOut.underflow;
    Enable();
    break;

  default:
    res = S_ERROR;
    break;
  }

  if (audioIn.bufSize && audioOut.bufSize) {
    audioFile.flags = __MASK_PRESENT | __MASK_OPEN | __MASK_CHARACTER_DEVICE |
      __MASK_FILE |
      __MASK_READABLE | __MASK_WRITABLE;
    PERIP(INT_ENABLE1_HP) |= INTF1_SRX|INTF1_STX;
  } else {
    audioFile.flags = __MASK_PRESENT | __MASK_OPEN | __MASK_CHARACTER_DEVICE |
      __MASK_FILE;
    PERIP(INT_ENABLE1_HP) &= ~(INTF1_SRX|INTF1_STX);
  }

  return res;
}



u_int16 AudioRead(register __i0 VO_FILE *self, u_int16 *buf,
		  u_int16 destinationIndex, u_int16 bytes) {
  u_int16 left = bytes>>1;
  /* Only whole samples are allowed. This is not a character device driver. */
  if (destinationIndex || (bytes&((audioIn.flags & AIN_32BIT) ? 7 : 3))) {
    return 0;
  }
  while (left) {
    u_int16 toRead = MIN(left, 16);
    while (AudioInFill() < toRead) {
      Delay(1);
    }
    audioIn.rd =
      RING_YX_S(ringcpyYX(buf, 1, audioIn.rd, audioIn.modifier, toRead));
    buf += toRead;
    left -= toRead;
  }
  return bytes;
}

u_int16 AudioWrite(register __i0 VO_FILE *self, u_int16 *buf,
		   u_int16 sourceIndex, u_int16 bytes) {
  u_int16 left = bytes>>1;
  if (sourceIndex || (bytes&((audioOut.flags & AIN_32BIT) ? 7 : 3))) {
    return 0;
  }
  while (left) {
    u_int16 toWrite = MIN(left, 16);
    while (AudioOutFree() < toWrite) {
      Delay(1);
    }
    audioOut.wr =
      RING_XY_D(ringcpyXY(audioOut.wr, audioOut.modifier, buf, 1, toWrite));
    buf += toWrite;
    left -= toWrite;
  }
  return bytes;
}

char *Identify(register __i0 void *self, char *buf, u_int16 bufSize) {
  strncpy(buf, "AUXSPD48", bufSize);
  buf[bufSize-1] = '\0';
  return buf;
}

FILEOPS audioFileOps = {
  NULL, /* Open() */
  NULL, /* Close() */
  AudioIoctl, /* Ioctl() */
  AudioRead, /* Read() */
  AudioWrite /* Write() */
};

SIMPLE_FILE audioFile = {
  __MASK_PRESENT|__MASK_OPEN|__MASK_WRITABLE|__MASK_READABLE|__MASK_FILE, /* Flags */
  Identify, /* Identify() */
  &audioFileOps,
  0, /* pos */
  0, /* ungetc_buffer */
  NULL, /* dev */
};
