/* For free support for VSIDE, please visit www.vsdsp-forum.com */

// This program determines the length of an audio file.

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <string.h>
#include <stdlib.h>
#include <vo_gpio.h>
#include <timers.h>
#include <swap.h>
#include <audio.h>
#include "audioinfo.h"

#define MAKE_ID(a,b,c,d) (((u_int32)(a)<<24)|((u_int32)(b)<<16)|((c)<<8)|(d))
#define MAKE_DI(d,c,b,a) (((u_int32)(a)<<24)|((u_int32)(b)<<16)|((c)<<8)|(d))

auto u_int16 Read16LE(register FILE *fp) {
  u_int16 res = fgetc(fp);
  return res | (fgetc(fp) << 8);
}

auto u_int32 Read32LE(register FILE *fp) {
  u_int16 lo = Read16LE(fp);
  return ((u_int32)Read16LE(fp) << 16) | lo;
}

auto u_int16 Read16BE(register FILE *fp) {
  return Swap16(Read16LE(fp));
}

auto u_int32 Read32BE(register FILE *fp) {
  return Swap32(Read32LE(fp));
}

auto u_int32 Read64BE(register FILE *fp) {
  Read32LE(fp);
  return Swap32(Read32LE(fp));
}

auto u_int32 Read64LE(register FILE *fp) {
  u_int32 res = Read32LE(fp);
  Read32LE(fp);
  return res;
}

auto double SamplesToTime(register u_int32 samplesHi,
			  register u_int32 samplesLo,
			  register u_int32 sampleRate) {
  return ((samplesHi*4294967296.0) + samplesLo) / sampleRate;
}

int SkipID3(register FILE *fp) {
  u_int32 pos = ftell(fp);
  if ((Read32BE(fp)&0xFFFFFF00U) == MAKE_ID('I','D','3',0)) {
    s_int16 shift = 21;
    Read16BE(fp); /* Skip version lower byte and flags byte */
    /* Can't use Read32LE() because every byte contains only 7 bits of data. */
    pos += 10;
    do {
      pos += ((u_int32)fgetc(fp)) << shift;
    } while ((shift-=7) >= 0);
  }
  fseek(fp, pos, SEEK_SET);
  return 0;
}



/*
  MP3-specific code.
  To know the length of an MP3 file we need to decode all file headers
  which are, depending on the bitrate, typically located at 200-1000 bytes
  from each other. So, in practice we need to read the whole file.
*/
const s_int16 __mem_y mp3BitRates[2][2][16] = {
  {
    {-1,  8, 16, 24, 32, 40, 48, 56,  64,  80,  96, 112, 128, 144, 160, -1},
    {-1, 32, 48, 56, 64, 80, 96,112, 128, 160, 192, 224, 256, 320, 384, -1},
  }, {
    {-1,  8, 16, 24, 32, 40, 48, 56,  64,  80,  96, 112, 128, 144, 160, -1},
    {-1, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, -1},
  }
};

const s_int32 __mem_y mp3SampleRates[4][4] = {
  {11025, 12000,  8000, -1},
  {11025, 12000,  8000, -1},
  {22050, 24000, 16000, -1},
  {44100, 48000, 32000, -1},
};

auto ioresult GetAudioInfoMp3(register FILE *fp, register struct AudioInfo *ai,
			      register u_int16 flags) {
  u_int32 startPos = ftell(fp);
  u_int32 frames = -1;
  s_int16 fullRate = 0, fullRateOrMp2 = 0;
  ioresult errCode = S_ERROR;
  s_int16 isMp2 = 0;

  while (!feof(fp)) {
    u_int16 b[2];
    s_int16 bitRateIndex;
    s_int16 padByte;
    s_int16 bitRate;
    s_int16 frameBytes;
    s_int16 fullRateIndex;
    {
      u_int32 t = Read32BE(fp);
      b[1] = (u_int16)(t>>16);
      b[0] = (u_int16)(t);
    }
    if ((b[1] & 0xffe6) != 0xffe2) {
      if ((b[1] & 0xffe6) != 0xffe4) {
	/* Not MP2 or MP3 */
	goto finally;
      }
      isMp2 = 1;
    }
    fullRateIndex = (b[1]>>3) & 3;
    fullRate = (fullRateIndex == 3);
    fullRateOrMp2 = fullRate | isMp2;
    bitRateIndex = b[0] >> 12;
    padByte = (b[0]>>9) & 1;

    bitRate = mp3BitRates[!isMp2][fullRate][bitRateIndex];
    if (bitRate < 0) {
      /* Bad or free-form bitrate */
      goto finally;
    }

    ai->channels = ((b[0] & 0xc0) == 0xc0) ? 1 : 2;
    ai->sampleRate = mp3SampleRates[fullRateIndex][(b[0]>>10)&3];

    frameBytes =
      (s_int16)((((s_int32)(bitRate<<fullRateOrMp2)*576000)/
		 ai->sampleRate) >> 3) + padByte;

    frames++;
    strcpy(ai->formatString, isMp2 ? "MP2" : "MP3");
    if (frames >= 16 && (flags & GAIF_FAST)) {
      goto finally2;
    }

    errCode = S_OK;

    fseek(fp, frameBytes-4, SEEK_CUR);
  }
 finally:
  if (errCode == S_OK) {
    ai->seconds = frames*(fullRateOrMp2 ? 1152.0 : 576.0)/ai->sampleRate;
  }
 finally2:
  fseek(fp, startPos, SEEK_SET);
  return errCode;
}


/*
  Ogg Vorbis-specific code.
  The parser analyzes the last 8 KiB of the file, then if it finds an
  Ogg Vorbis header that matches to the serial number, it determines how
  many samples there are in the file.
*/
auto ioresult GetAudioInfoOgg(register FILE *fp, register struct AudioInfo *ai,
			      register u_int16 flags) {
  u_int32 startPos = ftell(fp);
  u_int32 oggHeader = 0;
  u_int32 serialNumber;
  ioresult errCode = S_ERROR;
  int c;

  if (Read32LE(fp) != MAKE_DI('O','g','g','S')) {
    goto finally;
  }
  fseek(fp, 10, SEEK_CUR);
  serialNumber = Read32LE(fp);
  fseek(fp, 21, SEEK_CUR);
  ai->channels = fgetc(fp);
  ai->sampleRate = Read32LE(fp);
  ai->seconds = 0.0;
  strcpy(ai->formatString, "Ogg Vorbis");
  errCode = S_OK;

  /* Analyze only 8 last KiB of the file
     (or the whole file if the file is very short) */
  if (ai->fileBytes-startPos > 8192) {
    fseek(fp, -8192, SEEK_END);
  }

  while ((c = fgetc(fp)) != EOF) {
    oggHeader = (oggHeader<<8) | c; /* Read one character */
    if (oggHeader == MAKE_ID('O','g','g','S') &&
	!fgetc(fp) && fgetc(fp) < 8) { /* Version=0, Flags<8 */
      u_int32 grPosHi, grPosLo;
      grPosLo = Read32LE(fp);
      grPosHi = Read32LE(fp);
      if (Read32LE(fp) == serialNumber) {
	ai->seconds = SamplesToTime(grPosHi, grPosLo, ai->sampleRate);
      }
    }
  }

 finally:
  fseek(fp, startPos, SEEK_SET);
  return errCode;
}



/*
  RIFF WAV -specific code.
  Handles correctly basic and WAVE_FORMAT_EXTENSIBLE PCM, u-law, a-law, and
  IMD ADPCM data. May give wildly incorrect information to other formats.
*/
auto ioresult GetAudioInfoWav(register FILE *fp, register struct AudioInfo *ai,
			      register u_int16 flags) {
  u_int32 startPos = ftell(fp);
  s_int16 bitsPerSample = -1;
  ioresult errCode = S_ERROR;
  u_int16 imaSampleAlign = 0;

  if (Read32LE(fp) != MAKE_DI('R','I','F','F') ||
      !Read32LE(fp) ||
      Read32LE(fp) != MAKE_DI('W','A','V','E')) {
    goto finally;
  }

  while (!feof(fp)) {
    u_int32 id = Read32LE(fp);
    u_int32 len = Read32LE(fp);
    if (id == MAKE_DI('f','m','t',' ')) {
      if (len >= 16) {
	u_int32 t = Read32LE(fp);
	u_int16 audioFormat = (u_int16)t;
	ai->channels = (u_int16)(t>>16);
	ai->sampleRate = Read32LE(fp);
	fseek(fp, 4, SEEK_CUR);
	imaSampleAlign = Read16LE(fp)*2/ai->channels;
	if (audioFormat != 0x11) {
	  /* Not IMA ADPCM */
	  imaSampleAlign = 0;
	}
	bitsPerSample = fgetc(fp);
	len -= 15;
	strcpy(ai->formatString, "RIFF WAV");
	errCode = S_OK;
      }
    } else if (id == MAKE_DI('d','a','t','a')) {
      double exactBitsPerSample = bitsPerSample;
      if (imaSampleAlign) {
	exactBitsPerSample = 4.0 * (imaSampleAlign)/(imaSampleAlign-7);
      }
      if (exactBitsPerSample != 0.0) {
	ai->seconds = len*(8.0) /
	  (exactBitsPerSample*ai->channels*ai->sampleRate);
	goto finally;
      }
    } else if ((s_int32)id < 0 || (s_int32)len < 0) {
      goto finally;
    }

    fseek(fp, len, SEEK_CUR);
  }

 finally:
  fseek(fp, startPos, SEEK_SET);
  return errCode;
}



/*
  Flac-specific code.
  Reads the STREAMINFO metadata block that starts all FLAC files.
*/
auto ioresult GetAudioInfoFlac(register FILE *fp,register struct AudioInfo *ai,
			       register u_int16 flags) {
  u_int32 startPos = ftell(fp);
  ioresult errCode = S_ERROR;
  u_int16 lenHi;
  u_int32 lenLo;

  if (Read32BE(fp) != MAKE_ID('f','L','a','C') /* Not FLAC */ ||
      (Read32BE(fp) & 0x7FFFFFFF) != 0x00000022 /* Not STREAMINFO */) {
    goto finally;
  }
  fseek(fp, 9, SEEK_CUR);
  {
    s_int32 t = Read32BE(fp); /* SRate in 23:4, chans in 3:1 */ 
    ai->channels = (((u_int16)t >> 1) & 7) + 1;
    ai->sampleRate = (t >> 4) & 0x000FFFFFU;
  }

  lenHi = fgetc(fp) & 0xF;
  lenLo = Read32BE(fp);
  if (lenHi || lenLo) {
    ai->seconds = SamplesToTime(lenHi, lenLo, ai->sampleRate);
  }

  strcpy(ai->formatString, "FLAC");
  errCode = S_OK;
 finally:
  fseek(fp, startPos, SEEK_SET);
  return errCode;
}



/*
  Reads and parses MPEG-4 .M4A files.
  Recognizes ALAC and AAC.
*/
#define MAX_M4A_DEPTH 8
auto ioresult GetAudioInfoM4A(register FILE *fp,register struct AudioInfo *ai,
			      register u_int16 flags) {
  s_int32 bytesLeft[MAX_M4A_DEPTH];
  u_int32 startPos = ftell(fp);
  ioresult errCode = S_ERROR;
  int firstTime = 1;
  int depth = 0;

  bytesLeft[0] = 0x7FFFFFFFU;

  while (!feof(fp) && depth >= 0) {
    s_int32 len = Read32BE(fp);
    u_int32 id = Read32BE(fp);
    if ((firstTime && id != MAKE_ID('f','t','y','p')) || len < 8) {
      goto finally;
    }
    firstTime = 0;
    if (id == MAKE_ID('m','o','o','v') || id == MAKE_ID('t','r','a','k') ||
	id == MAKE_ID('m','d','i','a') || id == MAKE_ID('m','i','n','f') ||
	id == MAKE_ID('s','t','b','l')) {
      /* Found the chunk we are after, go into it! */
      if (++depth >= MAX_M4A_DEPTH) {
	goto finally;
      }
      bytesLeft[depth] = len-8;
    } else {
      if (id == MAKE_ID('m','d','h','d') && len >= 0x20) {
	u_int32 n, timeFrame; /* usually samples, sampleRate */
	fseek(fp, 12, SEEK_CUR);
	timeFrame = Read32BE(fp);
	n = Read32BE(fp);
	len -= 20;
	if (n && timeFrame) {
	  ai->seconds = (double)n/timeFrame;
	}
      } else if (id == MAKE_ID('s','t','s','d') && len >= 0x2e) {
	/* Finally we get some real data */
	u_int32 sampleRate;
	u_int16 channels;
	fseek(fp, 12, SEEK_CUR);
	len -= 12;
	if ((id = Read32BE(fp)) == MAKE_ID('a','l','a','c') ||
	    id == MAKE_ID('m','p','4','a')) {
	  if (id == MAKE_ID('a','l','a','c')) {
	    fseek(fp, 49, SEEK_CUR);
	    ai->channels = fgetc(fp);
	    fseek(fp, 10, SEEK_CUR);
	    ai->sampleRate=Read32BE(fp);
	    len -= 4+49+1+10+4;
	    strcpy(ai->formatString, "ALAC");
	  } else {
	    fseek(fp, 17, SEEK_CUR);
	    ai->channels = fgetc(fp);
	    fseek(fp, 6, SEEK_CUR);
	    ai->sampleRate=Read16BE(fp);
	    len -= 4+17+1+4+4;
	    strcpy(ai->formatString, "AAC");
	  }
	  errCode = S_OK;
	}
      }
      bytesLeft[depth] -= len;
      if (fseek(fp, len-8, SEEK_CUR)) {
	goto finally;
      }
    }
    while (bytesLeft[depth] <= 0) {
      if (bytesLeft[depth] < 0 || --depth < 0) {
	goto finally;
      }
    }
  }

 finally:
  fseek(fp, startPos, SEEK_SET);
  return errCode;
}


auto ioresult GetAudioInfoDSD(register FILE *fp,register struct AudioInfo *ai,
			      register u_int16 flags) {
  u_int32 startPos = ftell(fp);
  u_int32 header = Read32BE(fp);
  s_int32 sz = 0;
  ioresult errCode = S_ERROR;

#if 0
  printf("GetAudioInfoDSD())\n");
#endif

  if (header == MAKE_ID('F','R','M','8')) { /* "FRM8" -> .dff */
    long dsz;

#if 0
    printf("FRM8\n");
#endif

    sz = Read64BE(fp); /* Datasize 32 LSbits */
#ifndef __VSDSP__
    printf(".dff\n");
    printf("%ld\n", (long)sz);
#endif
    if (Read32BE(fp) == MAKE_ID('D','S','D',' ')) {
      int chn = 0;
      long frq = 0;
      while (sz) {
	u_int32 chunkName = Read32BE(fp);
	dsz = Read64BE(fp);
#if 0
	printf("cName  %c%c%c%c dsz=%ld, @%ld\n",
	       (int)(chunkName>>24)&0xff,
	       (int)(chunkName>>16)&0xff,
	       (int)(chunkName>> 8)&0xff,
	       (int)(chunkName>> 0)&0xff,
	       (long)dsz, ftell(fp)-4);
#endif

	if (chunkName == MAKE_ID('P','R','O','P')) {
	  u_int32 propType = Read32BE(fp); /*should be "SND " */
	  dsz-=4;
#ifndef __VSDSP__
	  printf("pType  %c%c%c%c dsz=%ld, @%ld\n",
		 (int)(propType>>24)&0xff,
		 (int)(propType>>16)&0xff,
		 (int)(propType>> 8)&0xff,
		 (int)(propType>> 0)&0xff,
		 (long)dsz, ftell(fp)-4);
#endif
	  if (propType == MAKE_ID('S','N','D',' ')) {
	    while (dsz >= 12) {
	      u_int32 localType = Read32BE(fp);
	      long lsz;
	      lsz = Read64BE(fp);
	      dsz -= 12 + lsz;
#ifndef __VSDSP__
	      printf("lType  %c%c%c%c, lsz %ld, dsz %ld\n",
		     (int)(localType>>24)&0xff,
		     (int)(localType>>16)&0xff,
		     (int)(localType>> 8)&0xff,
		     (int)(localType>> 0)&0xff,
		     (long)lsz, (long)dsz);
#endif
	      if (localType == MAKE_ID('F','S',' ',' ')) {
		/* ID            ckID;       // 'FS '
		   double ulong  ckDataSize; // 4
		   ulong         sampleRate; // sample rate in [Hz]
		*/
		ai->sampleRate = Read32BE(fp);
		lsz -= 4;
	      } else if (localType == MAKE_ID('C','H','N','L')) {
		/*     ID           ckID;        // 'CHNL'
		       double ulong ckDataSize;
		       ushort       numChannels; // number of audio channels
		       ID           chID[];      // channels ID's
		*/
		ai->channels = Read16BE(fp);
		lsz -= 2;
	      }
	      
	      /* Skip rest of chunk */
	      if (lsz > 0) {
		fseek(fp, lsz, SEEK_CUR);
		lsz = 0;
	      }
	    }
	  }
	} else if (chunkName == MAKE_ID('D','S','D',' ')) {
	  /*     ID              ckID;           // 'DSD '
		       double ulong    ckDataSize;
		       uchar           DSDsoundData[]; // (interleaved) DSD data
	  */
#if 0
	  printf("### DDSD1, seconds %f\n", ai->seconds);
#endif
	  sz = dsz;
	  errCode = S_OK;
	  goto finally;
	}
	

	/* Skip rest of chunk */
	if (dsz > 0) {
#if 0
	  printf("Skip %ld\n", dsz);
#endif
	  fseek(fp, dsz, SEEK_CUR);
	  dsz = 0;
	}
      }
    }
    goto finally;
  }

  if (header == MAKE_ID('D','S','D',' ')) { /* "DSD " */
    long dsz;
    long fmt, cht, chn, frq, bps, bsz;
    long smp;
    u_int32 chunkName;


#if 0
    printf("### DSD, sz %ld \n", sz);
#endif
    fseek(fp, 24, SEEK_CUR);
  nextChunk:
    chunkName = Read32LE(fp);
    sz = Read64LE(fp)-12; /* Chunk size */

#if 0
    printf("### cName  %c%c%c%c sz=%ld, @%ld\n",
	   (int)(chunkName>> 0)&0xff,
	   (int)(chunkName>> 8)&0xff,
	   (int)(chunkName>>16)&0xff,
	   (int)(chunkName>>24)&0xff,
	   (long)sz, ftell(fp)-4);
#endif

    if (chunkName == MAKE_DI('f','m','t',' ')) {
      /* "fmt " */
#if 0
      Read32LE(fp); /* format version */
      fmt = Read32LE(fp); /* format ID 0:DSD RAW*/
      cht = Read32LE(fp); /* Channel Type */
      chn = Read32LE(fp); /* Channel Number */
      frq = Read32LE(fp); /* Sampling Frequency */
      ai->sampleRate = frq;
      ai->channels = chn;
      bps = Read32LE(fp); /* Bits per sample */
      smp = Read64LE(fp); /* Samples */
      bsz = Read32LE(fp); /* Block size per channel (4096) */
      sz -= 36;
#else
      fseek(fp, 12, SEEK_CUR);
      ai->channels = (u_int16)Read32LE(fp);
      ai->sampleRate = Read32LE(fp);
      sz -= 20;
#endif
    } else if (chunkName == MAKE_DI('d','a','t','a')) {
#if 0
      printf("### DDSD2 %ld\n", sz);
#endif
      errCode = S_OK;
      goto finally;
    }
    /* Skip rest of chunk */
    if (sz) {
#if 0
      printf("Skip %ld\n", sz);
#endif
      fseek(fp, sz, SEEK_CUR);
      sz = 0;
    }
    goto nextChunk;
  }


 finally:
  if (errCode == S_OK) {
    ai->sampleRate >>= 6;
    ai->seconds = (1.0/16.0)*sz/ai->sampleRate;
  }
  fseek(fp, startPos, SEEK_SET);
  return errCode;
}




/*
  AIFF -specific code.
  Handles correctly basic formats.
  May give wildly incorrect information to unknown formats.
*/
auto ioresult GetAudioInfoAiff(register FILE *fp, register struct AudioInfo *ai,
			       register u_int16 flags) {
  u_int32 startPos = ftell(fp);
  ioresult errCode = S_ERROR;

  if (Read32BE(fp) != MAKE_ID('F','O','R','M') ||
      !Read32BE(fp) ||
      Read32BE(fp) != MAKE_ID('A','I','F','F')) {
    goto finally;
  }

  while (!feof(fp)) {
    u_int32 id = Read32BE(fp);
    u_int32 len = Read32BE(fp);
    if (id == MAKE_ID('C','O','M','M')) {
      if (len >= 18) {
	u_int32 samples;
	u_int16 exponent;
	int i;
	ai->channels = Read16BE(fp);
	samples = Read32BE(fp);
	exponent = (u_int16)Read32BE(fp); /* Ignore 16 MSbs which are field sampleSize */
	ai->sampleRate = Read32BE(fp) >> (30-(exponent&0xff));
	ai->seconds = (double)samples/ai->sampleRate;
	len -= 14;
	strcpy(ai->formatString, "AIFF");
	errCode = S_OK;
	goto finally;
      }
    } else if ((s_int32)id < 0 || (s_int32)len < 0) {
      goto finally;
    }

    fseek(fp, len, SEEK_CUR);
  }

 finally:
  fseek(fp, startPos, SEEK_SET);
  return errCode;
}







/*
  AIFF -specific code.
  Handles correctly basic formats.
  May give wildly incorrect information to unknown formats.
*/
auto ioresult GetAudioInfoWma(register FILE *fp, register struct AudioInfo *ai,
			      register u_int16 flags) {
  u_int32 startPos = ftell(fp);
  ioresult errCode = S_ERROR;

  if (Read32BE(fp) != 0x3026b275 ||
      Read32BE(fp) != 0x8e66cf11 ||
      Read32BE(fp) != 0xa6d900aa ||
      Read32BE(fp) != 0x0062ce6c) {
    goto finally;
  }

  ai->channels = 0;
  ai->sampleRate = 0;
  ai->seconds = 0;
  strcpy(ai->formatString, "WMA");
  errCode = S_OK;

 finally:
  fseek(fp, startPos, SEEK_SET);
  return errCode;
}




/*
  CAF -specific code.
*/
auto ioresult GetAudioInfoCaf(register FILE *fp, register struct AudioInfo *ai,
			      register u_int16 flags) {
  u_int32 startPos = ftell(fp);
  ioresult errCode = S_ERROR;
  int isAlac = 0;

  if (Read32BE(fp) != MAKE_ID('c','a','f','f')) {
    goto finally;
  }
  Read32BE(fp); /* Skip 4 bytes */

  while (!feof(fp)) {
    u_int32 id = Read32BE(fp);
    u_int32 len = Read64BE(fp);
    if (id == MAKE_ID('d','e','s','c')) {
      if (len >= 12) {
	len -= 12;
	Read64BE(fp);
	if (Read32BE(fp) != MAKE_ID('a','l','a','c')) {
	  goto finally;
	}
	isAlac = 1;
      }
    } else if (id == MAKE_ID('k','u','k','i')) {
      u_int32 t32;
    again:
      len -= 8;
      t32 = Read64BE(fp);
      if (t32 == MAKE_ID('f','r','m','a') || t32 == MAKE_ID('a','l','a','c')) {
	Read32BE(fp); //skip 4+4+4 bytes
	len -= 4;
	if (len >= 8) {
	  goto again;
	}
      }
      if (len >= 16 && isAlac) {
	len -= 16;
	ai->channels = Read16BE(fp)&0xFF;
	Read16BE(fp);
	t32 = Read64BE(fp);
	ai->sampleRate = Read32BE(fp);
	if (t32) {
	  u_int32 fileSize;
	  fseek(fp, 0, SEEK_END);
	  fileSize = ftell(fp);
	  ai->seconds = 8.0*fileSize/t32;
	} else {
	  ai->seconds = 0.0;
	}
	strcpy(ai->formatString, "ALAC");
	errCode = S_OK;
	goto finally;
      }
    }
    fseek(fp, len, SEEK_CUR);
  }

 finally:
  fseek(fp, startPos, SEEK_SET);
  return errCode;
}



#if 0
/*
  ADIF -specific code.
*/
auto ioresult GetAudioInfoAdif(register FILE *fp, register struct AudioInfo *ai,
			       register u_int16 flags) {
  u_int32 startPos = ftell(fp);
  ioresult errCode = S_ERROR;
  int isAlac = 0;

  if (Read32BE(fp) != MAKE_ID('A','D','I','F')) {
    goto finally;
  }

  ai->channels = 0;
  ai->sampleRate = 0;
  ai->seconds = 0;
  strcpy(ai->formatString, "AAC");
  errCode = S_OK;

 finally:
  fseek(fp, startPos, SEEK_SET);
  return errCode;
}
#endif

s_int32 __mem_y adtsSampleRates[16] = {
  96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050,
  16000, 12000, 11025,  8000,     0,     0,     0,     0};

auto ioresult GetAudioInfoADTS(register FILE *fp, register struct AudioInfo *ai,
			       register u_int16 flags) {
  u_int32 startPos = ftell(fp);
  u_int32 frames = -1;
  ioresult errCode = S_ERROR;

  if (!feof(fp)) {
    s_int16 profile, rateIndex, channelConf;
    u_int16 b[2];
    {
      u_int32 t = Read32BE(fp);
      b[1] = (u_int16)(t>>16);
      b[0] = (u_int16)(t);
    }

    if ((b[1] & 0xfff6) != 0xfff0) {
      /* Not ADTS MP4 */
      goto finally;
    }
    profile = (b[0]>>14) & 3; /* 0 = main, 1 = LC, 2 = SSR, 3 = LTP */
    rateIndex = (b[0]>>10) & 15;
    channelConf = (b[0]>>6) & 7;
    /*    frameLength = (b[0]>>0) & 3; ** headers continue in next word */

    if (profile != 1) {
      goto finally;
    }

    ai->channels = 1 + (channelConf == 2 || channelConf > 6);
    ai->sampleRate = adtsSampleRates[rateIndex];

    strcpy(ai->formatString, "AAC");
  }

  errCode = S_OK;
 finally:
  if (errCode == S_OK) {
    ai->seconds = 0.0;
  }
 finally2:
  fseek(fp, startPos, SEEK_SET);
  return errCode;
}




const auto ioresult
(*analyzeFuncs[])(register FILE *fp, register struct AudioInfo *ai,
		  register u_int16 flags) = {
  GetAudioInfoMp3,
  GetAudioInfoOgg,
  GetAudioInfoWav,
  GetAudioInfoFlac,
  GetAudioInfoM4A,
  GetAudioInfoDSD,
  GetAudioInfoAiff,
  GetAudioInfoWma,
  GetAudioInfoCaf,
#if 0
  GetAudioInfoAdif,
#endif
  GetAudioInfoADTS,
  NULL
};

/*
  Wrapper for the analyzer routines.
 */
ioresult GetAudioInfo(register const char *fileName,
		      register struct AudioInfo *ai,
		      register FILE *outFile,
		      register u_int16 flags) {
  FILE *fp = NULL;
  ioresult errCode = S_ERROR;

  fp = fopen(fileName, "rb");
  if (!fp) {
    if (outFile) {
      fprintf(outFile, "Couldn't open %s\n", fileName);
    }
    goto finally;
  }

  if (outFile) {
    fprintf(outFile, "%s:\n", fileName);
  }

  errCode = GetAudioInfoFP(fp, ai, outFile, flags);

 finally:
  if (fp) {
    fclose(fp);
    fp = NULL;
  }

  return errCode;
}


/*
  Wrapper for the analyzer routines.
 */
ioresult GetAudioInfoFP(register FILE *fp,
			register struct AudioInfo *ai,
			register FILE *outFile,
			register u_int16 flags) {
  ioresult errCode = S_ERROR;
  auto ioresult
    (**currAnFunc)(register FILE *fp, register struct AudioInfo *ai,
		   register u_int16 flags) =
    analyzeFuncs;
  u_int32 startTime, endTime;
  u_int32 origPos = ftell(fp);

  /* Determine file size, put to audioInfo */
  fseek(fp, 0, SEEK_END);
  ai->fileBytes = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  startTime = ReadTimeCount();

  /* Skip ID3v2 header, if such a thing exists */
  SkipID3(fp);

  while (*currAnFunc && errCode == S_ERROR) {
    /* Initialize audioInfo (except fileBytes which already has valid data) */
    ai->seconds = -1.0;
    ai->channels = -1;
    ai->sampleRate = -1;
    ai->formatString[0] = '\0';

    errCode = (*currAnFunc)(fp, ai, flags);
    currAnFunc++;
  }

  endTime = ReadTimeCount();

  if (outFile) {
    fprintf(outFile, "     size: %3.1f KiB\n", ai->fileBytes*(1.0/1024.0));
    if (ai->formatString[0]) {
      fprintf(outFile, "   format: %s\n", ai->formatString);
    }
    if (ai->channels > 0 && ai->sampleRate > 0) {
      fprintf(outFile, "     conf: %d channels at %ld Hz\n",
	      ai->channels,
	      ai->sampleRate);
    }
    if (ai->seconds >= 0.0) {
      s_int16 minutes = (s_int16)(ai->seconds/60.0);
      fprintf(outFile, "     time: %d:%04.1f seconds\n"
	      "  bitrate: %3.1f kbit/s\n",
	      minutes, ai->seconds - (double)((s_int32)minutes*60),
	      ai->fileBytes*(8.0/1000.0)/ai->seconds);
    }
    fprintf(outFile, "  analyze: %4.2f seconds\n", (endTime-startTime)*0.001);
  } /* if (outFile) */

  fseek(fp, origPos, SEEK_SET);

 finally:
  return errCode;
}
