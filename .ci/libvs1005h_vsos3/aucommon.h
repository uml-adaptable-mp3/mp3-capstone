#ifndef AU_COMMON_H
#define AU_COMMON_H

#define AIN_32BIT_B	0
#define AIN_AD23FLIP_B	1

#define AIN_32BIT (1<<AIN_32BIT_B)
#define AIN_AD23FLIP (1<<AIN_AD23FLIP_B)

#define AIN_WR_OFFSET		 0
#define AIN_RD_OFFSET		 1
#define AIN_BUF_OFFSET		 2
#define AIN_BUFSIZE_OFFSET	 3
#define AIN_MODIFIER_OFFSET	 4
#define AIN_FLAGS_OFFSET	 5
#define AIN_ERROR0_OFFSET	 6
#define AIN_ERROR1_OFFSET	 7
#define AIN_SAMPLE_COUNTERL_OFFSET	 8
#define AIN_SAMPLE_COUNTERH_OFFSET	 9
#define AIN_SAMPLE_RATEL_OFFSET	 10
#define AIN_SAMPLE_RATEH_OFFSET	 11
#define AIN_OVERFLOWL_OFFSET	12
#define AIN_OVERFLOWH_OFFSET	13
#define AIN_STRUCT_SIZE		14

#define AOUT_WR_OFFSET		 0
#define AOUT_RD_OFFSET		 1
#define AOUT_BUF_OFFSET		 2
#define AOUT_BUFSIZE_OFFSET	 3
#define AOUT_MODIFIER_OFFSET	 4
#define AOUT_FLAGS_OFFSET	 5
#define AOUT_ERROR0_OFFSET	 6
#define AOUT_ERROR1_OFFSET	 7
#define AOUT_SAMPLE_COUNTERL_OFFSET	 8
#define AOUT_SAMPLE_COUNTERH_OFFSET	 9
#define AOUT_SAMPLE_RATEL_OFFSET	10
#define AOUT_SAMPLE_RATEH_OFFSET	11
#define AOUT_UNDERFLOWL_OFFSET	12
#define AOUT_UNDERFLOWH_OFFSET	13
#define AOUT_STRUCT_SIZE	14

#ifndef ASM

struct AudioIn {
  s_int16 __mem_y *wr;	///< Buffer write pointer
  s_int16 __mem_y *rd;	///< Buffer read pointer
  u_int16 __mem_y *buf;	///< Buffer start
  u_int16 bufSize;	///< Buffer size
  u_int16 modifier;	///< Typically 0x8000 + bufSize - 1 */
  u_int16 flags;	///< Flags
  u_int16 error[2];	///< Dithering info in 16-bit mode
  u_int32 sampleCounter;///< Sample counter, for syncing
  u_int32 sampleRate;	///< Sample rate after downconversion
  u_int32 overflow;	///< Gets added to each time an overflow happens
};

struct AudioOut {
  s_int16 __mem_y *wr;	///< Buffer write pointer
  s_int16 __mem_y *rd;	///< Buffer read pointer
  u_int16 __mem_y *buf;	///< Buffer start
  u_int16 bufSize;	///< Buffer size
  u_int16 modifier;	///< Typically 0x8000 + bufSize - 1 */
  u_int16 flags;	///< Flags
  u_int16 error[2];	///< Dithering if 16-bit output
  u_int32 sampleCounter;///< Sample counter, for syncing
  u_int32 sampleRate;	///< Sample rate after downconversion
  u_int32 underflow;	///< Gets added to each time an underflow happens
};

#endif /* !ASM */


/* IOCTL definitions */
#define IOCTL_AUDIO_GET_OCHANNELS	200
#define IOCTL_AUDIO_SET_OCHANNELS	201
#define IOCTL_AUDIO_GET_ICHANNELS	202
#define IOCTL_AUDIO_SET_ICHANNELS	203
#define IOCTL_AUDIO_GET_ORATE		204
#define IOCTL_AUDIO_SET_ORATE		205
#define IOCTL_AUDIO_SET_ORATE_BLOCK	206
#define IOCTL_AUDIO_GET_IRATE		207
#define IOCTL_AUDIO_SET_IRATE		208
#define IOCTL_AUDIO_SET_IRATE_BLOCK	209
#define IOCTL_AUDIO_GET_BITS		210
#define IOCTL_AUDIO_SET_BITS		211
#define IOCTL_AUDIO_OPEN_INPUT		212
#define IOCTL_AUDIO_CLOSE_INPUT		213
#define IOCTL_AUDIO_SELECT_INPUT	214
#define IOCTL_AUDIO_GET_INPUT_BUFFER_SIZE	215
#define IOCTL_AUDIO_SET_INPUT_BUFFER_SIZE	216
#define IOCTL_AUDIO_GET_INPUT_BUFFER_FILL	217
#define IOCTL_AUDIO_GET_OUTPUT_BUFFER_SIZE	218
#define IOCTL_AUDIO_SET_OUTPUT_BUFFER_SIZE	219
#define IOCTL_AUDIO_GET_OUTPUT_BUFFER_FREE	220
/* Added for VSOS 0.24 2013-07-16 */
#define IOCTL_AUDIO_GET_OUTPUTS		221
#define IOCTL_AUDIO_SET_OUTPUTS		222
#define IOCTL_AUDIO_GET_SPDIF_VOLUME	223
#define IOCTL_AUDIO_SET_SPDIF_VOLUME	224
/* Added for VSOS 3.07 2014-04-17 */
#define IOCTL_AUDIO_GET_I2S		225
#define IOCTL_AUDIO_SET_I2S		226
/* Added for VSOS 3.23 2015-03-27 */
#define IOCTL_AUDIO_GET_VOLUME		227
#define IOCTL_AUDIO_SET_VOLUME		228
#define IOCTL_AUDIO_SET_RATE_AND_BITS	229
#define IOCTL_AUDIO_GET_SAMPLE_COUNTER	230
#define IOCTL_AUDIO_GET_OVERFLOWS	231
#define IOCTL_AUDIO_GET_UNDERFLOWS	232
/* Added for VSOS 3.23 2015-08-26 */
#define IOCTL_AUDIO_GET_EQU_FILTER	233
#define IOCTL_AUDIO_SET_EQU_FILTER	234
#define IOCTL_AUDIO_GET_EQU_MAX_FILTERS	235
/* Added for VSOS 3.23 2015-09-03 */
#define IOCTL_AUDIO_GET_DC_BLOCK	240 /* More info below and in <agc.h> */
#define IOCTL_AUDIO_SET_DC_BLOCK	241
#define IOCTL_AUDIO_GET_AGC32		250 /* More info in <agc.h> */
#define IOCTL_AUDIO_SET_AGC32		251
/* Added for VSOS 3.26 2016-02-02 */
#define IOCTL_AUDIO_AUTOVOL		260
/* Added for VSOS 3.42 2017-03-14 */
#define IOCTL_AUDIO_GET_PITCH		270
#define IOCTL_AUDIO_SET_PITCH		271
#define IOCTL_AUDIO_GET_SPEED		272
#define IOCTL_AUDIO_SET_SPEED		273


#define FLT_EQUF_LEFT	1
#define FLT_EQUF_RIGHT	2

#define DC_BLOCK_HIFI	0x4000U /* -1 dB point below 20 Hz; subsonic filter   */
#define DC_BLOCK_SPEECH	0x8000U /* -3 dB point just above 100 Hz              */
#define DC_BLOCK_FORCE	0xC000U /* Set shift value by hand; use only if sure! */
#define DC_BLOCK_AUTO	0x0000U /* Dynamic filter using sample rate as control*/
                                /*   - 8 kHz = SPEECH                         */
                                /*   - 48 kHz = HIFI                          */
                                /*   - Rates between these automatically      */
                                /*     calculated.                            */

#define AUTOVOL_RESET	0x1000U /* Reset AutoVol; do each time a song with unknown volume starts. */
#define AUTOVOL_GET	0x2000U /* Get current AutoVol gain value. Store for each song when playback ends. */
#define AUTOVOL_SET	0x3000U /* Set AutoVol to known value; do each time such a song starts that you know the song volume for. */
#define AUTOVOL_PAUSE	0x4000U /* Pause AutoVol. */


#ifndef ASM

struct FilterEqualizer {
  s_int16 filterNumber;	/* 0 .. MAX_FILTERS-1 */
  u_int16 flags;	/* FLT_EQUF_ flags */
  double centerFrequencyHz;
  double gainDB;        /* Recommended -12.0 .. +12.0 dB */
  double qFactor;       /* Recommended 0.1 .. 4.0 (higher is steeper) */
};

#endif /* !ASM */

/* For selecting input in ADC driver */
#define AID_I2S_NOT_IN_USE         0x0001U
#define AID_SPDIF_NOT_IN_USE       0x0002U

/* Left:   Digital AD1 input, pin 53 */
#define AID_DIA1	0x0004U
/* Right:  Digital AD2 input, pin 55 */
#define AID_DIA2	0x0008U
/* Right:  Digital AD3 input, pin 51 */
#define AID_DIA3	0x0010U

/* Stereo: FM demodulation, pins 75,76 */
#define AID_FM		0x0020U

/* Left:   LINE1_1, pin 73 */
#define AID_LINE1_1	0x0040U
/* Left:   LINE2_1, pin 68 */
#define AID_LINE2_1	0x0080U
/* Left:   LINE3_1, pin 71 */
#define AID_LINE3_1	0x0100U
/* Left:   MIC1, pins 72,73 */
#define AID_MIC1	0x0200U

/* Right:  LINE1_2, pin 72 */
#define AID_LINE1_2	0x0400U
/* Right:  LINE2_2, pin 67 */
#define AID_LINE2_2	0x0800U
/* Right:  LINE3_2, pin 70 */
#define AID_LINE3_2	0x1000U
/* Right:  MIC2, pins 70,71 */
#define AID_MIC2	0x2000U

/* Right:  LINE1_3, pin 69 */
#define AID_LINE1_3	0x4000U

/* Force decimate-by-6 hardware */
#define AID_DEC6	0x8000U



#endif /* !AU_COMMON_H */
