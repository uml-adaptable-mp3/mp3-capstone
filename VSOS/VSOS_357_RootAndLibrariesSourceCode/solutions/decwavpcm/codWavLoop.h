#ifndef COD_WAV_LOOP_H
#define COD_WAV_LOOP_H

#include <codec.h>

struct WavLoop {
  u_int32 currentLoop;   /**< Clear by the caller before activation */
  u_int32 numberOfLoops; /**< How many loops to perform; clear to end looping */
  u_int16 flags;         /**< CFL_ flags */
  s_int32 startSeconds;  /**< Start point in seconds */
  s_int32 startSamples;  /**< Start point in samples after last second */
  s_int32 endSeconds;    /**< End point in seconds */
  s_int32 endSamples;    /**< End point in samples after last second */
  s_int16 extension[4];  /**< Clear by the caller before activation */
  /* -------- Upto 16 words of decoder-specific fields ------------- */
};

#endif /* COD_WAV_LOOP_H */
