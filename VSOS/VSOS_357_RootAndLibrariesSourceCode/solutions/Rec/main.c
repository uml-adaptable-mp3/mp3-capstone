/*

  Text-based MP3 / Ogg Vorbis Encoder demonstration for VS1205 / VS1005.
  VLSI Solution 2017-03-xx HH

  This VSOS application reads audio from VS1005g's default input
  (typically line input) and writes it as an MP3/OGG file to a given file.


  IMPORTANT!!!

  To properly work, this program requires either the FILEBUF.DL3
  of FBUF23.DL3 dynamic library to be installed.

  FBUF23.DL3 looks if it can find one or several VS23S010 or VS23S040
  SRAM ICs, then uses them as a file buffer, varying in size between
  128 and 512 KiB.

  FILEBUF.DL3 allocates the largest contiguous Y memory chunk it can
  get. If it can get 38 kilobytes or more for buffering purposes,
  256 kbps or lower MP3 files don't suffer from interruptions with
  SD cards tested by VLSI. Encoding FLAC is not possible with this
  library.

  This program requires the ENCMP3.DL3 dynamic library to be installed
  if you want to encode MP3 files.

  This program requires the ENCVORB.DL3 dynamic library to be installed
  if you want to encode Ogg Vorbis files.

  This program requires the ENCFLAC.DL3 dynamic library to be installed
  if you want to encode FLAC files.

  It is very recommendable to load and set up the Subsonic library
  DCBL.DL3 or the Automatic Gain Control library AGC.DL3 prior to
  starting recording.


  DISCLAIMER

  This code may be used freely in any product containing one or more ICs
  by VLSI Solution.

  No guarantee is given for the usability of this code.
  
*/
#include <vo_stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sysmemory.h>
#include <encoder.h>
#include <devAudio.h>
#include <ringbuf.h>
#include <vsostasks.h>
#include <timers.h>
#include <audio.h>
#include <vsos.h>
#include <apploader.h>
#include <kernel.h>
#include <libfilebuf.h>
#include <autobuffer.h>
#include <saturate.h>
#include <string.h>
#include <sysmemory.h>
#include <vo_gpio.h>
#include <uimessages.h>
#include <consolestate.h>
#include <strings.h>
#include <vo_fat.h>
#include <ctype.h>
#include <aucommon.h>
#include <libencmp3.h>
#include <libencvorb.h>
#include <libencflac.h>


/* ----- USER ADJUSTABLE PARAMETERS START HERE!!! ----- */


enum ChannelMode {
  chmJointStereo = 0,	/* Normal stereo */
  chmLeft = 2,		/* Mono, use only left channel */
  chmRight,		/* Mono, use only right channel */
  chmMono		/* Mono downmix of left & right */
};

/*
  To use Line in / Mic, and if you are using the VS1005 Developer Board,
  you need to adjust some jumpers if you also need the right channel:
  Mic:
  - Move JP5 jumper left to MIC2 pins 3 & 2.
  - Insert JP6 jumper left to MIC2 pins 3 & 2.
  Line in:
  - Move JP5 jumper right to LINE2 pins 2 & 1.
  - Remove JP6 jumper.
  Note that AGC is very useful when using the microphone input.
*/






#ifndef MIN
#  define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef MAX
#  define MAX(a,b) (((a)>(b))?(a):(b))
#endif

FILE *oFp = NULL;       ///< Output file pointer
struct Encoder *enc; ///< Encoder structure
enum ChannelMode channelMode = chmJointStereo;

u_int16 vuMeterMemory[2] = {0, 0};
u_int16 clearVuMeter = 1;
u_int16 pauseMode = 0;


auto void VuMeter(register __i2 const s_int16 *d, register __d0 u_int16 n,
                  register __i1 u_int16 *mem) {
  u_int16 memCopy = *mem;
  u_int16 i;
  for (i=0; i<n; i++) {
    u_int16 t = abs(*d++);
    if (t > memCopy) {
      memCopy = t;
    }
  }
  *mem = memCopy;
}


/**
   struct EncoderServices service ESRead(). Reads bytes into an u_int16
   table.

   \param es Encoder Service structure.
   \param data Pointer to destination data pointers, one for each channel.
   \param samples How many n-channel 16-bit samples to read.
        word is only half filled, only the MSB is changed.
   \param chan Number of channels.
   \return Number of samples read. 0 is an EOF or error.
*/
s_int16 ESRead(struct EncoderServices *es, s_int16 **data, s_int16 samples,
               u_int16 chan) {
  u_int16 i;
#define STB 64
  static s_int16 stb[STB]; /* Temporary buffer */
  s_int16 *dataCopy[2];
  dataCopy[0] = data[0];
  dataCopy[1] = data[1];

  if (samples < 0) {
    return -1;
  }

  if (chan == 1) {
    /* Mono */
    u_int16 left = samples;
    while (left) {
      int toRead = MIN(left, STB/2);
      fread(stb, sizeof(s_int16), toRead*2, stdaudioin);
      if (channelMode == chmRight) {
	ringcpyXX((u_int16 *)stb  , 2, (u_int16 *)stb+1, 2, toRead);
      } else if (channelMode == chmMono) {
	int i;
	s_int16 *tp = stb;
	for (i=0; i<toRead; i++) {
	  s_int16 t = Sat32To16(((s_int32)tp[0]+(s_int32)tp[1])>>1);
	  *tp++ = t; *tp++ = t;
	}
      }
      /* Now that left channel contains correct data, copy that to result */
      ringcpyXX((u_int16 *)dataCopy[0], 1, (u_int16 *)stb, 2, toRead);

      /* Copy back to stereo ring buffer so we can play it. */
      ringcpyXX((u_int16 *)stb  , 2, (u_int16 *)dataCopy[0], 1, toRead);
      ringcpyXX((u_int16 *)stb+1, 2, (u_int16 *)dataCopy[0], 1, toRead);
      fwrite(stb, sizeof(s_int16), toRead*2, stdaudioout);
      if (clearVuMeter) {
	clearVuMeter = 0;
	vuMeterMemory[0] = vuMeterMemory[1] = 0;
      }
      VuMeter(dataCopy[0], toRead, vuMeterMemory);
      if (es->cancel || !pauseMode) {
	dataCopy[0] += toRead;
	left -= toRead;
      }
    }
  } else {
    u_int16 left = samples;
    while (left) {
      int toRead = MIN(left, STB/2);
      fread(stb, sizeof(s_int16), toRead*2, stdaudioin);
      ringcpyXX((u_int16 *)dataCopy[0], 1, (u_int16 *)stb  , 2, toRead);
      ringcpyXX((u_int16 *)dataCopy[1], 1, (u_int16 *)stb+1, 2, toRead);
      fwrite(stb, sizeof(s_int16), toRead*2, stdaudioout);
      if (clearVuMeter) {
	clearVuMeter = 0;
        vuMeterMemory[0] = vuMeterMemory[1] = 0;
      }
      VuMeter(dataCopy[0], toRead, vuMeterMemory  );
      VuMeter(dataCopy[1], toRead, vuMeterMemory+1);
      if (es->cancel || !pauseMode) {
	dataCopy[0] += toRead;
	dataCopy[1] += toRead;
	left -= toRead;
      }
    }
  }

  return samples;
}




/**
   struct EncoderServices service ESOutput(). Writes data to MP3 file.

   \param es Encoder Service structure.
   \param data Interleaved audio data. This data buffer may
        be used as working space by Output().
   \param bytes How many bytes of data to write.
*/
s_int16 ESOutput(struct EncoderServices *es, u_int16 *data, s_int16 bytes) {
  if (oFp->op->Write(oFp, data, 0, bytes) == bytes) {
    return 0;
  }
  return -1;
}



const u_int16 linToDBTab[5] = {36781, 41285, 46341, 52016, 58386};

/*
  Converts a linear 16-bit value between 0..65535 to decibels.
    Reference level: 32768 = 96dB (32767 = 95dB).
  Bugs:
    - For the input of 0, 0 dB is returned, because minus infinity cannot
      be represented with integers.
    - Assumes a ratio of 2 is 6 dB, when it actually is approx. 6.02 dB.
*/
auto u_int16 LinToDB(u_int16 n) {
  int res = 96, i;

  if (!n)               /* No signal should return minus infinity */
    return 0;

  while (n < 32768U) {  /* Amplify weak signals */
    res -= 6;
    n <<= 1;
  }

  for (i=0; i<5; i++)   /* Find exact scale */
    if (n >= linToDBTab[i])
      res++;

  return res;
}


#if 0
const u_int16 DBToLinTab[6] = {
#if 0
  /* Correct rounding, so perfect complement to LinToDB() */
  34716, 38968, 43740, 49097, 55109, 61858
#else
  /* Gives nicer powers of two but does not exactly complement LinToDB() */
  32768, 36781, 41285, 46341, 52016, 58386
#endif
};

auto u_int16 DBToLin(u_int16 n) {
  u_int16 res = 1;
  if (n < 6) {
    return 0;
  }
  while ((n-=6) >= 6) {
    res <<= 1;
  }
  res = ((u_int32)res * DBToLinTab[n] + (1<<14)) >> 15;
  return res;
}
#endif





enum EncoderError encErrCode = eeOk;
const char *encErrStr = "";

/*
  This is the encoder task. Because it "only" calls the MP3 Encoder
  function, it seems very trivial. However, this tasks does all the
  encoding, and the encoder also calls ESRead() and ESOutput()
  functions through the EncoderServices structure in enc.
*/
void EncoderTask(void) {
  encErrCode = enc->Encode((struct Encoder *)enc, &encErrStr);
}


int channels = 1;

struct EncoderServices es;

#define MAX_CMD_LEN 80
char cmd[MAX_CMD_LEN] = "";

enum AudioFormat {
  afUnknown=-1,
  afMp3,
  afOgg,
  afFlac
};

char *encoderLibName[] = {"ENCMP3", "ENCVORB", "ENCFLAC"};
char *audioFmtName[] = {"MP3", "Ogg Vorbis", "FLAC"};

ioresult main(char *parameters) {
  int nParam, i;
  char *p = parameters;
  ioresult retVal = S_ERROR;
  u_int32 lastSeconds = 0, sampleRate = 0;
  void *fileBufLib = NULL, *encLib = NULL;
  struct AutoBufferElement *abe = NULL;
  int fileNumber = 0;
  char *fileName = NULL;
  int nextFile = 1;
  int oldPauseMode;
  static s_int16 cmdLen = -1;
  enum AudioFormat audioFormat = afUnknown;
  s_int16 bitRate = 0;
  s_int16 quality = 7;
  s_int16 useFileBuf = 0;
  u_int32 origOverflows = 0, lastOverflows = 0;

  nParam = RunProgram("ParamSpl", parameters);
  for (i=0; i<nParam; i++) {
    if (!strcmp(p, "-h")) {
      printf("Usage: Rec [-fm|-fo|-ff|-cs|-cm|-cl|-cr|-q{x}|-b{x}|-r{x}|-f|+f|-h] outFile\n"
	     "-fm|-fo|-ff\tFormat MP3|Ogg Vorbis|FLAC (alternative: file suffix)\n"
	     "-cs|-cm|-cl|-cr\tStereo|Mono|Left|Right\n"
	     "-qx\tSet quality to x (0-10, higher is better)\n"
	     "-bx\tSet bitrate to x kbit/s (1-511)\n"
	     "-rx\tSet sample rate to x Hz, or x kHz if x<1000\n"
	     "-f/+f\tUse / Don't Use FILEBUF library for audio data buffering\n"
	     "-h\tShow this help\n"
	     "outFile\tThe output file name (e.g. D:REC.MP3)\n");
      goto finally;
    } else if (!strcmp(p, "-fm")) {
      audioFormat = afMp3;
    } else if (!strcmp(p, "-fo")) {
      audioFormat = afOgg;
    } else if (!strcmp(p, "-ff")) {
      audioFormat = afFlac;
    } else if (!strcmp(p, "-cs")) {
      channelMode = chmJointStereo;
    } else if (!strcmp(p, "-cm")) {
      channelMode = chmMono;
    } else if (!strcmp(p, "-cl")) {
      channelMode = chmLeft;
    } else if (!strcmp(p, "-cr")) {
      channelMode = chmRight;
    } else if (!strncmp(p, "-q", 2)) {
      int t = atoi(p+2);
      if (t >= 0 && t <= 10) {
	quality = t;
	bitRate = 0;
      }
    } else if (!strncmp(p, "-b", 2)) {
      int t = atoi(p+2);
      if (t >= 0 && t <= 511) {
	bitRate = t;
	quality = -1;
      }
    } else if (!strncmp(p, "-r", 2)) {
      s_int32 t = strtol(p+2, NULL, 0);
      if (t > 0) {
	if (t < 1000) {
	  t *= 1000;
	}
	sampleRate = t;
      }
    } else if (!strcmp(p, "-f")) {
      useFileBuf = 1;
    } else if (!strcmp(p, "+f")) {
      useFileBuf = 0;
    } else if (!fileName) {
      fileName = p;
    } else {
      printf("!PARAM \"%s\"\n", p);
      goto finally;
    }
    p += strlen(p)+1;
  }

  channels = (channelMode == chmJointStereo) ? 2 : 1;

  if (!stdaudioin) {
    printf("!noStdaudioin\n");
    goto finally;
  }

  if (!fileName) {
    printf("!noFileName\n");
    goto finally;
  }

  if (audioFormat == afUnknown && (i = strlen(fileName)) >= 4) {
    char *p = fileName+i-4;
    if (!strcasecmp(p, ".mp3")) {
      audioFormat = afMp3;
    } else if (!strcasecmp(p, ".ogg")) {
      audioFormat = afOgg;
    } else if (!strcasecmp(p, ".fla") ||
	       !strcasecmp(p, ".flc")) {
      audioFormat = afFlac;
    } else if (!strcasecmp(p-1, ".flac")) {
      p[3] = '\0';
      audioFormat = afFlac;
    }
  }

  if (audioFormat == afUnknown) {
    printf("!noFormat\n");
    goto finally;
  }

  if (useFileBuf) {
    fileBufLib = LoadLibrary("filebuf");
    if (!fileBufLib) {
      printf("!FILEBUF.DL3\n");
      goto finally;
    }
  }

  encLib = LoadLibrary(encoderLibName[audioFormat]);
  if (!encLib) {
    printf("!%s.DL3\n", encoderLibName[audioFormat]);
    goto finally;
  }

  if (quality < 0 && !bitRate) {
    quality = 7;
  }

  /* Make file buffer at least 1024 stereo samples long to make sure
     sound is uninterrupted even while the encoder is running. */
  if (ioctl(stdaudioin, IOCTL_AUDIO_GET_INPUT_BUFFER_SIZE, NULL) < 2048) {
    ioctl(stdaudioin, IOCTL_AUDIO_SET_INPUT_BUFFER_SIZE, (void *)2048);
  }

  /* Note that the input sample rate can be adjusted only with a few
     steps, so you after setting the sample rate you need to reread
     what you actually got. */
  if (sampleRate) {
    ioctl(stdaudioin, IOCTL_AUDIO_SET_IRATE, (void *)(&sampleRate));
  }
  ioctl(stdaudioin, IOCTL_AUDIO_GET_IRATE, (void *)(&sampleRate));

  printf("Recording %s: %ld Hz %s",
	 audioFmtName[audioFormat],
	 sampleRate, (channels == 1) ? "mono" : "stereo");
  if (audioFormat == afFlac) {
    printf("\n");
  } else if (bitRate) {
    printf(" at %d kbit/s\n", bitRate);
  } else {
    printf(" at quality %d\n", quality);
  }

  ioctl(stdaudioout, IOCTL_AUDIO_SET_ORATE, &sampleRate);


  while (nextFile) {
    u_int16 qualityParam;
    u_int32 firstBlockNum;
    FatFileInfo *fi = NULL;
    FatDeviceInfo *di = NULL;
    DEVICE *dev;

    printf("~%04x'%s\n", UIMSG_TEXT_SHORT_FILE_NAME, fileName);

    oFp = fopen(fileName, "wb");
    if (!oFp) {
      printf("!cantOpenWriteFile:%s\n", fileName);
      goto finally;
    }
    dev = oFp->dev;
    di = dev->deviceInfo;
    fi = oFp->fileInfo;
    firstBlockNum = fi->currentFragment.startSector;

    /* Prepare EncoderServices structure */
    memset(&es, 0, sizeof(es));
    es.Read = ESRead;
    es.Output = ESOutput;
    pauseMode = 0;
    oldPauseMode = 2;

    if (audioFormat != afFlac) {
      qualityParam = bitRate ?
	(ENCODER_QUALITY_CBR|ENCODER_QUALITY_MULT1K|bitRate):
	(ENCODER_QUALITY_QUALITY|quality);
    } else {
      /*
	FLAC:
	- 0 = always mid-side (fastest)
	- 2 = choose between mid-side and discrete stereo
	- 4 = choose between all four channel modes
      */
      qualityParam = 0;
    }

    /* Create the encoder (same call convention for all encoders, so
       no need to have separate code for them. */
    i = 0;
    do {
      enc = LibEncMp3Create(encLib, &es, channels, sampleRate, qualityParam);
      if (!enc) Delay(100);
    } while (!enc && ++i < 20);

    if (!enc) {
      printf("!createEnc\n");
      goto finally;
    }

    /* Attach file buffer. Make it as big as possible upto 49152 bytes. */
    if (fileBufLib) {
      u_int16 bufferSize = 24576; /* In 16-bit words */
      while (bufferSize >= 2048 &&
	     !(abe = CreateFileBuf(fileBufLib, oFp, bufferSize))) {
	bufferSize -= 1024;
      }
      if (!abe) {
	bufferSize = 0;
      }
      printf("FILEBUF buffer size %u bytes\n", bufferSize*2);
    }

#if 0
    /* Demo for setting limited frame length for Ogg Vorbis.
       This code contains a bit of "poke" code magic and is not
       as such a very nice example. But until we have official
       support for setting the Ogg frame maximum # of samples, it
       will do. */
#ifdef USE_OGG
    {
      u_int16 *maxSamplesInFrame = ((u_int16 *)enc) + 39;
      /* When encoder has more than maxSamplesInFrame samples in one
	 Ogg frame, it will finalize it and create a new one. The
	 number may be between 128-1 and 61440. Default is 61440. */
      *maxSamplesInFrame = 2048-1;
    }
#endif /* USE_OGG */
#endif

    /* Clear audio input buffer. */
    {
      u_int16 wordsInBuffer =
	(ioctl(stdaudioin, IOCTL_AUDIO_GET_INPUT_BUFFER_FILL, NULL) + 32) & ~15;
      do {
	static u_int16 tBuf[16];
	fread(tBuf, sizeof(s_int16), 16, stdaudioin);
      } while (wordsInBuffer -= 16);
      ioctl(stdaudioin, IOCTL_AUDIO_GET_OVERFLOWS, (char *)(&origOverflows));
      lastOverflows = origOverflows;
    }

    /* Start the encoder task function EncoderTask(). TASK_DECODER
       starts as a default at a higher priority than this main task. */
    StartTask(TASK_DECODER, EncoderTask);

    /* Main loop runs as long as EncoderTask() has not ended.
       The main loop will take care of writing the MP3/Vorbis data
       EncoderTask() has created into the ring buffer. Also the user
       interface is handled here. */
    while (pSysTasks[TASK_DECODER].task.tc_State &&
	   pSysTasks[TASK_DECODER].task.tc_State != TS_REMOVED) {
      static u_int32 totalIoBlocked = 0;
      int hadAnythingToDo = 0;

      if (appFlags & APP_FLAG_QUIT) {
	es.cancel = 1; // Asks the encoder task to stop.
	nextFile = 0;
      }

      if (ioctl(stdin, IOCTL_TEST, NULL) > 0) {
	char c = fgetc(stdin);
	int targetOffset = 0;

	hadAnythingToDo = 1;

	if (cmdLen < 0) {
	  switch (c) {
	  case ' ':
	    pauseMode = !pauseMode;
	    break;
	  case 'q':
	    es.cancel = 1; // Asks the encoder task to stop.
	    nextFile = 0;
	    break;
	  case 'n':
	    es.cancel = 1;
	    nextFile = 2;
	    break;
	  case '~':
	    cmdLen = 0;
	    cmd[0] = '\0';
	    if (appFlags & APP_FLAG_ECHO) {
	      putchar('~');
	    }
	    break;
	  } /* switch(c) */
	} else { /* cmdLen >= 0 */
	  if (c==8 || c==0x7f) {
	    if (appFlags & APP_FLAG_ECHO) {
	      printf("%c[1D",27);
	      printf("%c[0K",27);
	    }
	    if (--cmdLen >= 0) {
	      cmd[cmdLen] = '\0';
	    }
	  } else {
	    if (appFlags & APP_FLAG_ECHO) {
	      putchar(c);
	    }
	    if (c != '\n') {
	      cmd[cmdLen] = c;
	      if (cmdLen < MAX_CMD_LEN-1) {
		cmdLen++;
	      }
	      cmd[cmdLen] = '\0';
	    } else { /* c == '\n' */
	      int i;
	      u_int16 message = strtol(cmd+0, NULL, 16);
	      if (cmdLen == 4) {
		if (message == UIMSG_U32_FILE_POSITION) {
		  printf("~%04x=%lu\n", UIMSG_U32_FILE_POSITION, ftell(oFp));
		} else if (message == UIMSG_U32_BITS_PER_SECOND) {
		  printf("~%04x=%lu\n", UIMSG_U32_BITS_PER_SECOND,
			 es.avgBitRate);
		} else if (message == UIMSG_S16_MAX_VU) {
		  u_int16 vuLin;
		  s_int16 vuDB;
		  vuLin = MAX(vuMeterMemory[0], vuMeterMemory[1]);
		  clearVuMeter = 1;
		  vuDB = LinToDB(vuLin)-95;
		  if (vuDB > 0) {
		    vuDB = 0;
		  }
		  printf("~%04x=%d\n", UIMSG_S16_MAX_VU, vuDB);
		} else {
		  printf("!cmdUnknown\n");
		}
	      } else if (cmdLen < 6 || (cmd[4] != '=' && cmd[4] != '\'')) {
		printf("!cmdLine\n");
	      } else {
		s_int32 val = strtol(cmd+5, NULL, 0);
		if (message == UIMSG_BUT_PAUSE) {
		  pauseMode = val;
		} else {
		  printf("!cmdUnknown\n");
		}
	      } /* c == '\n' */
	      cmdLen = -1;
	    }
	  }
	}
      } /* if (ioctl(stdin, IOCTL_TEST, NULL) > 0) */
      
      /* Check that the output file has not grown close to VSOS's
	 capabilities to handle -> limit file size to just over
	 2 billion bytes, then autojump to a new file number. */
      if (ftell(oFp) > 2000000000UL) {
	hadAnythingToDo = 1;
	es.cancel = 1;
	nextFile = 2;
      }

      if (pauseMode != oldPauseMode) {
	hadAnythingToDo = 1;
	if (oldPauseMode < 2) {
	  printf("~%04x=%d\n", UIMSG_BUT_PAUSE, pauseMode);
	}
	oldPauseMode = pauseMode;
      }

      if (es.playTimeSeconds != lastSeconds &&
	  (cmdLen < 0 || !(appFlags & APP_FLAG_ECHO))) {
	u_int32 overflows = 0;
	ioctl(stdaudioin, IOCTL_AUDIO_GET_OVERFLOWS, (char *)(&overflows));
	hadAnythingToDo = 1;
	lastSeconds = es.playTimeSeconds;
	  
	printf("~%04x=%lu\n", UIMSG_U32_PLAY_TIME_SECONDS, lastSeconds);

	if (overflows != lastOverflows) {
	  printf("Lost %ld samples, %5.3f seconds in total\n",
		 overflows-lastOverflows,
		 (double)(overflows-origOverflows)/sampleRate);
	  lastOverflows = overflows;
	}

      }

      if (!hadAnythingToDo) {
	Delay(20);
      }
    } /* while (encoder task running) */

    if (nextFile == 2) {
      /* Automatic file numbering. */
      char *p = strrchr(fileName,'.')-1;
      nextFile = 0;
      if (p) {
	while (p >= fileName && isdigit(*p) && !nextFile) {
	  if (*p == '9') {
	    *p-- = '0';
	  } else {
	    (*p)++;
	    nextFile = 1;
	  }
	}
      }
    }

    if (enc) {
      if (encErrCode > 0) {
	printf("!errCode:%d,errStr:\"%s\"\n", encErrCode, encErrStr);
	retVal = S_ERROR;
	nextFile = 0;
      } else {
	retVal = S_OK;
      }

      if (audioFormat == afFlac) {
	u_int32 samples = sampleRate*es.playTimeSeconds + es.playTimeSamples;
	if (samples / sampleRate >= es.playTimeSeconds) {
	  u_int16 t;
	  u_int16 *b = malloc(256);
	  if (b) {
	    /* Patch FLAC length data into first block of file. */

	    /*
	      FLAC FORMAT:
	       0  0   32  fLaC
	       4  0   32  00 00 00 22
	       8  0   16  Min block size
	      10  0   16  Max block size
	      12  0   24  Min frame size
	      15  0   24  Max frame size
	      18  0   20  Sample rate
	      20  4    4  number of channels - 1
	      20  7    5  bit per sample - 1
	      21  4    4  samples (36 bits)
	      26  0  128  MD5
	      42  -    -  Actual data
	    */
	    dev->BlockRead(dev, firstBlockNum, 1, b);
	    b[11] = (u_int16)(samples >> 16);
	    b[12] = (u_int16)(samples);
	    dev->BlockWrite(dev, firstBlockNum, 1, b);
	    free(b);
	  } /* if (b) */
	} /* if (can represent file length with 32 bits) */
      }
      enc->Delete((struct Encoder *)enc);
      enc = NULL;
    }

    if (oFp) {
      fclose(oFp);
      oFp = NULL;
    }
  } /* while (nextFile) */

 finally:

  if (encLib) {
    if (enc) {
      enc->Delete((struct Encoder *)enc);
      enc = NULL;
    }
    DropLibrary(encLib);
    encLib = NULL;
  }

  if (oFp) {
    fclose(oFp);
    oFp = NULL;
  }

  if (fileBufLib) {
    DropLibrary(fileBufLib);
    fileBufLib = NULL;
  }

  printf("~%04x=%d\n", UIMSG_BUT_DECODER_CLOSED, 1);

  return retVal;
}
