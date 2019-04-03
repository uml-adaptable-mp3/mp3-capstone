/**
   \file filter.h Filter definition.
*/

#ifndef FILTER_H
#define FILTER_H

#include <vstypes.h>
#ifndef ASM
#include <lists.h>
#endif /* !ASM */

/**
   Current version number. 8 MSBs contain version number, 8 LSBs size of
   the structure in words.

   Version number history:
   <OL>
	<LI>0x0212
   </OL>
*/
#define FILTER_VERSION 0x010A


/**
   A filter doesn't need to be able to receive more than this number
   of stereo samples. The required number of samples to handle is
   0..MAX_FILTER_SAMPLES.
 */
#define MAX_FILTER_SAMPLES 64


#ifndef ASM
/**
   Standard filter functions.
 */
struct FilterFunctions {
  /**
     Create filter. This funtion allocates the public and private
     resources needed for a filter and returns a pointer to a Filter
     structure. If allocation fails, NULL is returned.
   */
  struct Filter *(*Create)(void);
  /**
     Set filter parameters.
     \a handle is a Filter handle as returned earlier by Create().
     \a maxGain is the maximum amount of gain that can be added to the filter
	chain later, in 1/2 dB steps. Example: 7 means that the later
	filter chain is capable of adding 3.5 dB to the gain.
     \a sampleRate is the sample rate that is provided to the filter.
     \a params, which is a varargs list contains filter-specific parameters.
     Returns 0 if successful.
  */
  s_int16 (*Set)(struct Filter *handle, s_int16 maxGain,
		 u_int32 sampleRate, ...);
  /**
     Filters audio.
     \a handle is a Filter handle.
     \a data is 16-bit interleaved stereo data.
	The filter may be destructive.
     \a path is the number of the filter input path.
     \a n is the number of input stereo samples. n is either 0 or between
	2 and MAX_FILTER_SAMPLES stereo samples.
	Filter() returns the number of stereo samples it has created,
	which in many cases is the same as \a n. It also has the same
	restrictions as \a n. If a filter needs to output
	more than MAX_FILTER_SAMPLES samples, it will negate its result,
	and will be later called again with \a n = 0 until it returns a
	non-negative value.
     The function returns the gain value.
  */
  s_int16 (*Filter)(register __i0 struct Filter *handle,
		    register __i1 s_int16 *data,
		    register __a1 s_int16 path,
		    register __a0 s_int16 n);
  /**
     A filter may have upto four outputs, depending on the filter.
     After creating a filter, the user needs to fill in the number of
     filter outputs required by the filter.
  */
  s_int16 (*Output[4])(register __i0 struct Filter *handle,
		       register __i1 s_int16 *data,
		       register __a1 s_int16 path,
		       register __a0 s_int16 n);
  /**
     The path parameter for filter outputs. If not set by the user,
     path defaults to 0.
  */
  s_int16 outputPath[4];
  /**
     Deletes a filter and unallocates its resources.
     \a handle is a Filter handle.
  */
  void (*Delete)(struct Filter *handle);
};


/**
   Filter structure. This should be the beginning of every user
   filter.
 */
struct Filter {
  /** Version number. 8 MSBs contain version number, 8 LSBs size of
      the structure in words. */
  u_int16 version;
  /** Output sample rate of this filter. */
  u_int32 outSampleRate;
  /**
     Maximum amount of gain that can be added
     to the filter
     chain later, in 1/2 dB steps. Example: 7 means that the later
     filter chain is capable of adding 3.5 dB to the gain.
  */
  s_int16 gain;
  /** Mean delay time in stereo samples. */
  s_int16 meanDelay;
  /** Maximum delay time in stereo samples. */
  s_int16 maxDelay;
  /** Standard function pointers */
  struct FilterFunctions func;
};

#endif /* !ASM */

#endif /* FILTER_H */
