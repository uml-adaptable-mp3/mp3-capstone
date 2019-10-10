/*

  Wav to MP3 / Ogg Vorbis Encoder demonstration for VS1205 / VS1005.
  VLSI Solution 2015-02-16 HH

  This VSOS application reads audio from a WAV file, then encodes it
  into MP3 or Ogg Vorbis

  - To choose audio format, see USE_MP3 and USE_OGG below.
  - To change output bitrate/quality, replace BITRATE_KBPS/QUALITY below.
  - To change output file name base, replace FILE_NAME_BASE definition below.
  
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
#include <stdbuttons.h>
#include <audio.h>
#include <lcd.h>
#include <rgb565.h>
#include <vsos.h>
#include <apploader.h>
#include <libfilebuf.h>
#include <autobuffer.h>
#include <saturate.h>
#include <string.h>
#include <libagc.h>
#include <agc.h>
#include <sysmemory.h>
#include <libaudiodec.h>
#include <libencmp3.h>
#include <libencvorb.h>
#include <exec.h>
#include <kernel.h>


/* ----- USER ADJUSTABLE PARAMETERS START HERE!!! ----- */

/* Input file name without the .wav extension. Make it 8 characters
   or less. */
#define FILE_NAME_BASE "E:ENCTEST"

/*
  If #if below is set, then encode in MP3 format, otherwise in
  Ogg Vorbis format.

  If encoding MP3, and if you intend to record in Constant Bit-Rate
  format (CBR), you may use any valid MP3 bitrate. The alternatives are:
  For 32000, 44100, and 48000 Hz:
  - 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320
  For 8000, 11025, 12000, 16000, 22050, and 24000 Hz:
  - 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160
  If you try to select another value, the closest allowed value is
  chosen.

  For near-CD quality at least 192 kbit/s is recommended.

  It is also possible to use Variable Bit-Rate (VBR), and set a quality
  setting instead of a bitrate. Using VBR results in better-sounding
  or smaller files than using CBR.
*/
#if 1
#  define USE_MP3
#  if 0
     /* MP3 CBR example, 192 kbit/s */
#    define BITRATE_KBPS 192
#  else
     /* MP3 VBR example with quality setting 6 (on the scale of 0 to 10) */
#    define QUALITY_SETTING 7
#  endif
#endif

/*
  Options for encoding in Ogg Vorbis format.

  Ogg Vorbis is designed to be a variable bitrate (VBR) format. So, instead
  of setting a bitrate, the format works best if it is given a quality
  setting. The quality setting may vary from 0 (worst) to 10 (best).
  In this example we set quality to 6, which will typically result in
  a bitstream of approximately 150 kbit/s. However, depending on the audio
  signal, there may be lots of variation.

  The Ogg Vorbis does not have a CBR mode. If told to generate CBR, it
  will convert it to an internal quality setting and follow it.

  Note: Ogg Vorbis is a newer and technically a more advanced format than
  MP3. If MP3 compatibility is not an issue, and you don't need CBR, Ogg
  Vorbis can reach better audio fidelity.
*/
#ifndef USE_MP3
#  define USE_OGG
#  define QUALITY_SETTING 6
#endif


/* ----- USER ADJUSTABLE PARAMETERS END HERE!!! ----- */





#ifdef BITRATE_KBPS
#  define QUALITY (ENCODER_QUALITY_CBR|ENCODER_QUALITY_MULT1K|BITRATE_KBPS)
#else
#  ifndef QUALITY_SETTING
#    define QUALITY_SETTING 6     /* Default: Quality 6 */
#  endif
#  define QUALITY (ENCODER_QUALITY_QUALITY|QUALITY_SETTING)
#endif

#define SIGF_FILE_BUFFER_DATA (1<<2)


#ifndef min
#  define min(a,b) (((a)<(b))?(a):(b))
#endif

int failed = 0, cancelled = 0;
u_int16 *encLib = NULL;
struct Encoder *enc = NULL; ///< Encoder structure
AUDIO_DECODER *auDec = NULL;
struct EncoderServices es;
s_int32 decodeSeconds = 0;
struct TASK *mainTaskP = NULL;

void EncoderTask(void);

/* These are real file pointers. */
FILE *decInFp = NULL, *encOutFp = NULL;

/* These are pseudo file handles that we use to communicate between our
   decoder and encoder. */
IOCTL_RESULT DecOutIoctl(register __i0 VO_FILE *self, s_int16 request,
			 IOCTL_ARGUMENT arg) {
  return S_ERROR; /* To prevent from encoder setting 32-bit audio mode */
}


/* Variables that DecOutWrite() and EncInRead() use to communicate.
   Only one can write to them at the time, so though the functions
   are running in different processes, no explicit mutex mechanism
   is needed. */
volatile u_int16 *encDecData = NULL, encDecWords = 0;

/* Decoder write function is quite large.
   The reason the encoder is not created earlier is that the samplerate and
   the number of audio channels is not known before the decoder is started,
   but it _is_ known before the first call to DecOutWrite(). On the other
   hand, the encoder needs to know these parameters before it can be
   created, so we will create the encoder here when necessary. */
u_int16 DecOutWrite(register __i0 VO_FILE *self, void *buf, u_int16 sourceIndex, u_int16 bytes) {
  static u_int32 startTime = 0;
  u_int32 currentTime = ReadTimeCount();

  if (!startTime) {
    startTime = currentTime;
  }

  if (auDec->cs.playTimeSeconds >= decodeSeconds+10) {
    u_int32 in = ftell(decInFp);
    u_int32 out = ftell(encOutFp);
    decodeSeconds = auDec->cs.playTimeSeconds;
    printf("encoded %lds in %lds, "
	   "in %ldKiB (%5.1f kbit/s), out %ldKiB (%5.1f kbit/s)\n",
	   decodeSeconds,
	   ((currentTime-startTime)/TICKS_PER_SEC),
	   in>>10,
	   in*(8.0/1000.0)/decodeSeconds,
	   out>>10,
	   out*(8.0/1000.0)/decodeSeconds);
  }

  if (failed) {
    bytes = 0;
    goto finally;
  }

  while (encDecWords) {
    Wait(SIGF_FILE_BUFFER_DATA);
  }

  if (!enc) {
    /* Create the encoder (same call convention for both MP3 and Ogg Vorbis
       formats, so no need to have separate code for different encoders. */
    enc = LibEncMp3Create(encLib, &es, auDec->cs.channels,
			  auDec->cs.sampleRate, QUALITY);
    if (!enc) {
      failed = 1;
      auDec->cs.cancel = 1;
      printf("Couldn't create encoder\n");
      bytes = 0;
      goto finally;
    }

    /* Start the encoder task function EncoderTask(). TASK_DECODER
       starts as a default at a higher priority than this main task,
       but we need the tasks to start at the same priority for multitasking
       to work properly. */
    pSysTasks[TASK_DECODER].priority = pSysTasks[TASK_IO].priority;
    StartTask(TASK_DECODER, EncoderTask);
  }

  if (bytes) {
    encDecData = buf;       /* sourceIdx is always 0 for audio */
    encDecWords = bytes>>1; /* Always even for audio, trigger write */
    Signal(&pSysTasks[TASK_DECODER].task, SIGF_FILE_BUFFER_DATA);
  }

 finally:
  return bytes;
}

u_int16 EncInRead(register __i0 VO_FILE *self, void *buf, u_int16 destinationIndex, u_int16 bytes) {
  u_int16 wordsWritten = 0;
  u_int16 wordsLeft = bytes>>1;

  while (wordsLeft) {
    u_int16 wordsToWrite;
    if (cancelled) {
      fprintf(stderr, "### TIME TO STOP ENCODER!\n");
      goto finally;
    }

    if (!encDecWords) {
      Signal(mainTaskP, SIGF_FILE_BUFFER_DATA);
      Wait(SIGF_FILE_BUFFER_DATA);
    } else {
      wordsToWrite = min(wordsLeft, encDecWords);
      memcpy(buf, encDecData, wordsToWrite);
      buf += wordsToWrite;
      encDecData += wordsToWrite;
      encDecWords -= wordsToWrite;
      wordsLeft -= wordsToWrite;
      wordsWritten += wordsToWrite;
    }
  }

 finally:
  return 2*wordsWritten;
}

FILEOPS decOutFileOps = {
  NULL, /* Open() */
  NULL, /* Close() */
  DecOutIoctl, /* Ioctl() */
  NULL, /* Read() */
  DecOutWrite /* Write() */
};

FILE decOutFile = {
  __MASK_PRESENT|__MASK_OPEN|__MASK_WRITABLE|__MASK_READABLE|__MASK_FILE, /* Flags */
  NULL, /* Identify() */
  &decOutFileOps,
  0, /* pos */
  0, /* ungetc_buffer */
  /* There are many more fields , but they are not needed so left empty. */
};

FILEOPS encInFileOps = {
  NULL, /* Open() */
  NULL, /* Close() */
  NULL, /* Ioctl() */
  EncInRead, /* Read() */
  NULL /* Write() */
};

FILE encInFile = {
  __MASK_PRESENT|__MASK_OPEN|__MASK_WRITABLE|__MASK_READABLE|__MASK_FILE, /* Flags */
  NULL, /* Identify() */
  &encInFileOps,
  0, /* pos */
  0, /* ungetc_buffer */
  /* There are many more fields , but they are not needed so left empty. */
};




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
u_int16 ESRead(struct EncoderServices *es, s_int16 **data, u_int16 samples,
               u_int16 chan) {
  u_int16 i;
#define STB 64
  static s_int16 stb[STB]; /* Temporary buffer */

  if (chan == 1) {
    samples = fread(stb, sizeof(s_int16), samples, &encInFile);
  } else {
    u_int16 left = samples;
    /* Encoder requires stereo input data in deinterleaved format,
       while the WAV file contains interleaved data. So, let's
       deinterleave it. */
    while (left) {
      int toRead = min(left, STB/2);
      int haveRead = fread(stb, 2*sizeof(s_int16), toRead, &encInFile);
      ringcpyXX((u_int16 *)data[0], 1, (u_int16 *)stb  , 2, toRead);
      ringcpyXX((u_int16 *)data[1], 1, (u_int16 *)stb+1, 2, toRead);
      if (toRead == haveRead) {
	data[0] += toRead;
	data[1] += toRead;
	left -= toRead;
      } else {
	samples = samples-left-haveRead;
	left = 0;
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
  /* We need to be able to output at byte boundaries, so we cannot use
     fwrite(). But file method Write() can output to any byte boundary */
  if (encOutFp->op->Write(encOutFp, data, 0, bytes) == bytes) {
    return 0;
  }
  return -1;
}



enum EncoderError encErrCode = eeOk;
const char *encErrStr = "";

/*
  This is the encoder task. Because it "only" calls the MP3 Encoder
  function, it seems very trivial. However, this tasks does all the
  encoding, and the encoder also calls ESRead() and ESOutput()
  functions through the EncoderServices structure in enc.
*/
void EncoderTask(void) {
  printf("Started EncoderTask\n");
  encErrCode = enc->Encode((struct Encoder *)enc, &encErrStr);
  printf("Closing EncoderTask\n");
}







int main(void) {
  int i;
  int retVal = EXIT_SUCCESS;
  u_int16 *auDecLib = NULL;
  const char *decEStr = NULL;
  int decECode = 0;

  mainTaskP = thisTask; /* Remember main task pointer so we can signal it. */

  printf("Opening " FILE_NAME_BASE ".wav...\n");	
  decInFp = fopen(FILE_NAME_BASE ".wav", "rb");
  if (!decInFp) {
    printf("Couldn't open " FILE_NAME_BASE ".wav for reading.\n");
    retVal = S_ERROR;
    goto finally;
  }

  /* Open the audio decoding library */
  if (!(auDecLib = LoadLibrary("audiodec"))) {
    printf("Couldn't open AUDIODEC library\n");
    retVal = S_ERROR;
    goto finally;
  }
	
  /* We could give a hint of the the file format, but we'll leave it to
     the decoder to determine. This is OK if the file is seekable. */
  printf("Starting the decoder...\n");
  auDec = CreateAudioDecoder(auDecLib, decInFp, &decOutFile, NULL, auDecFGuess);
  if (!auDec) {
    printf("Couldn't create decoder\n");
    retVal = S_ERROR;
    goto finally;
  }

#ifdef USE_MP3
  encLib = LoadLibrary("encmp3");
  if (!encLib) {
    printf("Couldn't open MP3 encoder library ENCMP3.DL3!\n");
    retVal = EXIT_FAILURE;
    goto finally;
  }
#else
  encLib = LoadLibrary("encvorb");
  if (!encLib) {
    printf("Couldn't open Ogg Vorbis encoder library ENCVORB.DL3!\n");
    retVal = EXIT_FAILURE;
    goto finally;
  }
#endif

  encOutFp = fopen(FILE_NAME_BASE
#ifdef USE_MP3
		   ".MP3"
#else
		   ".OGG"
#endif
		   , "wb");
  if (!encOutFp) {
    printf("Couldn't open write file\n");
    retVal = EXIT_FAILURE;
    goto finally;
  }

  /* Prepare EncoderServices structure */
  memset(&es, 0, sizeof(es));
  es.Read = ESRead;
  es.Output = ESOutput;

  /* Now, we can just decode audio, and encoding will happen at the same
     time. */
  printf("Decoding audio...\n");
  decECode = DecodeAudio(auDecLib, auDec, &decEStr);
  printf("Decoder returns %d, \"%s\"\n", decECode, decEStr ? decEStr : "(null)");
  cancelled = 1;

  Signal(&pSysTasks[TASK_DECODER].task, SIGF_FILE_BUFFER_DATA);
  while (pSysTasks[TASK_DECODER].task.tc_State &&
	 pSysTasks[TASK_DECODER].task.tc_State != TS_REMOVED) {
    fprintf(stderr, "Waiting for encoder to stop\n");
    Delay(TICKS_PER_SEC/10);
  }

	
 finally:
  printf("Clean-up.\n");
	
  if (encLib) {
    if (enc) {
      enc->Delete((struct Encoder *)enc);
      enc = NULL;
    }
    DropLibrary(encLib);
    encLib = NULL;
  }

  if (encOutFp) {
    fclose(encOutFp);
    encOutFp = NULL;
  }

  if (auDecLib) {
    if (auDec) {
      DeleteAudioDecoder(auDecLib, auDec);
      auDec = NULL;
    }
    DropLibrary(auDecLib);
    auDecLib = NULL;
  }
  if (decInFp) {
    fclose(decInFp);
    decInFp = NULL;
  }

  return retVal;
}
