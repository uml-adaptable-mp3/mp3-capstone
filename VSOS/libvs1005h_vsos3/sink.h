/**
   \file sink.h Sink interface. A Sink writes the output data of an
   audio path to a destination, be it a file or a hardware buffer.
*/

#ifndef SINK_H
#define SINK_H

#include <stdio.h>
#include <vstypes.h>


/**
   Current version number. 8 MSBs contain version number, 8 LSBs size of
   the structure in words.

   Version number history:
   <OL>
	<LI>0x010B
   </OL>
*/
#define SINK_VERSION 0x010B


/** Standard Sink wrap-up structure */
struct Sink {
  /** Version number. */
  u_int16 version;
  /** Create and allocate space for Sink. */
  struct Sink *(*Create)(void *param, s_int16 chan, s_int32 sampleRate,
                         u_int32 samples);
  /** Write samples. \a n may 0 or between 2..MAX_FILTER_SAMPLES. */
  s_int16 (*Write)(struct Sink *snk, s_int16 *data, s_int16 n);
  /** Free all resources allocated for Sink. */
  s_int16 (*Delete)(struct Sink *snk);
  /** Sample rate. */
  u_int32 sampleRate;
  /** Channels */
  u_int16 channels;
  /** Total number of samples */
  u_int32 samples;
  /** Samples left */
  u_int32 samplesLeft;
};


#endif /* !MATRIXER */
