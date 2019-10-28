/**
   \file codecwav.c Wav Codec.
*/

#include <filter.h>
#include "swap.h"
#include "codWavGeneral.h"
#include "codWavLinear.h"
#include "codecwav.h"



/**
   Doesn't yet allow re-entrancy.

   \todo Allow re-entrant instead of static allocation.
*/
struct CodecWav codecWav = {
  {0}
};

/* Documented in codecwav.h */
struct Codec *CodWavCreate(void) {
  struct CodecWav *cw = &codecWav;

  if (cw->used)
    return NULL;

  cw->used++;
  cw->c.version = CODEC_VERSION;
  cw->c.Create = CodWavCreate;
  cw->c.Delete = CodWavDelete;
  cw->c.Decode = CodWavDecode;

  return (struct Codec *)cw;
}


/* Documented in codecwav.h */
void CodWavDelete(struct Codec *codec) {
  struct CodecWav *cod = (struct CodecWav *)codec;
  if (cod) {
    cod->used--;
  }
}





/* Documented in codecwav.h */
enum CodecError CodWavDecode(struct Codec *codec, struct CodecServices *cs,
			     const char **errorString) {
  struct CodecWav * __mem_y cw = (struct CodecWav *)codec;
  enum CodecError err = ceFormatNotFound;
  u_int16 read32Err = 0;
  cw->realPlayTimeSamples = cw->realPlayTimeSeconds = 0;
  *errorString = "Not Riff WAV";
  if (CodWavRead32(cs, &read32Err) != MAKE_ID('R','I','F','F')) {
    return ceFormatNotFound;
  }
  cw->riffLeft = CodWavRead32I(cs, &read32Err);
#if 0
  printf("RIFF left %04" FMT_H32 ", file left %04" FMT_H32 "\n",
	 cw->riffLeft, cs->fileLeft);
#endif
  if (cs->fileLeft < cw->riffLeft)
    cw->riffLeft = cs->fileLeft;

  cw->coding = wcUnknown;
  if (cw->riffLeft < 4 || CodWavRead32(cs, &read32Err) != MAKE_ID('W','A','V','E'))
    return ceFormatNotFound;

  cw->riffLeft -= 4;

  *errorString = "Not known RIFF format";

  while (cw->riffLeft >= 8 && !cs->cancel && !read32Err) {
    u_int32 nam = CodWavRead32(cs, &read32Err);
    u_int32 __mem_y n = CodWavRead32I(cs, &read32Err);
    u_int16 inBuf = (n > 0x28) ? 0x28 : (n & ~1);
    cw->riffLeft -= 8+n;
#if 0
    printf("Section %08" FMT_H32 ", size %" FMT_S32 ", left = %" FMT_S32 "\n",
	   nam, n, cw->riffLeft);
#endif
    if (inBuf && nam != MAKE_ID('d','a','t','a')) {
#if 0
      printf("ReadToBuf(%" FMT_U16 ")\n", inBuf);
      printf("imaBlockAlign %" FMT_U16 ", imaSamplesPerBlock %" FMT_U16 "\n",
	     cw->imaBlockLen, cw->imaSamplesPerBlock);
#endif

      if (cs->Read(cs, cw->buf, 0, inBuf) != inBuf) {
	  *errorString = "Unexpected end of file";
	  return ceUnexpectedFileEnd;
      }
      n -= inBuf;
    } else {
      inBuf = 0;
    }

    switch(nam) {
    case MAKE_ID('f','m','t',' '):
      /* Is there enough minimum data? */
      if (inBuf >= sizeof(struct CodecWavRiffFormatTag)*(2/sizeof(u_int16))) {
	register struct CodecWavRiffFormatTag *fmt =
	  (struct CodecWavRiffFormatTag *)cw->buf;
#if 0
	printf("fmt size ok\n");
#endif
	cs->channels = Swap16(fmt->channels);
	cw->fileSampleRate = Swap32Mix(fmt->samplesPerSecond);
	cw->bitsPerSample = Swap16(fmt->bitsPerSample);
	cs->avgBitRate = cs->currBitRate = cs->peakBitRate =
	  (u_int32)cw->fileSampleRate*cs->channels*cw->bitsPerSample;
#if 0
	printf("ch=%" FMT_U16 ", fs=%" FMT_U32 ", bpSample=%" FMT_U16 "\n",
	       cs->channels, cw->fileSampleRate, cw->bitsPerSample);
#endif
	if (inBuf >= 26 && Swap16(fmt->formatTag) == WAVE_FORMAT_PCMEX) {
	  fmt->formatTag = cw->buf[12];
	}
	switch(Swap16(fmt->formatTag)) {
	case WAVE_FORMAT_PCM:
	  cw->coding = wcLinear;
	  break;
	case WAVE_FORMAT_IEEE_FLOAT:
	  if (cw->bitsPerSample == 32) {
	    cw->coding = wcFloat;
	  }
	  break;
	default:
	  *errorString = "RIFF subformat not supported";
	  return ceFormatNotSupported;
	  break;
	}

      }
      break;
    case MAKE_ID('d','a','t','a'):
      cw->dataStartPos = cs->Tell(cs);
#if 0
      printf("cw->dataStartPos %" FMT_U32 "\n", cw->dataStartPos);
#endif
      switch(cw->coding) {
      case wcLinear:
      case wcFloat:
	err = CodWavDecodeLinear(cw, cs, n, errorString);
	n = 0;
	break;
      default:
//	puts("No coding for data");
	break;
      }
      if (err > 0) {
	goto finally;
      }
      break;
    case MAKE_ID('f','a','c','t'):
      if (inBuf >= 4) {
	cw->samples = Swap16(cw->buf[0])|
	  (s_int32)Swap16(cw->buf[1])<<16;
	break;
      }
    }
    if (n)
      cs->Skip(cs, n);
  }
 finally:
  /* Clear cancel flag in case it was set during playback. */
  cs->cancel = 0;

  if (err == ceOk)
    *errorString = "Ok";

  return err;
}
