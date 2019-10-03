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
#include "ungetbuffer.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

u_int16 ungetBuffer[4] = {0};
u_int16 ungetBufferPointer = 8;
u_int32 ungetBufferFilePos = 0;
s_int16 originalAudioBufferSize = 0, currentAudioBufferSize = 0;
u_int32 lastReport = 0;
u_int32 lastUnderflows = 0;


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
  u_int16 bytesRead = 0;
  AUDIO_DECODER *auDec = (AUDIO_DECODER *)cs;
  FILE *inFp = auDec->inFp;

  if (inFp->pos != ungetBufferFilePos) {
    ungetBufferPointer = 8;
  }

  if (ungetBufferPointer < 8) {
    bytesRead = MIN(8-ungetBufferPointer, bytes);
    bytes -= bytesRead;
    MemCopyPackedBigEndian(data, firstOdd, ungetBuffer,
			   ungetBufferPointer, bytesRead);
    firstOdd += bytesRead;
    ungetBufferPointer += bytesRead;
  }

  if (bytes+bytesRead > cs->fileLeft) {
    bytes = (u_int16)cs->fileLeft-bytesRead;
  }

  /* The file system function Read() offers direct access to byte
     offsets. Thus we are using this as an efficient replacement for
     the blunter fread()/fgetc() tool where we'd need to align bytes
     by hand. */
  bytesRead += inFp->op->Read(inFp, data, firstOdd, bytes);

  if (cs->fileLeft != 0xFFFFFFFFU) {
    cs->fileLeft -= (s_int32)bytesRead;
  }

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

  if (whence == SEEK_CUR && inFp->pos == ungetBufferFilePos) {
    offset -= 8-ungetBufferPointer;
  }
  ungetBufferPointer = 8;
  ret = fseek(inFp, offset, whence);
  if (cs->fileLeft != 0xFFFFFFFFU) {
    cs->fileLeft = cs->fileSize - inFp->pos;
  }
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


/* Fade in/out works is designed for stereo data, but still works
   acceptably with other combinations. */
void CsIntFade(register FILE *fp, register const s_int16 *d,
	       register u_int16 wordsPerSample,
	       register u_int16 startMult,
	       register s_int16 multipDelta, register u_int16 rounds) {
  u_int16 localD[4] = {0};
  u_int16 *localDP = (localD-1)+wordsPerSample;
  s_int16 left = d[wordsPerSample-1];
  s_int16 right = d[2*wordsPerSample-1];

  do {
    localDP[             0] = (s_int16)(((s_int32)left  * startMult) >> 16);
    localDP[wordsPerSample] = (s_int16)(((s_int32)right * startMult) >> 16);
    fwrite(localD, sizeof(localD[0]), 2*wordsPerSample, fp);
    startMult += multipDelta;
  } while (--rounds);
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
  s_int16 mustTruncateTo16Bits = 0; /* Audio cannot handle 32 bits */
  s_int16 channels = cs->channels;
  s_int16 outBufSize = 0, outBufRequest = 0;
  static s_int16 lastPause = 0, mustConvertMonoToStereo = 0;
  static s_int16 problemsWithAllocatingMemory = 0;
  u_int32 currTimeCount;
  FILE *outFp = auDec->outFp;

  n = abs(n);

  outBufRequest = cs->audioBufferRequest ? cs->audioBufferRequest : 4096;

  /* If there is a buffer request, try to meet it. */
  /* In case the file system isn't an audio file system, and doesn't support
     this Ioctl(), errors will be silently ignored. */
  if (!problemsWithAllocatingMemory && outBufRequest > currentAudioBufferSize) {
    while (outBufRequest && ioctl(outFp, IOCTL_AUDIO_SET_OUTPUT_BUFFER_SIZE,
				  (void *)outBufRequest)) {
      outBufRequest >>= 1;
      problemsWithAllocatingMemory = 1;
    }
    currentAudioBufferSize = outBufRequest;
  }


  /* In case the file system isn't an audio file system, and doesn't support
     this Ioctl(), errors will be silently ignored. */
  if (cs->sampleRate != auDec->sampleRate) {
    auDec->sampleRate = cs->sampleRate;
    ioctl(outFp, IOCTL_AUDIO_SET_ORATE, (void *)(&auDec->sampleRate));
  }

  /* In case the file system isn't an audio file system, and doesn't support
     this Ioctl(), errors will be silently ignored. */
  if (channels != auDec->channels) {
    auDec->channels = channels;
    mustConvertMonoToStereo = 0;
    if (ioctl(outFp, IOCTL_AUDIO_SET_OCHANNELS, (void *)channels)) {
      if (channels == 1) {
	mustConvertMonoToStereo = 1;
      }
    }
  }

  /* In case the file system isn't an audio file system, and doesn't support
     this Ioctl(), errors will be silently ignored. */
  if (n && wordsPerSample != auDec->wordsPerSample) {
    if (ioctl(outFp, IOCTL_AUDIO_SET_BITS, (void *)(16*wordsPerSample))
	== S_OK || wordsPerSample == 1) {
      auDec->wordsPerSample = wordsPerSample;
    } else {
      wordsPerSample = 1;
      mustTruncateTo16Bits = 1;
    }
  }

  if (data) {
    u_int16 words = n*channels*wordsPerSample;
    if (mustTruncateTo16Bits) {
      /* Truncate from 32-bit mixed-endian to 16-bit format */
      ringcpy((u_int16 *)data, 1, (u_int16 *)data+1, 2, words);
    }

    /* Be multitasking-friendly:
       if paused, send zeroes to output and call callback function. */
    if (lastPause) {
      /* Fade in */
      CsIntFade(outFp, data, wordsPerSample,     0,  100, 655);
    }
    lastPause = auDec->pause;
    if (lastPause) {
      /* Fade out */
      CsIntFade(outFp, data, wordsPerSample, 65500, -100, 655);
      memset(data, 0, words);
    }
    do {
      if (mustConvertMonoToStereo) {
	/* Slow method for converting mono to stereo, but still ok for
	   audio drivers that cannot natively handle mono. */
	u_int16 *dp = (u_int16 *)data;
	u_int16 i;
	for (i=0; i<n; i++) {
	  u_int32 tBuf[2];
	  if (wordsPerSample == 1) {
	    tBuf[0] = 0x10001L * (u_int32)(*dp++); /* Make copy to both LSW and MSW */
	  } else {
	    tBuf[0] = tBuf[1] = *((u_int32 *)dp);
	    dp += 2;
	  }
	  fwrite(tBuf, sizeof(*tBuf), wordsPerSample, outFp);
	}
      } else {
	fwrite(data, sizeof(*data), words, outFp);
      }
      if (auDec->callback) {
	auDec->callback(auDec, n);
      }
    } while (lastPause && auDec->pause && !cs->cancel);
  } /* if (data) */

  /* If out of CPU (audio underflows grow), then give some CPU to other
     tasks 10 times a second. This will allow multitasking to continue
     working even when all CPU would be required for decoding audio. */
  currTimeCount = ReadTimeCount();
  if (currTimeCount - lastReport > TICKS_PER_SEC/10) {
    u_int32 __mem_y currUnderflows = 0;
    lastReport = currTimeCount;
    ioctl(outFp, IOCTL_AUDIO_GET_UNDERFLOWS, (void *)(&currUnderflows));
    if (currUnderflows != lastUnderflows) {
      lastUnderflows = currUnderflows;
      Delay(1);
    }
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
  static s_int16 firstTime = 1;

  if (firstTime) {
    firstTime = 0;
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
