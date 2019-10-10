/* For free support for VSIDE, please visit www.vsdsp-forum.com */

/*
  Audio decoder AUDIODEC.DL3 source code v1.05 2017-12-05
*/
#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <string.h>
#include <codecmpg.h>
#include <libdecvorb.h>
#include <libdecwav.h>
#include <sysmemory.h>
#include <libaudiodec.h>
#include <aucommon.h>
#include <kernel.h>
#include "codecservices.h"
#include "ungetbuffer.h"


void (*Delete)(struct Codec *cod) = NULL;
enum AuDecFormat lastAuDecFormat = auDecFGuess;
u_int16 *lastDecoderLib = NULL;

#define MAKE_ID(a,b,c,d) (((u_int32)(a)<<24)|((u_int32)(b)<<16)|((c)<<8)|(d))
#define MAKE_ID_MIX(a,b,c,d) (((u_int32)(c)<<24)|((u_int32)(d)<<16)|((a)<<8)|(b))

int MyDeleteAudioDecoder(register AUDIO_DECODER *dec);

struct AudioFormat {
  u_int32 mask, value;
  enum AuDecFormat format;
  char *libName;
} __mem_y audioFormat[] = {
  {0xFF00FFFFUL, MAKE_ID_MIX('I','D','3', 0 ), auDecFId3v2, NULL},
  {0xFFFFFFFFUL, MAKE_ID_MIX('R','I','F','F'), auDecFWav,   "decwav"},
  {0xFFFFFFFFUL, MAKE_ID_MIX('O','g','g','S'), auDecFOgg,   "decvorb"},
  {0xFFFFFFFFUL, MAKE_ID_MIX('f','L','a','C'), auDecFFlac,  "decflac"},
  {0xFFFFFFFFUL, MAKE_ID_MIX('c','a','f','f'), auDecFAlac,  "decalac"},
  {0xFFFFFFFFUL, MAKE_ID_MIX('0','&',178,'u'), auDecFWma,   "decwma"},
  {0xFFFFFFFFUL, MAKE_ID_MIX('A','D','I','F'), auDecFAac,   "decaac"},
  {0xFFFFFFFFUL, MAKE_ID_MIX('f','t','y','p'), auDecFAacOrAlac, "decaac"},
  {0xFFFFFFFFUL, MAKE_ID_MIX('m','o','o','v'), auDecFAacOrAlac, "decaac"},
  {0xFFFFFFFFUL, MAKE_ID_MIX('M','T','h','d'), auDecFMidi,  "decmidi"},
  {0xFFFFFFFFUL, MAKE_ID_MIX('D','S','D',' '), auDecFDsd,   "decdsd"},
  {0xFFFFFFFFUL, MAKE_ID_MIX('F','R','M','8'), auDecFDsd,   "decdsd"},
  {0xFFFFFFFFUL, MAKE_ID_MIX('F','O','R','M'), auDecFAiff,  "decaiff"},
  {0x0000FFE2UL, 0x0000FFE2UL,                 auDecFMp3,   "decmp3"},
  {0x0000FFF6UL, 0x0000FFF0UL,                 auDecFAac,   "decaac"},
  {0,0,0},
};

DLLENTRY(MyCreateAudioDecoder)
AUDIO_DECODER *MyCreateAudioDecoder(register FILE *inFp,
				    register FILE *outFp,
				    register void (*callback)
				    (AUDIO_DECODER *dec, u_int16 samples),
				    register enum AuDecFormat auDecFormat) {
  AUDIO_DECODER *dec = calloc(1, sizeof(AUDIO_DECODER));
  const char *decoderLibName = NULL;
  if (!dec) {
    return NULL;
  }

  {
    u_int32 fourBytes[2];
  restart:
    fread(ungetBuffer, sizeof(ungetBuffer[0])*4, 1, inFp);
    memcpy(fourBytes, ungetBuffer, sizeof(ungetBuffer));
    ungetBufferFilePos = inFp->pos;

    while (auDecFormat == auDecFGuess) {
      struct AudioFormat __mem_y *af = audioFormat;
      u_int32 mask;
      while ((mask = af->mask)) {
	if ((fourBytes[0] & mask) == af->value) {
	  auDecFormat = af->format;
	  decoderLibName = af->libName;
	  break;
	}
	af++;
      }
      if (auDecFormat == auDecFId3v2) {
	/* ID3v2 tag; let's skip it. */
	u_int32 id3Size;
	u_int16 t16;
	auDecFormat = auDecFGuess;
	id3Size  = (u_int32)(ungetBuffer[3]&0xFF00U) << (21-8);
	id3Size += (u_int32)(ungetBuffer[3]&0x00FFU) << 14;
	t16  = fgetc(inFp) << 7;
	t16 += fgetc(inFp);
	id3Size += t16;
	inFp->pos += id3Size;
	goto restart;
      } else if (auDecFormat == auDecFAacOrAlac) { /* MP4 container. */
	auDecFormat = RunLibraryFunction("mp4file", ENTRY_1, (int)inFp);
	if (auDecFormat == auDecFAlac) {
	  decoderLibName = "decalac";
	}
      }
      if (auDecFormat == auDecFGuess) {
	fourBytes[0] = fourBytes[1];
	fourBytes[1] = 0x67fba112;
	if (fourBytes[0] == 0x67fba112) {
	  /* Nothing matches? Try decoding as MP3. */
	  auDecFormat = auDecFMp3;
	  decoderLibName = "decmp3";
	}
      }
    }
  }

  dec->format = auDecFormat;

  if (decoderLibName) {
    if (auDecFormat != lastAuDecFormat) {
      if (lastDecoderLib) {
	DropLibrary(lastDecoderLib);
      }
      lastDecoderLib = LoadLibrary(decoderLibName);
      if (!lastDecoderLib) {
	lastAuDecFormat = auDecFGuess;
	MyDeleteAudioDecoder(dec);
	return NULL;
      }
      lastAuDecFormat = auDecFormat;
    }
    if (!(dec->cod = LibCodGenericCreate(lastDecoderLib))) {
      MyDeleteAudioDecoder(dec);
      return NULL;
    }
  }

  dec->decoderLib = lastDecoderLib;

  {
    struct CodecServices *cs = &dec->cs;

    memcpyYX(cs, &audioDecoderDefaultCodecServices,
	     sizeof(audioDecoderDefaultCodecServices));

    dec->cod->cs = cs;
    dec->inFp = inFp;
    dec->outFp = outFp;
    dec->flags = AUDECF_STOPPED;
    dec->callback = callback;
    cs->fileSize = cs->fileLeft = (__F_SEEKABLE(inFp)) ?
      inFp->fileSize : 0xFFFFFFFFU;
  }

  ungetBufferPointer = 0;

  {
    s_int16 t = ioctl(dec->outFp, IOCTL_AUDIO_GET_OUTPUT_BUFFER_SIZE, NULL);
    if (t > 0) {
      currentAudioBufferSize = originalAudioBufferSize = t;
    }
  }
  modelCallbacks[MODEL_CALLBACK_AUDEC] = dec;
  return dec;
}

DLLENTRY(MyDeleteAudioDecoder)
int MyDeleteAudioDecoder(register AUDIO_DECODER *dec) {
  if (dec) {
    struct Codec *cod = dec->cod;
    void (*del)(struct Codec *) = cod->Delete;
    if (cod && del) {
      del(cod);
    }
    free(dec);
  }

  if (currentAudioBufferSize != originalAudioBufferSize) {
    ioctl(dec->outFp, IOCTL_AUDIO_SET_OUTPUT_BUFFER_SIZE,
	  (void *)originalAudioBufferSize);
  }
  originalAudioBufferSize = currentAudioBufferSize = 0;

  return 0;
}


DLLENTRY(MyDecodeAudio)
u_int16 MyDecodeAudio(register AUDIO_DECODER *dec,
		      register const char **eStr) {
  int errCode;
  //	printf("#soon decode\n");
  dec->flags = (dec->flags & ~AUDECF_STOPPED) | AUDECF_PLAYING;
  errCode = dec->cod->Decode(dec->cod, &dec->cs, eStr);
  //	printf("#decode returned %d, \"%s\"\n", errCode, *eStr);
  dec->cs.audioBufferRequest = 0;
  dec->cs.Output(&dec->cs, NULL, 0);
  dec->flags = (dec->flags & ~AUDECF_PLAYING) | AUDECF_STOPPED;
  return errCode;
}


/*
  By calling the main function the user gets its parameter list replaced by
  potential file suffixes. The parameter list must be a null-terminated
  string.

  Example:
  #include <apploader.h>
  #define STR_LEN 80
  static char s[STR_LEN];
  memset(s,' ',STR_LEN-1); // To let audiodec know how much there is space
  RunProgram("audiodec", (u_int16)s);
  printf("Suffix list: \"%s\"\n", s);

*/
ioresult main(char *parameters) {
  if (parameters) {
    strncpy(parameters, "MP3,OGG,WAV,FLA,MP2,M4A,MP4,AIF,CAF,DSF,AAC,WMA,ASF",
	    strlen(parameters));
  }
  return EXIT_SUCCESS;
}


void fini(void) {
  modelCallbacks[MODEL_CALLBACK_AUDEC] = NULL;
  if (lastDecoderLib) {
    DropLibrary(lastDecoderLib);
  }
}
