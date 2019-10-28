#ifndef AUIADC_H
#define AUIADC_H

#include <vstypes.h>
#include <aucommon.h>

#define ADCCHAN_SIZE 16

#define AD_INT_MAC0	0
#define AD_INT_MAC1     1
#define AD_INT_MAC2	2
#define AD_INT_I2S	3
#define AD_INT_SRC	4
#define AD_INT_SPDIF	5
#define AD_INTS		6

#define AIR_MAC0	(1U << AD_INT_MAC0)
#define AIR_MAC1	(1U << AD_INT_MAC1)
#define AIR_MAC2	(1U << AD_INT_MAC2)
#define AIR_I2S		(1U << AD_INT_I2S)
#define AIR_SRC		(1U << AD_INT_SRC)
#define AIR_SPDIF	(1U << AD_INT_SPDIF)
#define AIR_LINE1_1	0x0040U
#define AIR_MIC1N	0x0040U
#define AIR_LINE2_1	0x0080U
#define AIR_AUX1	0x0080U
#define AIR_LINE3_1	0x0100U
#define AIR_MIC2N	0x0100U
#define AIR_AUX4	0x0100U
#define AIR_LINE1_2	0x0200U
#define AIR_MIC1P	0x0200U
#define AIR_LINE2_2	0x0400U
#define AIR_AUX0	0x0400U
#define AIR_LINE3_2	0x0800U
#define AIR_MIC2P	0x0800U
#define AIR_AUX3	0x0800U
#define AIR_LINE1_3	0x1000U
#define AIR_AUX2	0x1000U
#define AIR_AD23	0x8000U

#define AIF_LEFT		0x0001
#define AIF_RIGHT		0x0002
#define AIF_RIGHT_FLIPPED	0x0004
#define AIF_CENTER		0x0008
#define AIF_CENTER_FLIPPED	0x0010
#define AIF_STEREO		0x0020
#define AIF_AD23FLIP		0x0040
#define AIF_CAN_USE_DEC6	0x0080
#define AIF_NEEDS_DEC6		0x0100
#define AIF_IS_DEC6		0x0200

#define AID_REGS 7


#ifndef ASM

struct Adc {
  s_int16 iChannel;
  u_int16 iResources;
};

extern struct Adc adc;

extern const u_int16 __mem_y adInts[AD_INTS];

extern struct AudioIn audioIn;
extern struct AudioOut audioOut;
extern SIMPLE_FILE audioFile;

auto ioresult AudioSetInput(register u_int16 idPattern);
auto void AudioCloseInput(void);
auto void AudioClose(void);
auto ioresult AudioSetRate(register s_int32 sampleRate,
			     register u_int16 hiBits);



struct AudioChanToIdPattern {
  const char *name;
  u_int16 id;
};

struct AudioIdReg {
  u_int16 addr;	// If MSb is 0, then this is for flipped operation + skip next
  u_int16 mask;
  u_int16 val;
};

struct AudioIdPattern {
  u_int16 flags;
  u_int16 resources;
  u_int16 interrupt;
  struct AudioIdReg startReg[AID_REGS];
};

#endif /* !ASM */

#endif /* !AUIADC_H */
