/**
   \file matrixer.h Channel Matrixer interfaces. The matrixer makes
   a conversion between an M channel input and an N channel output.
   Also the location of the channels may be defined to a degree.
*/

#ifndef MATRIXER_H
#define MATRIXER_H

#include <stdio.h>
#include <vstypes.h>


/**
   Current version number. 8 MSBs contain version number, 8 LSBs size of
   the structure in words.

   Version number history:
   <OL>
	<LI>0x010A
   </OL>
*/
#define MATRIXER_VERSION 0x010A

/** This encompasses Dolby Digital and DTS without their newer extensions */
#define MAX_SOURCE_CHANNELS 6

/** Channel positions */
enum ChannelMatrix {
  cmUnknown,
  cmLeft,
  cmCenter,
  cmRight,
  cmRearRight,
  cmRearCenter,
  cmRearLeft,
  cmLFE
};



/** Standard Matrixer wrap-up structure */
struct Matrixer {
  /** Version number. 8 MSBs contain version number, 8 LSBs size of
      the structure in words. */
  u_int16 version;
  /** Create and allocate space for Matrixer. */
  struct Matrixer *(*Create)(void);
  /** Set parameters of Matrixer. Returns 0 if successful. */
  s_int16 (*Set)(struct Matrixer *mat,
		 s_int16 inChannels, const enum ChannelMatrix *inMatrix,
		 s_int16 outChannels, const enum ChannelMatrix *outMatrix,
		 u_int32 sampleRate);
  /** Matrix samples. \a n may 0 or between 2..MAX_FILTER_SAMPLES. The
      number of samples output must also obey these rules. If the
      matrix has a built-in delay, it may output a number of samples
      different from the number input, as long as the output does not
      exceed MAX_FILTER_SAMPLES. \a data must have space for the maximum
      possible amount of output samples which is
      MAX_FILTER_SAMPLES * \a outChannels. */
  s_int16 (*Matrix)(struct Matrixer *mat, s_int16 *data, s_int16 n);
  /** Free all resources allocated for Matrixer. */
  void (*Delete)(struct Matrixer *mat);
  /** Matrixer output sample rate. */
  u_int32 sampleRate;
  /** Maximum matrixer input channels. */
  u_int16 maxInChannels;
  /** Maximum matrixer output channels. */
  u_int16 maxOutChannels;
  /** Delay of the matrixer in samples. */
  u_int16 delay;
};


#endif /* !MATRIXER */
