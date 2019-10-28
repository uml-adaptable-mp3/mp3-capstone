#include <vo_stdio.h>
#include <vstypes.h>
#include <audiofs.h>
#include <string.h>
#include <stdlib.h>
#include <codec.h>
#include <sysmemory.h>
#include <codecmpg.h>
#include <timers.h>
#include "decodeAudio.h"


#define MIN(a,b) (((a)<(b))?(a):(b))


/**
   struct CodecServices service DefaultCSRead(). Reads bytes into an u_int16
   table. Handles properly odd bytes.

   \param cs Codec Service structure.
   \param data Pointer to the destination data.
   \param firstOdd is non-zero if the first byte an odd byte. In this
	case data is written only to the LSB of the first destination word
	pointed to by ptr;
   \param bytes How many bytes to read. If the last destination
	word is only half filled, only the MSB is changed.
   \return Number of bytes read. 0 is an EOF or error.
*/
u_int16 DefaultCSRead(struct CodecServices *cs, u_int16 *data,
	       u_int16 firstOdd, u_int16 bytes) {
  u_int16 readBytes;
  AUDIO_DECODER *auDec = (AUDIO_DECODER *)cs;

  if (bytes > cs->fileLeft)
    bytes = cs->fileLeft;


  /* The file system function Read() offers direct access to byte
     offsets. Thus we are using this as an efficient replacement for
     the blunter fread()/fgetc() tool where we'd need to align bytes
     by hand. */
  readBytes = auDec->inFp->op->Read(auDec->inFp, data, firstOdd, bytes);

  if (cs->fileLeft != 0xFFFFFFFFU)
    cs->fileLeft -= readBytes;

  return readBytes;
}


/**
   struct CodecServices service DefaultCSSkip(). Skips given number of bytes.

   \param cs Codec Service structure.
   \param bytes How many bytes to skip.
   \return Number of bytes skipped. May be lower than \a bytes if an error
	or EOF was encountered.
*/
u_int32 DefaultCSSkip(struct CodecServices *cs, u_int32 bytes) {
  AUDIO_DECODER *auDec = (AUDIO_DECODER *)cs;
  u_int32 toSkip = MIN(bytes, cs->fileLeft);
  fseek(auDec->inFp, toSkip, SEEK_CUR);
  cs->fileLeft -= toSkip;
  return toSkip;
}


/**
   struct CodecServices service DefaultCSSeek().

   \param cs Codec Service structure.
   \return 0 if successful.
*/
s_int16 DefaultCSSeek(struct CodecServices *cs, s_int32 offset,
		      s_int16 whence) {
  AUDIO_DECODER *auDec = (AUDIO_DECODER *)cs;
  s_int16 ret = fseek(auDec->inFp, offset, whence);
  if (cs->fileLeft != 0xFFFFFFFFU)
    cs->fileLeft = cs->fileSize - ftell(auDec->inFp);
  return ret;
}

/**
   struct CodecServices service DefaultCSTell().

   \param cs Codec Service structure.
   \return Byte position in file.
*/
s_int32 DefaultCSTell(struct CodecServices *cs) {
  AUDIO_DECODER *auDec = (AUDIO_DECODER *)cs;
  return ftell(auDec->inFp);
}


/**
   struct CodecServices service DefaultCSOutput().

   Writes data to a 16-bit WAV file.

   \param cs Codec Service structure.
   \param data Interleaved audio data. This data buffer may
	be used as working space by Output().
   \param n Number of \a cs->channels channel samples. The total number
	of individual samples is \a n * \a cs->channels.
*/
s_int16 DefaultCSOutput(struct CodecServices *cs, s_int16 *data, s_int16 n) {
  AUDIO_DECODER *auDec = (AUDIO_DECODER *)cs;

  /* If the file system isn't an audio file system, and doesn't support
     this Ioctl(), such errors will be silently ignored. */
  if (cs->sampleRate != auDec->sampleRate) {
    auDec->sampleRate = cs->sampleRate;
    ioctl(auDec->outFp, IOCTL_AUDIO_SET_ORATE, &cs->sampleRate);
  }

  /* If the file system isn't an audio file system, and doesn't support
     this Ioctl(), such errors will be silently ignored. */
  if (cs->channels != auDec->channels) {
    auDec->channels = cs->channels;
    ioctl(auDec->outFp, IOCTL_AUDIO_SET_OCHANNELS, (char *)cs->channels);
  }

  if (data) {
    /* Be multitasking-friendly */
    do {
      u_int16 words = n*cs->channels;
      fwrite(data, sizeof(*data), words, auDec->outFp);
      memset(data, 0, words);
      if (auDec->callback)
	auDec->callback(auDec, n);
    } while (auDec->pause);
  }

  return 0;
}


#if 0
/**
   struct CodecServices service DefaultCSComment().
   Writes comments from Ogg and Flac files to the screen.

   \param cs Codec Service structure.
   \param c data, one character at the time.
*/
void DefaultCSComment(struct CodecServices *cs, u_int16 c) {
  static s_int16 virgin = 1;

  if (virgin) {
    virgin = 0;
    printf("-- Start of comments\n");
  }
  if (c == FS_CODSER_COMMENT_END_OF_LINE) {
    putchar('\n');
  } else if (c == FS_CODSER_COMMENT_END_OF_COMMENTS) {
    printf("-- End of comments: ");
    printf("gain = %d dB/2 = %c%d.%d dB\n",
	   cs->gain, (cs->gain < 0) ? '-' : '+',
	   abs(cs->gain)>>1,
	   (cs->gain&1) ? 5 : 0);
  } else {
    putchar(c);
  }
}
#endif



const struct CodecServices __mem_y audioDecoderDefaultCodecServices = {
  FS_CODEC_SERVICES_VERSION,
  DefaultCSRead,
  DefaultCSSkip,
  DefaultCSSeek,
  DefaultCSTell,
  DefaultCSOutput,
  NULL, /* Comment */
  NULL, /* Spectrum */
  0,    /* fileSize */
  0,	/* fileLeft */
  0xFFFFU, /* goTo */
  0,	/* cancel */
  0,	/* playTimeSeconds */
  0,	/* playTimeSamples */
  0,	/* playTimeTotal */
  0,	/* sampleRate */
  0,	/* channels */
  {0},	/* matrix */
  0,	/* avgBitRate */
  0,	/* currBitRate */
  0,	/* peakBitRate */
  0,	/* gain */
  1,	/* fastForward */
};

auto AUDIO_DECODER *CreateAudioDecoder(register FILE *inFp,
				       register FILE *outFp,
				       register void (*callback)
				       (AUDIO_DECODER *dec, u_int16 samples),
				       register enum AuDecFormat auDecFormat) {
  AUDIO_DECODER *dec = calloc(1, sizeof(AUDIO_DECODER));
  if (!dec)
    return NULL;
  dec->cod = CodMpgCreate();
  if (!dec->cod) {
    free(dec);
    return NULL;
  }
  CodMpgSetTags(dec->cod,
		codMpgDisableCrc, codMpgEnableLayer2,
		codMpgLeaveOnError, 0);

  memcpyYX(&dec->cs, &audioDecoderDefaultCodecServices,
	   sizeof(audioDecoderDefaultCodecServices));

  dec->inFp = inFp;
  dec->outFp = outFp;
  if (!fseek(dec->inFp, 0, SEEK_END)) {
    dec->cs.fileSize = dec->cs.fileLeft = ftell(dec->inFp);
    fseek(dec->inFp, 0, SEEK_SET);
  } else {
    dec->cs.fileSize = dec->cs.fileLeft = 0xFFFFFFFFU;
  }
  dec->callback = callback;

  return dec;
}

auto int DeleteAudioDecoder(register AUDIO_DECODER *dec) {
  dec->cod->Delete(dec->cod);
  free(dec);
  return 0;
}


auto u_int16 DecodeAudio(register AUDIO_DECODER *dec,
			 register const char **eStr) {
  int errCode;
  errCode = dec->cod->Decode(dec->cod, &dec->cs, eStr);
  return errCode;
}
