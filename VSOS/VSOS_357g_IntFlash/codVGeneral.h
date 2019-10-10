/**
   \file codVGeneral.h General Vorbis definitions.
*/

#ifndef COD_VORBIS_GENERAL_H
#define COD_VORBIS_GENERAL_H

#include <vstypes.h>
#include <codec.h>
#include <filter.h>
#include "codecvorbis.h"



/* Vorbis definitions */
#define MAX_AUDIO_CHANNELS 2
#define MAX_SAMPLES_PER_BLOCK 4096
#define MAX_SAMPLES_PER_2 (MAX_SAMPLES_PER_BLOCK/2)


/* General definitions */
#define WORDMASK(b)             (((ULL)-1)>>(sizeof(ULL)*8-(b)))


/** Maximum number of channels handled by the VORBIS codec. */
#define COD_VORBIS_MAX_CHANNELS 2

#ifndef ASM

/**
   Vorbis Codec extensions to the basic Codec structure.
   Doesn't yet allow re-entrancy.
*/
struct CodecVorbis {
  /** Basic Codec structure. */
  struct Codec c;
  /** Temporary flag until re-entrancy is corrected. */
  s_int16 used;
  /** Internal variables */
  u_int16 state;
  /* ---------------- setup - -------------------- */
  u_int16 audioChannels;
  u_int32 audioSampleRate;
  u_int32 bitRateMaximum;
  u_int32 bitRateNominal;
  u_int32 bitRateMinimum;
  u_int16 blockSize[2];
  const u_int16 __mem_y *window[2];
  u_int16 codeBooks;
  struct CodeBook *codeBook;
  u_int16 floors;
  struct Floor1 __mem_y *floor;
  u_int16 residues;
  struct Residue *residue;
  u_int16 maps;
  struct Map *map;
  u_int16 modes;
  struct Mode *mode;
  /* ---------------- decode -------------------- */
  /** Mode number */
  u_int16 modeNumber;
  /** Type of previous window */
  s_int16 prevWinType;
  /** Byte offset to the point where actual AUDIO data begins */
  u_int32 audioBegins;
  /** How many bits downshifted for last frame. */
  s_int16 oldFrameDownshift[COD_VORBIS_MAX_CHANNELS];
  /** How many bits could have been downshifted for last frame. */
  s_int16 oldFramePotentialDownshift[COD_VORBIS_MAX_CHANNELS];
};



/* Integers */
#ifdef __VSDSP__
typedef s_int16 sample_t;
#else
typedef s_int32 sample_t;
#endif

#endif /* !ASM */

#endif /* !COD_WAV_GENERAL_H */
