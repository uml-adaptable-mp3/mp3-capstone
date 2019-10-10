#include <vo_stdio.h>
#include <vstypes.h>
#include <vs1005h.h>
#include <audio.h>
#include <aucommon.h>
#include <string.h>
#include <stdlib.h>
#include <codec.h>
#include <sysmemory.h>
#include <timers.h>
#include <libaudiodec.h>
#include <ringbuf.h>
#include <math.h>

#define MIN(a,b) (((a)<(b))?(a):(b))

u_int32 lastReport = 0;

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
  u_int16 bytesRead;
  AUDIO_DECODER *auDec = (AUDIO_DECODER *)cs;
  FILE *inFp = auDec->inFp;

  bytes = (u_int16)(MIN((u_int32)bytes, cs->fileLeft));

  /* The file system function Read() offers direct access to byte
     offsets. Thus we are using this as an efficient replacement for
     the blunter fread()/fgetc() tool where we'd need to align bytes
     by hand. */
  bytesRead = inFp->op->Read(inFp, data, firstOdd, bytes);

  cs->fileLeft -= bytesRead;

  return bytesRead;
}


/**
   struct CodecServices service DefaultCSSeek().

   \param cs Codec Service structure.
   \return 0 if successful.
*/
s_int16 DefaultCSSeek(struct CodecServices *cs, s_int32 offset,
		      s_int16 whence) {
  AUDIO_DECODER *auDec = (AUDIO_DECODER *)cs;
  FILE *inFp = auDec->inFp;
  s_int16 ret;

  ret = fseek(inFp, offset, whence);

  cs->fileLeft = cs->fileSize - inFp->pos;

  return ret;
}


/**
   struct CodecServices service DefaultCSSkip(). Skips given number of bytes.

   \param cs Codec Service structure.
   \param bytes How many bytes to skip.
   \return Number of bytes skipped. May be lower than \a bytes if an error
	or EOF was encountered.
*/
u_int32 DefaultCSSkip(struct CodecServices *cs, u_int32 bytes) {
  u_int32 toSkip = MIN(bytes, cs->fileLeft);
  DefaultCSSeek(cs, toSkip, SEEK_CUR);
  return toSkip;
}



/**
   struct CodecServices service DefaultCSTell().

   \param cs Codec Service structure.
   \return Byte position in file.
*/
s_int32 DefaultCSTell(struct CodecServices *cs) {
  AUDIO_DECODER *auDec = (AUDIO_DECODER *)cs;
  return auDec->inFp->pos;
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
  s_int16 wordsPerSample = (n < 0) ? 2 : 1;
  s_int16 channels = cs->channels;
  u_int32 currTimeCount;
  FILE *outFp = auDec->outFp;

  n = abs(n);

  /* In case the file system isn't an audio file system, and doesn't support
     this Ioctl(), errors will be silently ignored. */
  if (cs->sampleRate != auDec->sampleRate) {
    auDec->sampleRate = cs->sampleRate;
    ioctl(outFp, IOCTL_AUDIO_SET_ORATE, (void *)(&auDec->sampleRate));
  }

  if (channels != auDec->channels) {
    auDec->channels = channels;
  }

  if (n && wordsPerSample != auDec->wordsPerSample) {
    auDec->wordsPerSample = wordsPerSample; 
    ioctl(outFp, IOCTL_AUDIO_SET_BITS, (void *)(16*wordsPerSample));
  }

  if (data) {
    u_int16 words = n*channels*wordsPerSample;

    /* Be multitasking-friendly:
       if paused, send zeroes to output and call callback function. */
    do {
      if (!cs->cancel) {
	if (channels == 1) {
	  /* Slow method for converting mono to stereo, but still ok for
	     audio drivers that cannot natively handle mono. */
	  u_int16 *dp = (u_int16 *)data;
	  u_int16 i;
	  for (i=0; i<n; i++) {
	    u_int32 tBuf[2];
	    if (wordsPerSample == 1) {
	      tBuf[0] = 0x10001L * *dp++; /* Make copy to both LSW and MSW */
	    } else {
	      tBuf[0] = tBuf[1] = *((u_int32 *)dp);
	      dp += 2;
	    }
	    fwrite(tBuf, sizeof(*tBuf), wordsPerSample, outFp);
	  }
	} else {
	  fwrite(data, sizeof(*data), words, outFp);
	}
      }
      if (auDec->callback) {
	auDec->callback(auDec, n);
      }
      memset(data, 0, words);
    } while (auDec->pause && !cs->cancel);
  } /* if (data) */

  /* Give 1 ms CPU to other tasks 10 times every second to make sure
     multitasking doesn't completely stop even if the decoder is
     CPU hungry. */
  currTimeCount = ReadTimeCount();
  if (currTimeCount - lastReport > TICKS_PER_SEC/10) {
    lastReport = currTimeCount;
    Delay(1);
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
  0,	/* audioBufferRequest */
  0,	/* unused1 */
  0,	/* unused2 */
  0,	/* unused3 */
  0,	/* unused4 */
  0,	/* avgBitRate */
  0,	/* currBitRate */
  0,	/* peakBitRate */
  0,	/* gain */
  1,	/* fastForward */
};
