#ifndef DEV_AUDIO_H
#define DEV_AUDIO_H

#define ADC_CHANNELS 3
#define DEC6_CHANNELS 2
#define ADC_DEC6_CHANNELS (ADC_CHANNELS+DEC6_CHANNELS) /**< 3xA/D channels + 2xDec6 channels for FM */

#define ADCPTR_CHANNELS	0
#define ADCPTR_ADCCHAN	4
#define ADCPTR_WITHOUT_ADCCHAN_SIZE     4

#if 0
#define ADCCHAN_WR		 0
#define ADCCHAN_RD		 1
#define ADCCHAN_MODIFIER	 2
#define ADCCHAN_BIT24		 3
#define ADCCHAN_OVERFLOW	 4
#define ADCCHAN_ERROR		 5
#define ADCCHAN_VOL		 6
#define ADCCHAN_SAMPLERATE	 7
#define ADCCHAN_DECIMATE	 9
#define ADCCHAN_BUF		10
#define ADCCHAN_BUFSIZE		11
#else
#define ADCCHAN_WR		 0
#define ADCCHAN_RD		 1
#define ADCCHAN_BUF		 2
#define ADCCHAN_BUFSIZE		 3
#define ADCCHAN_MODIFIER	 4
#define ADCCHAN_FLAGS		 5
#define ADCCHAN_ERROR0		 6
#define ADCCHAN_ERROR1		 7
#define ADCCHAN_VOL0		 8
#define ADCCHAN_VOL1		 9
#define ADCCHAN_OVERFLOW	10
#define ADCCHAN_SAMPLERATE	11
#define ADCCHAN_DECIMATE	13
#define ADCCHAN_DUMMY		14
#endif

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

#define AOR_DAC		0x0001U
#define AOR_I2S		0x0002U
#define AOR_SPDIF	0x0004U

#ifndef ASM

#include "vsos.h"

/* hardwareInfo[6] filled by this struct, perhaps also use deviceInfo[16]? */
typedef struct devAudioHardwareInfoStruct {
  u_int32 outputSampleRate;
  u_int32 inputSampleRate;
  u_int16 oResources;
} devAudioHwInfo;

ioresult DevAudioCreate(register __i0 DEVICE *dev, const void *name, u_int16 extraInfo);
u_int16 DevAudioRead(register __i0 VO_FILE *f, void *buf, u_int16 destinationIndex, u_int16 bytes);
u_int16 DevAudioWrite(register __i0 VO_FILE *f, void *buf, u_int16 sourceIndex, u_int16 bytes);	
ioresult DevAudioDelete(register __i0 VO_FILE *f);
const char* DevAudioIdentify(register __i0 void *obj, char *buf, u_int16 bufsize);
ioresult DevAudioIoctl(register __i0 void *self, s_int16 request, char *argp);

#endif /* !ASM */

#define ADCHAN_FLAGS_32BIT	   1
#define ADCHAN_FLAGS_USES_AD23FLIP 2
#define ADCHAN_FLAGS_ACTIVE	   4

#ifndef ASM

struct AdcChannel {
  s_int16 __y *wr;	///< Buffer write pointer
  s_int16 __y *rd;	///< Buffer read pointer
  u_int16 __y *buf;	///< Buffer start
  u_int16 bufSize;	///< Buffer size
  u_int16 modifier;	///< e.g. 0x8000 + size - 1 */
  u_int16 flags;	///< Flags
  u_int16 error[2];	///< Dithering info in 16-bit mode
  u_int16 vol[2];	///< Recording volume or max. AGC gain if bit 15 is set
  u_int16 overflow;	///< Gets added each time an overflow is encountered
  u_int32 samplerate;	///< Samplerate after downconversion
  u_int16 decimate;	///< Decimation ratio
  u_int16 dummy[2];	///< To keep the size of this structure at 16 words
};

#endif /* !ASM */

/* Following offsets are relatice to Src structure */
#define SRC_WR_OFFSET		0
#define SRC_RD_OFFSET		1
#define SRC_MODIFIER_OFFSET	2
#define SRC_VOLUMEDB_OFFSET	3
#define SRC_VOLUMELIN_OFFSET	4

#define SRC_BUFFER_SIZE		32

#ifndef ASM

struct Src {
  s_int16 *wr;		///< Buffer read pointer for SRC interrupt
  s_int16 *rd;		///< Buffer write pointer for S/PDIF interrupt
  s_int16 modifier;	///< Modifier register
  u_int16 spdifVolHalfDb;///< SPDIF attenuation in 1/2 dB steps (20 = -10 dB)
  u_int16 spdifVolLin;	///< SPDIF volume in linear format (32768 = 0 dB)
};

extern __align s_int16 srcBuf[SRC_BUFFER_SIZE];
extern struct Src src;

struct Adc {
  u_int16 resources;	///< Bit mask of total resources that are used
  u_int16 ad23Flip;	///< How many channels use ad23Flip bit
  u_int16 flags;
  u_int16 dummy;	///< Place holder
  struct AdcChannel adc[AD_INTS]; /// Channel-specific data
};

extern const u_int16 __y adInts[AD_INTS];

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

#define AID_I2S         0x0001U
#define AID_SPDIF	0x0002U
#define AID_DIA1	0x0004U
#define AID_DIA2	0x0008U
#define AID_DIA3	0x0010U
#define AID_FM		0x0020U
#define AID_LINE1_1	0x0040U
#define AID_LINE2_1	0x0080U
#define AID_LINE3_1	0x0100U
#define AID_MIC1	0x0200U
#define AID_LINE1_2	0x0400U
#define AID_LINE2_2	0x0800U
#define AID_LINE3_2	0x1000U
#define AID_MIC2	0x2000U
#define AID_LINE1_3	0x4000U
#define AID_DEC6	0x8000U

struct AudioChanToIdPattern {
  const char *name;
  u_int16 id;
};

#define AID_REGS 7

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


extern struct Adc adc;

#endif /* !ASM */

#endif /* !DEV_AUDIO */
