/**
   \file decodeAudio.h General audio decoding library.
   The audio decoding library takes care of creating an audio decoder
   for an audio file. As of February 2015 it can decode several audio
   formats, including but not limited to MP3, Ogg Vorbis, Flac, RIFF WAV,
   AAC LC, and WMA.
*/
#ifndef LIB_AUDIO_DEC_H
#define LIB_AUDIO_DEC_H

#include <vo_stdio.h>
#include <codec.h>

/** Hint for decoding format. If unknown, use auDecFGuess. */
enum AuDecFormat {
  /** Use this value if you want the decoder to determine your file format,
      and if the file is seekable. */
  auDecFGuess,
  /** Audio is in MP3 format. */
  auDecFMp3,
  /** Audio is in WAV format. */
  auDecFWav,
  /** Audio is in Ogg Vorbis format. */
  auDecFOgg,
  /** Audio is in FLAC format. */
  auDecFFlac,
  /** Audio is in ALAC format. */
  auDecFAlac,
  /** Audio is in WMA format. */
  auDecFWma,
  /** Audio is in AAC format. */
  auDecFAac,
  /** Audio is in MIDI format. */
  auDecFMidi,
  /** Audio is in DSD format. */
  auDecFDsd,
  /** Audio is in OPUS format. */
  auDecFOpus,
  /** Audio is in AIFF format. */
  auDecFAiff,
  /** Audio is in either AAC of ALAC format (unknown at the time) */
  auDecFAacOrAlac = 256,
  /** ID3v2 header, not actual audio file */
  auDecFId3v2
};

struct AudioDecoder;
typedef struct AudioDecoder AUDIO_DECODER;

#define AUDECF_PLAYING_B 0
#define AUDECF_STOPPED_B 1

#define AUDECF_PLAYING (1<<AUDECF_PLAYING_B)
#define AUDECF_STOPPED (1<<AUDECF_STOPPED_B)

/** The audio decoder structure contains all the elements needed for the
    decoder to do its task. */
struct AudioDecoder {
  struct CodecServices cs;
  VO_FILE *inFp;
  VO_FILE *outFp;
  struct Codec *cod;
  s_int16 pause;
  u_int32 sampleRate;
  u_int16 channels;
  void (*callback)(AUDIO_DECODER *auDec, u_int16 samples);
  enum auDecFormat format;
  u_int16 *decoderLib;
  u_int16 wordsPerSample;
  u_int16 flags;
};

/** Create an audio decoder. */
#if 0
auto AUDIO_DECODER *CreateAudioDecoder(register FILE *inFp,
				       register FILE *outFp,
				       register void (*callback)
				       (AUDIO_DECODER *dec, u_int16 samples),
				       register enum AuDecFormat auDecFormat);
#else
#define CreateAudioDecoder(lib, inFp, outFp, callback, auDecFormat) ( ((AUDIO_DECODER *(*)(register FILE *, register FILE *, register void (*)(AUDIO_DECODER *, u_int16), register enum AuDecFormat))(*((u_int16 *)(lib)+2+(ENTRY_1))))(inFp, outFp, callback, auDecFormat) )
#endif

/** Delete an audio decoder. */
#if 0
int DeleteAudioDecoder(register AUDIO_DECODER *dec);
#else
#define DeleteAudioDecoder(lib, dec) ( ((int (*)(register AUDIO_DECODER *))(*((u_int16 *)(lib)+2+(ENTRY_2))))(dec) )
#endif

/** Decode audio using the audio decoder earlier created with
    CreateAudioDecoder(). */
#if 0
u_int16 DecodeAudio(register AUDIO_DECODER *dec,
			 register const char **eStr);
#else
#define DecodeAudio(lib, dec, eStr) ( ((u_int16 (*)(register AUDIO_DECODER *, register const char **))(*((u_int16 *)(lib)+2+(ENTRY_3))))(dec, eStr) )
#endif


#define LibCodGenericCreate(lib) ((struct Codec *)(RunLoadedFunction(lib, ENTRY_1, 0)))


#endif /* LIB_AUDIO_DEC_H */
