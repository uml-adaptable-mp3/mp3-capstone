/**
   \file encoder.h Encoder interfaces.
*/

#ifndef ENCODER_H
#define ENCODER_H

#include <vstypes.h>
#ifndef ASM
#include "matrixer.h"
#endif /* !ASM */

/**
   Current version number. 8 MSBs contain version number, 8 LSBs size of
   the structure in words.

   Version number history:
   <OL>
	<LI>0x010f First version
   </OL>
*/
#define ENCODER_VERSION 0x010f

/**
   Current version number. 8 MSBs contain version number, 8 LSBs size of
   the structure in words.

   Version number history:
   <OL>
	<LI>0x010e First version
   </OL>
*/
#define FS_ENCODER_SERVICES_VERSION 0x010e


#ifndef ASM

/**
   Encoder Services structure. The caller must provide values for
   elements \a Read and \a Output. Other fields should be cleared
   prior to opening an encoder.
   The caller may also set \a cancel for control operations.
 */
struct EncoderServices {
  /** Version number. 8 MSBs contain version number, 8 LSBs size of
      the structure in words. */
  u_int16 version;
  /** Read audio data from file. \a chan * \a samples audio samples are read,
      and values are written to vectors ptr[0..chan-1].
      Return value is the number of complete samples read,
      which should be the same as samples unless there is an error. */
  u_int16 (*Read)(struct EncoderServices *es, s_int16 **ptr, u_int16 samples,
		  u_int16 chan);
  /** Output to stream.
      \a data is a pointer to the data. \a bytes is the data size in bytes.
      If an odd number, the LSB of the last \a data word is not used. */
  s_int16 (*Output)(struct EncoderServices *es, u_int16 *data, s_int16 bytes);
  /** Request encoder to cancel recording.
      To request cancellation, set this to a positive value. When the
      encoder has finished cancelling, it will clear this value and
      return eeCancelled. */
  s_int16 cancel;
  /** Playback time from beginning of file in seconds.
      Updated by the encoder. */
  s_int32 playTimeSeconds;
  /** Samples played since last full second. Updated by the encoder. */
  s_int32 playTimeSamples;
  /** Average bitrate. Updated by the encoder. May change during playback. */
  u_int32 avgBitRate;
  /** Current bitrate. Updated by the encoder. May change during playback. */
  u_int32 currBitRate;
  /** Peak bitrate of the file. Updated by the encoder.
      May change during playback. */
  u_int32 peakBitRate;
};





/** Encoder error codes. */
enum EncoderError {
  eeCancel = -1,	/**< Cancelling requested, error. */
  eeOk = 0,		/**< End of file, no errors. */
  eeOtherError		/**< Unspecific error */
};


#define ENCODER_QUALITY_QUALITY 0x0000U
#define ENCODER_QUALITY_VBR     0x4000U
#define ENCODER_QUALITY_ABR     0x8000U
#define ENCODER_QUALITY_CBR     0xC000U
#define ENCODER_QUALITY_MULT10  0x0000U
#define ENCODER_QUALITY_MULT100 0x1000U
#define ENCODER_QUALITY_MULT1K  0x2000U
#define ENCODER_QUALITY_MULT10K 0x3000U

/** Standard Encoder wrap-up structure */
struct Encoder {
  /** Version number. 8 MSBs contain version number, 8 LSBs size of
      the structure in words. */
  u_int16 version;
  /** Create and allocate space for encoder. mode = quality[15:14],
      mult = quality[13:12], param = quality[11:10].
      If mode = 0, quality = param (limited betweem 0..10, 10 is best).
      if mode = 1 (VBR), 2 (ABR) or 3 (CBR), suggested bitrate =
      10^(mult+1). E.g. 0xE080 = CBR 128*1000 bit/s = CBR 128 kbit/s.
      Encoders are at freedom of implementing only some of the VBR, ABR
      and CBR options and use their best equivalents for missing modes. */
  struct Encoder *(*Create)(struct EncoderServices *es, u_int16 channels,
			    u_int32 sampleRate, u_int16 quality);
  /** Encode file. With zero or negative return value number,
      Encoder has succeeded. With a positive number, there has been an error.
      Upon return, an error string is also returned. */
  enum EncoderError (*Encode)(struct Encoder *enc, const char **errorString);
  /** Free all resources allocated for encoder. */
  void (*Delete)(struct Encoder *enc);
  /** A pointer that the encoder updates and uses for file I/O. */
  struct EncoderServices *es;
  /** Sample rate. Updated by the encoder. */
  u_int32 sampleRate;
  /** Number of channels. Updated by the encoder. */
  u_int16 channels;
  /** Quality setting. Updated by the encoder. */
  u_int16 quality;
  /** Channel matrix. Updated by the user if a special matrix is needed. */
  enum ChannelMatrix matrix[MAX_SOURCE_CHANNELS];
};

#endif /* !ASM */

#define EC_VERSION_OFFSET		 0
#define EC_READ_OFFSET			 1
#define EC_OUTPUT_OFFSET		 2
#define EC_CANCEL_OFFSET		 3
#define EC_PLAY_TIME_SECONDS_OFFSET	 4
#define EC_PLAY_TIME_SAMPLES_OFFSET	 6
#define EC_AVG_BIT_RATE_OFFSET		 8
#define EC_CURR_BIT_RATE_OFFSET		10
#define EC_PEAK_BIT_RATE_OFFSET		12

#define ENCODER_VERSION_OFFSET		0
#define ENCODER_CREATE_OFFSET		1
#define ENCODER_ENCODE_OFFSET		2
#define ENCODER_DELETE_OFFSET		3
#define ENCODER_EC_OFFSET			4
#define ENCODER_EC_SAMPLE_RATE_OFFSET	5
#define ENCODER_EC_CHANNELS_OFFSET	7
#define ENCODER_EC_MATRIX_OFFSET		8 /* 8..13 */

#endif /* !ENCODER_H */
