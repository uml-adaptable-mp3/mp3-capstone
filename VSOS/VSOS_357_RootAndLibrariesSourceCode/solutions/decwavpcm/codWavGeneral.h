/**
   \file codWavGeneral.h General WAV definitions.
*/

#ifndef COD_WAV_GENERAL_H
#define COD_WAV_GENERAL_H

#include <vstypes.h>
#include <codec.h>
#include <filter.h>

/** Maximum number of channels handled by the WAV codec. */
#define COD_WAV_MAX_CHANNELS 2
#define COD_WAV_FF_FADE_IN_POW 4
#define COD_WAV_FF_FADE_IN_SIZE (1<<COD_WAV_FF_BUFFER_POW)
#define COD_WAV_FF_WINDOW_SIZE 512

#ifndef ASM

/** Known WAV coding methods */
enum WavCoding {
  wcUnknown=0,
  wcLinear,
  wcIma,
  wcFloat,
  wcALaw,
  wcULaw
};

/**
   Wav Codec extensions to the basic Codec structure.
   Doesn't yet allow re-entrancy.
*/
struct CodecWav {
  /** Basic Codec structure. */
  struct Codec c;
  /** Temporary flag until re-entrancy is corrected. */
  s_int16 used;
  /** Number of bytes left in the RIFF file. */
  u_int32 riffLeft;
  /** Wav coding method. */
  enum WavCoding coding;
  /** Bits per sample. */
  u_int16 bitsPerSample;
  /** Number of n-channel samples in the file. Only required by IMA. */
  u_int32 samples;
  /** IMA block length in bytes (per channel) */
  u_int16 imaBlockLen;
  /** IMA samples per block (per channel) */
  u_int16 imaSamplesPerBlock;
  /** Temporary buffer storage */
  u_int16 buf[MAX_FILTER_SAMPLES*COD_WAV_MAX_CHANNELS];
  /** Position in file where audio data started. Used for seek operations. */
  u_int32 dataStartPos;
  /** Sample rate of the file. */
  u_int32 fileSampleRate;
  /** Current play time in seconds before downsampling. */
  u_int32 realPlayTimeSeconds;
  /** Current play time samples after last full second before downsampling. */
  u_int32 realPlayTimeSamples;
  /** Phase when fast forwarding */
  s_int32 fastForwardPhase;
  /** Fast forward fade in/out counter (in = positive, out = negative) */
  s_int16 fadeIn;
};

#endif /* !ASM */


/** PCM format */
#define	WAVE_FORMAT_PCM			0x0001 
/** Basic ADPCM (not supported) */
#define	WAVE_FORMAT_ADPCM		0x0002
/** Floating point 32-bit supported */
#define	WAVE_FORMAT_IEEE_FLOAT		0x0003
/** A-Law (not supported) */
#define	WAVE_FORMAT_ALAW		0x0006
/** Mu-Law (now supported) */
#define	WAVE_FORMAT_MULAW		0x0007
/** Oki ADPCM (not supported) */
#define	WAVE_FORMAT_OKI_ADPCM		0x0010
/** IMA ADPCM */
#define	WAVE_FORMAT_IMA_ADPCM		0x0011
/** Don't know (not supported) */
#define	WAVE_FORMAT_DIGISTD		0x0015
/** Don't know (not supported) */
#define	WAVE_FORMAT_DIGIFIX		0x0016
#define WAVE_FORMAT_G721_ADPCM          0x0040
#define WAVE_FORMAT_MPEG2		0x0050
#define WAVE_FORMAT_MP3			0x0055
#define WAVE_FORMAT_G722_ADPCM          0x0065
#define WAVE_FORMAT_AAC			0x00FF
#define	IBM_FORMAT_MULAW         	0x0101
#define	IBM_FORMAT_ALAW			0x0102
#define	IBM_FORMAT_ADPCM         	0x0103
#define WAVE_FORMAT_DOLBY_AC3		0x2000
#define WAVE_FORMAT_DVD_DTS		0x2001
#define WAVE_FORMAT_FLAC		0xF1AC
/** PCM format with extensions, used for 24-bit and 32-bit audio */
#define WAVE_FORMAT_PCMEX		0xFFFE


#ifndef ASM

/** Creates an ID number from the four parameters */
#define MAKE_ID(a,b,c,d) (((u_int32)(a)<<24)|((u_int32)(b)<<16)|((c)<<8)|(d))
/** Minimum of two numbers */
#define MIN(a,b) (((a)<(b))?(a):(b))

/** Riff 'f','m','t',' ' TAG format. This is a little-endian format. */
struct CodecWavRiffFormatTag {
  u_int16 formatTag;		/**< WAV subformat */
  u_int16 channels;		/**< Number of audio channels */
  u_int32 samplesPerSecond;	/**< Sample rate */
  u_int32 avgBytesPerSec;	/**< Average data rate */
  u_int16 blockAlign;		/**< Block align if format is block oriented */
  u_int16 bitsPerSample;	/**< Number of bits per audio sample */
};

u_int32 CodWavRead32(register struct CodecServices *cs, register u_int16 *err);
u_int32 CodWavRead32I(register struct CodecServices *cs, register u_int16 *err);
void CodWavOutput(register struct CodecWav *cw,
		  register struct CodecServices *cs,
		     register s_int16 *data, register s_int16 n);

#endif /* !ASM */

#endif /* !COD_WAV_GENERAL_H */
