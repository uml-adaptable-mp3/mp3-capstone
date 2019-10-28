#ifndef ENC_MP3_GENERAL_H
#define ENC_MP3_GENERAL_H

#ifndef ASM
#include <stdio.h>
#include <vstypes.h>
#include <encoder.h>
#endif /* !ASM */

#define MAX_GR 2
#define MAX_CH 2
#define MAX_WIN 3
#define MAX_REG 3
#define MAX_SCFSI 4
#define MAX_SFBL 22
#define MAX_SFBS 12

#ifndef ASM

struct EncMEmit {
  s_int16 prevFrameSize;	/* In bytes */
  s_int16 prevFrameFree;	/* In bytes */
  s_int16 currFrameSize;	/* In bytes */
  s_int16 currFrameFree;	/* In bytes */
  u_int16 rest;			/* For padding calculation, p. 22 */
  s_int16 bitrateIndex;
  s_int16 samplingFrequency;
  s_int16 paddingBit;
  s_int16 mode;
  s_int16 modeExtension;
  s_int16 mainDataBegin;
  s_int16 scfsi[MAX_CH][MAX_SCFSI];
  s_int16 part23Length[MAX_GR][MAX_CH];
  s_int16 bigValues[MAX_GR][MAX_CH];
  s_int16 count1[MAX_GR][MAX_CH];
  s_int16 globalGain[MAX_GR][MAX_CH];
  s_int16 scalefacCompress[MAX_GR][MAX_CH];
  s_int16 scalefacSLenBits[MAX_GR][MAX_CH][MAX_SCFSI];
  s_int16 windowSwitchingFlag[MAX_GR][MAX_CH];
  s_int16 blockType[MAX_GR][MAX_CH];
  s_int16 mixedBlockFlag[MAX_GR][MAX_CH];
  s_int16 tableSelect[MAX_GR][MAX_CH][MAX_REG];
  s_int16 subblockGain[MAX_GR][MAX_CH][MAX_WIN];
  s_int16 region0Count[MAX_GR][MAX_CH];
  s_int16 region1Count[MAX_GR][MAX_CH];
  s_int16 regionEnd[MAX_GR][MAX_CH][3];
  s_int16 preflag[MAX_GR][MAX_CH];
  s_int16 scalefacScale[MAX_GR][MAX_CH];
  s_int16 count1tableSelect[MAX_GR][MAX_CH];

  s_int16 scalefacL[MAX_GR][MAX_CH][MAX_SFBL];
  s_int16 scalefacS[MAX_GR][MAX_CH][MAX_SFBS][MAX_WIN];
  //  s_int16 noOfAncillaryBits;
  s_int16 hcod[MAX_GR][MAX_CH][576];
};

struct EncMQualitySettings {
  int sampleRateIndex;
  int bitRateNominal[2];
  double maxModPlus[2];
  int useLongBands;
  double longMean[22];
#if 0
  double shortMean[13];
#endif
};

#endif /* !ASM */



#define BITBUF_SIZE 2048

#ifndef ASM

struct EncMBits {
  u_int16 *bitBufP;
  s_int16 bitP;
  u_int16 bitBuf[BITBUF_SIZE];
};

struct EncMAnalysis {
  s_int16 winPhase;
  s_int16 bufShift[2];
  s_int32 tmpY[32];
  s_int16 y[32];
  s_int16 buf[2][512];
  s_int16 winBuf[512];
  s_int16 canShiftUp[2][16];
};

struct EncMQuantize {
  u_int16 scaleFacF[2][22];
};

struct EncMMain {
  s_int16 inSamples[2][32];
  s_int16 hybridIn[2][576];
  u_int16 analysisSh[2][18];
  s_int16 analysisMinSh;
};

struct EncMHybrid {
  s_int16 granuleMem[2][576];
  s_int16 granuleMemSh[2];
  s_int16 granuleMemCouldSh[2];
  s_int16 z[36];
};

struct EncoderMp3 {
  struct Encoder e;
  s_int16 granules;
  u_int16 format;	/* 0 = 1.0, 1 = 2.0, 2 = 2.5 */
  s_int16 wouldLikeMsStereo[MAX_GR];
  s_int16 forceBitrateIndex;
  double maxModPlus;
  s_int16 bitReservoirMask;
  s_int16 spi16;
  u_int32 kilobitSum;
  u_int32 frames;

  /* -- encMEmit.h + standard -- */
  struct EncMEmit em;
  /* -- encMBits.h -- */
  struct EncMBits b;
  /* -- encMQuality.h -- */
  struct EncMQualitySettings qs;
  /* -- encodermp3.c -- */
  struct EncMHybrid *hyb;
  struct EncMMain main;
  /* -- encMAnalysis.h -- */
  struct EncMAnalysis ana;
  /* -- EncMQuantize.h -- */
  struct EncMQuantize encMQuan;
};



struct BitRateInfo {
  u_int16 sampleRate;
  u_int16 bitReservoirSize;
  s_int16 kbps[16];
  s_int16 bytesPerFrame[16];
};

extern const struct BitRateInfo bitRateInfo[];

#endif /* !ASM */

#endif /* !ENCODER_MP3_H */
