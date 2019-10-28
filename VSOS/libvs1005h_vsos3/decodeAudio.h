/**
   \file decodeAudio.h General audio decoding library.
   The audio decoding library takes care of creating an audio decoder
   for an audio file. Currently it is able only to decode MP3 files,
   but this will be expanded in the future to include all VS1005 audio
   formats.
*/
#ifndef DECODE_AUDIO_H
#define DECODE_AUDIO_H

#include <vo_stdio.h>
#include <codec.h>

/** Hint for decoding format. If unknown, use auDecFGuess. */
enum AuDecFormat {
  /** Use this value if you want the decoder to determine your file format,
      and if the file is seekable. */
  auDecFGuess,
  /** Audio is in MP3 format. */
  auDecFMp3,
  auDecFWav,
  auDecFOgg
};

struct AudioDecoder;
typedef struct AudioDecoder AUDIO_DECODER;

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
};

/** Create an audio decoder. */
auto AUDIO_DECODER *CreateAudioDecoder(register FILE *inFp,
				       register FILE *outFp,
				       register void (*callback)
				       (AUDIO_DECODER *dec, u_int16 samples),
				       register enum AuDecFormat auDecFormat);

/** Delete an audio decoder. */
auto int DeleteAudioDecoder(register AUDIO_DECODER *dec);

/** Decode audio using the audio decoder earlier created with
    CreateAudioDecoder(). */
auto u_int16 DecodeAudio(register AUDIO_DECODER *dec,
			 register const char **eStr);



#endif /* DECODE_AUDIO_H */
