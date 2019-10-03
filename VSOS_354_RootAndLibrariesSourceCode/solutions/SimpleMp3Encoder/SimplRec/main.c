/*

  Very Simple MP3 / Ogg Vorbis Encoder demonstration for VS1205 / VS1005.
  VLSI Solution 2015-10-20 HH

  This VSOS application reads audio from VS1005g's default input
  (typically line input) and writes it as an MP3/OGG file to E:OUTxxx.MP3/OGG.

  - To choose audio format and bitrate/quality, see USE_MP3 and USE_OGG below.
  - To change output bitrate/quality, replace BITRATE_KBPS/QUALITY below.
  - To unset LCD touch user interface, unset USE_LCD_TOUCH below.
    Now S1 select Next File, S2 Pause, and S4 Quit.
    - Set USE_LCD_OUTPUT if you still want output to LCD.
  - To change output file name base, replace FILE_NAME_BASE definition below.
  - To change output stereo/mono mode, replace CHANNEL_MODE below.
  - To change sample rate, set ADJUST_SAMPLE_RATE below.
  - To use microphone inputs, set USE_MIC_INPUTS below.
  - To set automatic gain control, set USE_AGC below.
    Note that the AGC library is needed even if AGC itself is not used.
  - To unset Subsonic filter if AGC is not used, unset USE_SUBSONIC below.
  - To amplify the signal with a constant, set AMPLIFY_SIGNAL_BY below.
  

  IMPORTANT!!!

  To properly work, this program requires the FILEBUF.DL3 dynamic
  library to be installed first. If you are using VSOS 3.12 or
  older, you can compile the dynamic library yourself. If using
  VSOS 3.13 or newer, the library .h files are included in the VSOS
  release. Using the library, this program allocates the largest
  contiguous Y memory chunk it can get. If it can get 38 kilobytes
  or more for buffering purposes, 256 kbps or lower MP3 files don't
  suffer from interruptions with SD cards tested by VLSI.

  This program requires the ENCMP3.DL3 dynamic library to be installed
  if you want to encode MP3 files. If you are using VSOS 3.12 or older,
  you can compile the dynamic library yourself.

  This program requires the ENCVORB.DL3 dynamic library to be installed
  if you want to encode Ogg Vorbis files. If you are using VSOS 3.12 or
  older, you can compile the dynamic library yourself.

  Unless you disable both Automatic Gain Control and the Subsonic Filter,
  the AGC.DL3 dynamic library needs to be installed. If you are using
  VSOS 3.12 or older, you can compile the dynamic library yourself.


  RUNNING WITH VS1005G BREAKOUT BOARD

  On the VS1005g BOB buttons are connected as follows:
  S1 = GPIO0_0, S2 = GPIO0_1, S3 = GPIO0_2, S4 = GPIO0_3
  Pull these pins to ground with 100k resistors.
  To push a button, pull the pin up to IOVDD with a 10k resistor.
  If you don't have anything multiplexed to the pin, you don't
  need the pull-up resistor.


  KNOWN BUGS

  At the moment of writing this (2015-10-20), the ENCVORB library has
  a bug that makes the Next button freeze.


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
#include <aucommon.h>
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
#include <vo_gpio.h>
#include <kernel.h>
#include "libencmp3.h"
#include "libencvorb.h"

/* The following symbols are still missing from encoder.h 2012-12-20 */
#ifndef ENCODER_QUALITY_QUALITY
#define ENCODER_QUALITY_QUALITY 0x0000U
#define ENCODER_QUALITY_VBR     0x4000U
#define ENCODER_QUALITY_ABR     0x8000U
#define ENCODER_QUALITY_CBR     0xC000U
#define ENCODER_QUALITY_MULT10  0x0000U
#define ENCODER_QUALITY_MULT100 0x1000U
#define ENCODER_QUALITY_MULT1K  0x2000U
#define ENCODER_QUALITY_MULT10K 0x3000U
#endif


/* ----- USER ADJUSTABLE PARAMETERS START HERE!!! ----- */


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

  If you set bitrate to higher than 256 (bit/s), and depending on
  the SD card that is used, there may be interruptions in sound.

  For near-CD quality at least 192 kbit/s at 48000 Hz is recommended.

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
     /* MP3 VBR example with quality setting 7 (on the scale of 0 to 10) */
#    define QUALITY_SETTING 7
#  endif
#endif

/*
  Encode in Ogg Vorbis format.

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
#  define QUALITY_SETTING 7
#endif

/* If defined, use LCD interface. Otherwise use two-button interface. */
#if 1
#  define USE_LCD_TOUCH
#  define USE_LCD_OUTPUT
#else
#  if 0
     /* Even if you don't use the LCD touch interface, you have the option
        of using LCD to see output. If this is not defined, only stdout
	and stderr will be used for output. */
#    define USE_LCD_OUTPUT
#  endif
#endif

/* Output file name base. Make the actual file name part 5 characters "
   or less so the the three-digit file number can be added to it. */
#define FILE_NAME_BASE "E:OUT"

/* Channel mode (see enumerations below for alternatives) */
#define CHANNEL_MODE chmJointStereo

enum ChannelMode {
  chmJointStereo = 0,	/* Normal stereo */
  chmLeft = 2,		/* Mono, use only left channel */
  chmRight,		/* Mono, use only right channel */
  chmMono		/* Mono downmix of left & right */
};

/*
  Make this "#if 1" if you want to adjust the sample rate from the
  default 48000 Hz. As of writing this (2014-09-23) VSOS only
  supports 24000 and 48000 Hz (only 32000 Hz supported for FM
  radio recording), but this may change in the future.
  If you request a sample rate that is not available, the closest
  one will be chosen.
*/
#if 0
#define ADJUST_SAMPLE_RATE 24000UL
#endif

/*
  Make this "#if 1" if you want to use microphone inputs instead of
  the stereo line input. If you are using the VS1005 Developer Board,
  you also need to adjust some jumpers if you want to use the right
  channel:
  Mic:
  - Move JP5 jumper left to MIC2 pins 3 & 2.
  - Insert JP6 jumper left to MIC2 pins 3 & 2.
  Line in:
  - Move JP5 jumper right to LINE2 pins 2 & 1.
  - Remove JP6 jumper.
  Note that AGC is very useful when using the microphone input.
*/
#if 0
#define USE_MIC_INPUTS
#endif

/*
  Make this #if 1 if you want to use Automatic Gain Control (AGC)
  AGC_MAX_GAIN may vary between 1.0 and 64.0.
  Recommended value is somewhere between 2.0 (+6 dB) and 8.0 (+18 dB).
  With higher values microphone background noise may become
  disturbing.
*/
#if 0
#  define USE_AGC
#  define AGC_MAX_GAIN (4.0) /* Value/expression should be in parenthesis! */
#else
   /* If you are not using AGC, you can disable the Subsonic Filter by
      putting #if 0 here. That way the AGC library is not needed. For best
      sound quality using the Subsonic Filter is highly recommended. */
#  if 0
#    define USE_SUBSONIC
#  endif
#endif

/*
  Make this "#if 1" if you want to digitally amplify the input
  signal before encoding it to MP3.
  If using an amplification of more than 1, you run the risk of
  distorting strong signals, but this may be useful if your
  microphone signal is "always too low". The best way to solve
  this issue usually is to use an AGC filter.

  The amount of amplification is linear, so 1.0 = unchanged,
  2.0 = +6 dB, 4.0 = +12 dB, etc. Maximum value is 64, but in
  practise you should stay well below that or you might end up
  hearing noise and distortion.
 */
#if 0
#define AMPLIFY_SIGNAL_BY (2.0) /* Value/expression should be in parenthesis! */
#endif

/* ----- USER ADJUSTABLE PARAMETERS END HERE!!! ----- */





#ifdef BITRATE_KBPS
#  define QUALITY (ENCODER_QUALITY_CBR|ENCODER_QUALITY_MULT1K|BITRATE_KBPS)
#else
#  ifndef QUALITY_SETTING
     /* Default: Quality 6 */
#    define QUALITY_SETTING 6
#  endif
#  define QUALITY (ENCODER_QUALITY_QUALITY|QUALITY_SETTING)
#endif


enum butActions {
  baQuit = 1,
  baNext,
  baPause
};

#ifndef MIN
#  define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef MAX
#  define MAX(a,b) (((a)>(b))?(a):(b))
#endif

FILE *oFp = NULL;       ///< Output file pointer
struct Encoder *enc; ///< Encoder structure
enum ChannelMode channelMode = CHANNEL_MODE;

u_int16 vuMeterMemory[2] = {0, 0};
u_int16 clearVuMeter = 1;
u_int16 pauseMode = 0;

#if defined(USE_SUBSONIC) || defined(USE_AGC)
#  define MAX_AGC_CHANNELS 2
#  include <libagc.h>
struct SubsonicM subsonicM[MAX_AGC_CHANNELS];
struct SubsonicG __mem_y subsonicG;
#  ifdef USE_AGC
struct Agc agc[MAX_AGC_CHANNELS];
struct AgcConsts __mem_y agcConsts;
#  endif /* USE_AGC */
void *agcLib = NULL;
#endif /* AGC_SUBSONIC */

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

#ifdef AMPLIFY_SIGNAL_BY
auto void Gain(register __i2 s_int16 *d, register __d0 u_int16 n,
	       register __c0 u_int16 gainX1024) {  
  int i;
  for (i=0; i<n; i++) {
      *d = Sat32To16((s_int32)(*d) * gainX1024 >> 10);
      d++;
  }
}
#endif

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
      ringcpyXX((u_int16 *)data[0], 1, (u_int16 *)stb, 2, toRead);
      /* Process data */
#ifdef AMPLIFY_SIGNAL_BY
      Gain(data[0], toRead,
	(u_int16)(AMPLIFY_SIGNAL_BY >= 64.0 ? 65535U : AMPLIFY_SIGNAL_BY*1024));
#endif /* AMPLIFY_SIGNAL_BY */
#if defined(USE_SUBSONIC) || defined(USE_AGC)
      LibSubsonic(agcLib, subsonicM, &subsonicG, data[0], toRead);
#endif /* USE_SUBSONIC || USE_AGC */
#ifdef USE_AGC
      LibAgc(agcLib, agc, &agcConsts, data[0], toRead);
#endif /* USE_AGC */

      /* Copy back to stereo ring buffer so we can play it. */
      ringcpyXX((u_int16 *)stb  , 2, (u_int16 *)data[0], 1, toRead);
      ringcpyXX((u_int16 *)stb+1, 2, (u_int16 *)data[0], 1, toRead);
      fwrite(stb, sizeof(s_int16), toRead*2, stdaudioout);
      if (clearVuMeter) {
	vuMeterMemory[0] = vuMeterMemory[1] = 0;
      }
      VuMeter(data[0], toRead, vuMeterMemory);
      if (es->cancel || !pauseMode) {
	data[0] += toRead;
	left -= toRead;
      }
    }
  } else {
    u_int16 left = samples;
    while (left) {
      int toRead = MIN(left, STB/2);
      fread(stb, sizeof(s_int16), toRead*2, stdaudioin);
#ifdef AMPLIFY_SIGNAL_BY
      Gain(stb, toRead*2,
	(u_int16)(AMPLIFY_SIGNAL_BY >= 64.0 ? 65535U : AMPLIFY_SIGNAL_BY*1024));
#endif
      ringcpyXX((u_int16 *)data[0], 1, (u_int16 *)stb  , 2, toRead);
      ringcpyXX((u_int16 *)data[1], 1, (u_int16 *)stb+1, 2, toRead);
#if defined(USE_SUBSONIC) || defined(USE_AGC)
      LibSubsonic(agcLib, subsonicM  , &subsonicG, data[0], toRead);
      LibSubsonic(agcLib, subsonicM+1, &subsonicG, data[1], toRead);
#endif /* USE_SUBSONIC || USE_AGC */
#ifdef USE_AGC
      LibAgc(agcLib, agc  , &agcConsts, data[0], toRead);
      LibAgc(agcLib, agc+1, &agcConsts, data[1], toRead);
#endif /* USE_AGC */
#if defined(USE_SUBSONIC) || defined(USE_AGC)
      ringcpyXX((u_int16 *)stb  , 2, (u_int16 *)data[0], 1, toRead);
      ringcpyXX((u_int16 *)stb+1, 2, (u_int16 *)data[1], 1, toRead);
#endif /* USE_SUBSONIC || USE_AGC */
      fwrite(stb, sizeof(s_int16), toRead*2, stdaudioout);
      if (clearVuMeter) {
        vuMeterMemory[0] = vuMeterMemory[1] = 0;
      }
      VuMeter(data[0], toRead, vuMeterMemory  );
      VuMeter(data[1], toRead, vuMeterMemory+1);
      if (es->cancel || !pauseMode) {
	data[0] += toRead;
	data[1] += toRead;
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

#ifdef USE_LCD_OUTPUT
/* DB_SCALE tells how many decibels the VU mater scale is.
   It must be over 12.*/
#define DB_SCALE 40

void RenderVuButton(register struct stdButtonStruct *button,
		    register u_int16 op,
		    register u_int16 x, register u_int16 y) {
  static int i = 0;
  static int firstTime = 1;
  s_int16 xscale = (button->x2-button->x1-8)/DB_SCALE;
  s_int16 yscale = (button->y2-button->y1-10)/channels;
  s_int16 w = DB_SCALE*xscale, h=channels*yscale;
  s_int16 xs = button->x1+4, ys = button->y1+4;
  static s_int16 oldT[2] = {0, 0};

  if (xscale > 0 && yscale > 0) {
    u_int16 vuMeterCopy[2];
    if (firstTime) {
      s_int16 xm12 = xs+w-12*xscale;
      s_int16 xm6 = xs+w-6*xscale;
      s_int16 y2 = ys+h+2+2*channels-1;
      firstTime = 0;
      LcdFilledRectangle(xs-2, ys, xm12-1, y2, NULL, __RGB565RGB(  0, 128, 0));
      LcdFilledRectangle(xm12, ys,  xm6-1, y2, NULL, __RGB565RGB(128, 128, 0));
      LcdFilledRectangle( xm6, ys, xs+w+1, y2, NULL, __RGB565RGB(128,   0, 0));
    }

    vuMeterCopy[0] = vuMeterMemory[0];
    vuMeterCopy[1] = vuMeterMemory[1];
    clearVuMeter = 1;
    for (i=0; i<channels; i++) {
      u_int16 color = __RGB565RGB(0, 255, 0);
      s_int16 t;
      s_int16 dB = LinToDB(vuMeterCopy[i]+1);
      s_int16 xxe, yys, yye;

      t = (dB-(96-DB_SCALE))*4;
      if (t < 0) {
	t = 0;
      }
      if ((oldT[i]-=2) < 0) {
	oldT[i] = 0;
      }
      if (oldT[i] < t) {
	oldT[i] = t;
      }
      t = oldT[i];
      if (t > 4*(DB_SCALE-6)) {
	u_int16 tt = (4*DB_SCALE-t)*10;
	color = __RGB565RGB(255, tt, 0);
      } else if (t > 4*(DB_SCALE-12)) {
	u_int16 tt = 255-(4*(DB_SCALE-6)-t)*10;
	color = __RGB565RGB(tt, 240, 0);
      }
      xxe = xs+(t*xscale)/4;
      yys = ys+2+i*(yscale+2);
      yye = yys + yscale-1;

      LcdFilledRectangle(xs, yys, xxe-1, yye, NULL, color);
      if (t < DB_SCALE*4) {
	LcdFilledRectangle(xxe, yys, xs+w-1, yye, NULL, __RGB565RGB(0, 0, 0));
      }
    } /* for (i=0; i<channels; i++) */
  } /* if (scale > 0 && yscale >> 0) */
}
#endif

#ifdef USE_LCD_OUTPUT
StdButton buttons[9+1] = {0};
#endif

int main(void) {
  // Remember to never allocate buffers from stack space. So, if you
  // allocate the space inside your function, never forget "static"!
  int i;
  int retVal = EXIT_SUCCESS;
  u_int32 lastUITimeCount = ReadTimeCount();
  u_int32 lastSeconds = 0, sampleRate;
#ifdef USE_LCD_OUTPUT
  StdButton *textArea, *vuButton, *timeButton, *bitRateButton, *fileNameButton;
  StdButton *audioMissingButton, *pauseButton;
  StdButton *currentButton = buttons;
#endif
  void *fileBufLib = NULL, *encLib = NULL;
  struct AutoBufferElement *abe = NULL;
  int fileNumber = 0;
  char *fileName = malloc(strlen(FILE_NAME_BASE)+8);
  int nextFile = 1;
  int oldPauseMode;

  channels = (channelMode == chmJointStereo) ? 2 : 1;

  if (!fileName) {
    printf("Couldn't allocate memory for file name\n");
    retVal = EXIT_FAILURE;
    goto finally;
  }
  strcpy(fileName, "NOFILE");

  /* Prepare screen */
#ifdef USE_LCD_OUTPUT
  ioctl(&console, IOCTL_START_FRAME, "MP3/Ogg Vorbis Encoder Demo");
  SetVirtualResolution(12,9);
#endif

#ifdef USE_LCD_OUTPUT
  fileNameButton = currentButton;
  CreateStdButton(currentButton++,     -1, BTN_TEXT|BTN_NO_BEVEL|BTN_DISABLED,  0,5, 4,1, fileName);
  timeButton = currentButton;
  CreateStdButton(currentButton++,     -1, BTN_TEXT|BTN_NO_BEVEL|BTN_DISABLED,  4,5, 4,1, NULL);
  bitRateButton = currentButton;
  CreateStdButton(currentButton++,     -1, BTN_TEXT|BTN_NO_BEVEL|BTN_DISABLED,  8,5, 4,1, NULL);
  audioMissingButton = currentButton;
  CreateStdButton(currentButton++,     -1, BTN_TEXT|BTN_NO_BEVEL|BTN_DISABLED,  2,7, 8,2, NULL);
  vuButton = currentButton;
  CreateStdButton(currentButton++,     -1, BTN_INVISIBLE,  0,6,12,1, NULL);
  vuButton->render = RenderVuButton; // Custom rendering function for VU meter
#ifdef USE_LCD_TOUCH
  CreateStdButton(currentButton++, baNext,  BTN_NORMAL,    0,7, 3,2, "Next");
  pauseButton = currentButton;
  CreateStdButton(currentButton++, baPause, BTN_CHECKABLE, 3,7, 3,2, "Pause");
  CreateStdButton(currentButton++, baQuit,  BTN_NORMAL,    9,7, 3,2, "Stop");
#endif
  textArea=currentButton;
  CreateStdButton(currentButton++, BTN_END, BTN_INVISIBLE|BTN_TEXT,
                  0,0, 12,5, "");

  SetClippingRectangleToButton(textArea);

  RenderStdButtons(buttons);
#endif

  printf("VSOS 3.25 Simple MP3/OGG Encoder v1.04\n"
         "2014-2015 VLSI Solution\n\n");
#ifndef USE_LCD_TOUCH
  printf("Push S1 for next file, S4 to quit\n\n");
#endif

  if (!stdaudioin || !stdaudioout) {
    printf("No audio in/out!\n");
    retVal = EXIT_FAILURE;
    goto finally;
  }

  fileBufLib = LoadLibrary("filebuf");
  if (!fileBufLib) {
    printf("Couldn't open file buffering library FILEBUF.DL3!\n");
    retVal = EXIT_FAILURE;
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

#ifdef USE_MIC_INPUTS
  ioctl(stdaudioin, IOCTL_AUDIO_SELECT_INPUT, (void *)(AID_MIC1|AID_MIC2));
#endif

  /* Make file buffer at least 1024 stereo samples long to make sure
     sound is uninterrupted even while the encoder is running. */
  if (ioctl(stdaudioin, IOCTL_AUDIO_GET_INPUT_BUFFER_SIZE, NULL) < 2048) {
    ioctl(stdaudioin, IOCTL_AUDIO_SET_INPUT_BUFFER_SIZE, (void *)2048);
  }

#ifdef ADJUST_SAMPLE_RATE
  /* Request change of sample rate.
     Note that the input sample rate can be adjusted only with a few
     steps, so you after setting the sample rate you need to reread
     what you actually got. */
  sampleRate = ADJUST_SAMPLE_RATE;
  ioctl(stdaudioin, IOCTL_AUDIO_SET_IRATE, (void *)(&sampleRate));
#endif

  ioctl(stdaudioin, IOCTL_AUDIO_GET_IRATE, (void *)(&sampleRate));
#ifdef BITRATE_KBPS
  printf("Recording %ld Hz %s at %d kbit/s\n",
	 sampleRate, (channels == 1) ? "mono" : "stereo",
	 BITRATE_KBPS);
#else
  printf("Recording %ld Hz %s at quality %d\n",
	 sampleRate, (channels == 1) ? "mono" : "stereo",
	 QUALITY_SETTING);
#endif

  ioctl(stdaudioout, IOCTL_AUDIO_SET_ORATE, &sampleRate);

#if defined(USE_SUBSONIC) || defined(USE_AGC)
  agcLib = LoadLibrary("agc");
  if (!agcLib) {
    printf("Couldn't open AGC library AGC.DL3!\n");
    retVal = EXIT_FAILURE;
    goto finally;
  }

  LibInitSubsonic(agcLib, subsonicM, channels, &subsonicG, sampleRate);
#ifdef USE_AGC
  {
    u_int16 maxGain = (AGC_MAX_GAIN >= 64.0) ? 65535U : AGC_MAX_GAIN*1024;
    LibInitAgc(agcLib, agc, channels, &agcConsts, maxGain, sampleRate);
  }
#endif /* USE_AGC */
#endif /* USE_SUBSONIC || USE_AGC */

  while (nextFile) {
    while (1) {
      FILE *fp;
#ifdef USE_MP3
      sprintf(fileName, "%s%03d.MP3", FILE_NAME_BASE, fileNumber);
#else
      sprintf(fileName, "%s%03d.OGG", FILE_NAME_BASE, fileNumber);
#endif
      fp = fopen(fileName, "rb");
      if (!fp) {
	break;
      }
      fclose (fp);
      if (++fileNumber > 999) {
	printf("No free file names\n");
	retVal = EXIT_FAILURE;
	goto finally;
      }
    }

#ifdef USE_LCD_OUTPUT
    RenderStdButton(fileNameButton);
#else
    printf("\nFile: %s\n", fileName);
#endif

    oFp = fopen(fileName, "wb");
    if (!oFp) {
      printf("Couldn't open write file %s\n", fileName);
      retVal = EXIT_FAILURE;
      goto finally;
    }

    /* Prepare EncoderServices structure */
    memset(&es, 0, sizeof(es));
    es.Read = ESRead;
    es.Output = ESOutput;
    pauseMode = 0;
    oldPauseMode = 2;

    /* Create the encoder (same call convention for both MP3 and Ogg Vorbis
       formats, so no need to have separate code for different encoders. */
    enc = LibEncMp3Create(encLib, &es, channels, sampleRate, QUALITY);
    if (!enc) {
      printf("Couldn't create encoder\n");
      retVal = EXIT_FAILURE;
      goto finally;
    }

    /* Attach file buffer. Make it as big as possible upto 49152 bytes. */
    {
      u_int16 bufferSize = 24576; /* In 16-bit words */
      while (bufferSize >= 2048 &&
	     !(abe = CreateFileBuf(fileBufLib, oFp, bufferSize))) {
	bufferSize -= 1024;
      }
      if (!abe) {
	printf("ERROR: Couldn't attach output file buffer!\n");
	retVal = EXIT_FAILURE;
	goto finally;
      }
      printf("Output buffer size is %u bytes\n", bufferSize*2);
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
    /* Start the encoder task function EncoderTask(). TASK_DECODER
       starts as a default at a higher priority than this main task. */
    StartTask(TASK_DECODER, EncoderTask);

    /* Main loop runs as long as EncoderTask() has not ended.
       The main loop will take care of writing the MP3/Vorbis data
       EncoderTask() has created into the ring buffer. Also the user
       interface is handled here. */

    while (pSysTasks[TASK_DECODER].task.tc_State &&
	   pSysTasks[TASK_DECODER].task.tc_State != TS_REMOVED) {
      u_int32 tmpTimeCount = ReadTimeCount();
      static u_int32 totalIoBlocked = 0;

      /* If there is data to write, and less then 1/50 seconds since user
	 interface was last handled... */
      if (tmpTimeCount-lastUITimeCount >= TICKS_PER_SEC/50) {
#ifndef USE_LCD_TOUCH
	static u_int16 lastButtonPress = 0;
#endif
	u_int16 buttonPress;


	/* Check that the output file has not grown close to VSOS's
	   capabilities to handle -> limit file size to just over
	   1 billion bytes, then autojump to a new file number. */
	if (ftell(oFp) > 1000000000UL) {
	  es.cancel = 1;
	  nextFile = 1;
	}

#ifdef USE_LCD_TOUCH
	buttonPress = GetStdButtonPress(buttons);
#else
	/* If no touch interface, read button S1 for Next, S4 for Quit. */
	buttonPress = 0;

	if (GpioReadPin(0x00)) {
	  buttonPress = baNext;
	} else if (GpioReadPin(0x01)) {
	  buttonPress = baPause;
	} else if (GpioReadPin(0x03)) {
	  buttonPress = baQuit;
	} else {
	  buttonPress = 0;
	}
	if (buttonPress == lastButtonPress) {
	  buttonPress = 0; /* If key (e.g. baNext) is being held, don't react.*/
	} else {
	  lastButtonPress = buttonPress;
	}
#endif

	/* Handle user interface */
	switch (buttonPress) {
	case baQuit:
	  es.cancel = 1; // Asks the encoder task to stop.
	  nextFile = 0;
	  break;
	case baNext:
	  es.cancel = 1;
	  nextFile = 1;
	  break;
	case baPause:
	  pauseMode = 1-pauseMode;
	  break;
	}

	if (pauseMode != oldPauseMode) {
#ifdef USE_LCD_TOUCH
	  pauseButton->flags = pauseMode ?
	    (BTN_CHECKABLE|BTN_CHECKED) : BTN_CHECKABLE;
	  RenderStdButton(pauseButton);
#else
	  if (oldPauseMode < 2) {
#ifdef USE_LCD_OUTPUT
	    printf(pauseMode ? "Pause on\n" : "Pause off\n");
#else
	    printf(pauseMode ? "\nPause on" : "\rPause off");
#endif
	  }
#endif
	  oldPauseMode = pauseMode;
	}

	if (abe->ioBlocked != totalIoBlocked) {
	  static char s[40];
	  totalIoBlocked = abe->ioBlocked;
#ifdef USE_LCD_OUTPUT
	  sprintf(s, "WARNING:\n%5.3f SECS AUDIO MISSING",
		  abe->ioBlocked*0.001);
	  audioMissingButton->caption = s;
	  RenderStdButton(audioMissingButton);
#else
	  printf(s, "WARNING:\n%5.3f SECS AUDIO MISSING",
		 abe->ioBlocked*0.001);
#endif
	}

	if (es.playTimeSeconds != lastSeconds) {
	  double br = es.avgBitRate*0.001;
#ifdef USE_LCD_OUTPUT
	  static char s1[9], s2[14], s3[14] = "";
#endif

	  lastSeconds = es.playTimeSeconds;
	  
#ifdef USE_LCD_OUTPUT
	  sprintf(s1, "%5lu:%02lu", lastSeconds/60, lastSeconds%60);
	  timeButton->caption = s1;
	  RenderStdButton(timeButton);
	  sprintf(s2, "%5.1lf kbit/s", es.avgBitRate*0.001);
	  if (strcmp(s2, s3)) {
	    strcpy(s3, s2);
	    bitRateButton->caption = s2;
	    RenderStdButton(bitRateButton);
	  }
#else
	  {
	    u_int16 vuLin;
	    s_int16 vuDB;
	    const char *warnStr = "             ";
	    vuLin = MAX(vuMeterMemory[0], vuMeterMemory[1]);
	    clearVuMeter = 1;
	    vuDB = LinToDB(vuLin) - 90;
	    if (vuDB >= 5) {
	      warnStr = "OVERLOAD!!!!!";
	    } else if (vuDB > 0) {
	      warnStr = "WARNING: LOUD";
	    }

	    printf("\r%2lu:%02lus, ", lastSeconds/60, lastSeconds%60);
	    printf("%5.1lf kbit/s ", es.avgBitRate*0.001);
	    printf("%+3d dB %s", vuDB, warnStr);
	  }
#endif
	}

#ifdef USE_LCD_OUTPUT
	RenderStdButton(vuButton);
#endif
	lastUITimeCount = tmpTimeCount;
      }  else { /* if (timeSinceLastUpdate > 20 ms) */
	Delay(TICKS_PER_SEC/100); /* Be multitasking-friendly */
      }
    } /* while (encoder task running) */

    if (enc) {
      if (encErrCode > 0) {
	printf("Encode errCode %d, errStr \"%s\"\n", encErrCode, encErrStr);
	retVal = EXIT_FAILURE;
      }

      //    printf("Delete encoder\n");
      enc->Delete((struct Encoder *)enc);
      enc = NULL;
    }

    if (oFp) {
      //   printf("Close output file\n");
      fclose(oFp);
      oFp = NULL;
    }
  } /* while (nextFile) */

 finally:
#if defined(USE_SUBSONIC) || defined(USE_AGC)
  if (agcLib) {
    DropLibrary(agcLib);
    agcLib = NULL;
  }
#endif /* USE_SUBSONIC || USE_AGC */

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

  if (fileName) {
    free(fileName);
    fileName = NULL;
  }

#ifdef USE_LCD_TOUCH
  fclose(appFile);
  appFile = fopen("S:SimplMon.ap3", "rb");
#endif

  return retVal;
}
