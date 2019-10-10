#include <vo_stdio.h>
#include <stdlib.h>
#include <string.h>
#include <apploader.h>
#include <saturate.h>
#include <consolestate.h>
#include <kernel.h>
#include <aucommon.h>
#include <uimessages.h>

#define BUFSIZE 32

#define LONG_MUTE 2000
#define SHORT_MUTE 1000

#define I2S_LIB_NAME "AUII2SM"

int main(void) {
  static s_int32 myBuf32[BUFSIZE];
  int stereoMode = 2; /* 0 = left, 1 = right, 2 = stereo */
  int retCode = EXIT_FAILURE;
  int muteLeft = LONG_MUTE;
  int outBits = -1, inBits = -1;
  s_int32 sampleRate = 48000;
  FILE *inputFile = stdaudioin;

  u_int16 *i2sLib = NULL;
  FILE *i2sFP = NULL;

  printf("Looping audio.\n"
	 "'0'-'9' to change input\n"
	 "'>' and '<' to change volume\n"
	 "'q' to quit\n");

  if (!stdaudioin) {
    printf("E: No stdaudioin!\n");
    goto finally;
  }

  if ((inBits = ioctl(stdaudioin, IOCTL_AUDIO_GET_BITS, NULL)) < 0) {
    printf("E: stdaudioin doesn't support IOCTL_AUDIO_GET_BITS\n");
    goto finally;
  }

  if ((outBits = ioctl(stdaudioout, IOCTL_AUDIO_GET_BITS, NULL)) < 0) {
    printf("E: stdaudioout doesn't support IOCTL_AUDIO_GET_BITS\n");
    goto finally;
  }

  if (ioctl(stdaudioin, IOCTL_AUDIO_SET_BITS, (void *)32) < 0) {
    printf("E: Couldn't set stdaudioin to 32 bits\n");
    goto finally;
  }

  if (ioctl(stdaudioin, IOCTL_AUDIO_SET_IRATE, (void *)(&sampleRate)) < 0) {
    printf("E: Couldn't set stdaudioin rate to %ld Hz\n", sampleRate);
    goto finally;
  }

  if (ioctl(stdaudioout, IOCTL_AUDIO_SET_BITS, (void *)32) < 0) {
    printf("E: Couldn't set stdaudioout to 32 bits\n");
    goto finally;
  }

  if (ioctl(stdaudioout, IOCTL_AUDIO_SET_ORATE, (void *)(&sampleRate)) < 0) {
    printf("E: Couldn't set stdaudioout rate to %ld Hz\n", sampleRate);
    goto finally; 
 }

  i2sLib = LoadLibrary(I2S_LIB_NAME);
  if (!i2sLib) {
    printf("W: Cannot load " I2S_LIB_NAME ".DL3 library\n");
  } else {
    s_int32 i2sRateAndBits = -sampleRate;
    i2sFP = (FILE *)RunLoadedFunction(i2sLib, ENTRY_3, 0);
    if (!i2sFP) {
      printf("W: Cannot open " I2S_LIB_NAME ".DL3 audio file\n");
    }
    if (ioctl(i2sFP, IOCTL_AUDIO_SET_RATE_AND_BITS, (void *)(&i2sRateAndBits)) < 0) {
      printf("E: Couldn't set I2S rate to %ld Hz and bits to 32\n", sampleRate);
      goto finally; 
    }
  }

  printf("Input: LINE1_1(pin 73) + LINE1_3(pin 69)\n");
  ioctl(stdaudioin, IOCTL_AUDIO_SELECT_INPUT, (void *)(AID_LINE1_1|AID_LINE1_3));

  while (!(appFlags & APP_FLAG_QUIT)) {
    if (ioctl(stdin, IOCTL_TEST, NULL) > 0) {
      int c = fgetc(stdin);
      switch (c) {
      case 'q':
	appFlags |= APP_FLAG_QUIT;
	break;
      case '0':
	if (!i2sFP) {
	  printf("Input: Cannot change to I2S, missing driver\n");
	} else {
	  printf("Input: I2S\n");
	  stereoMode = 2; inputFile = i2sFP;
	  muteLeft = SHORT_MUTE;
	}
	break;
      case '1':
	printf("Input: LINE1_1(pin 73) + LINE1_3(pin 69)\n");
	ioctl(stdaudioin, IOCTL_AUDIO_SELECT_INPUT, (void *)(AID_LINE1_1|AID_LINE1_3));
	stereoMode = 2; inputFile = stdaudioin;
	muteLeft = SHORT_MUTE;
	break;
      case '2':
	printf("Input: LINE2_1(pin 68) + LINE2_2(pin 67)\n");
	ioctl(stdaudioin, IOCTL_AUDIO_SELECT_INPUT, (void *)(AID_LINE2_1|AID_LINE2_2));
	stereoMode = 2; inputFile = stdaudioin;
	muteLeft = SHORT_MUTE;
	break;
      case '3':
	printf("Input: LINE3_1(pin 71) + LINE3_2(pin 70)\n");
	ioctl(stdaudioin, IOCTL_AUDIO_SELECT_INPUT, (void *)(AID_LINE3_1|AID_LINE3_2));
	stereoMode = 2; inputFile = stdaudioin;
	muteLeft = SHORT_MUTE;
	break;
      case '4':
	printf("Input: LINE1_1(pin 73) + LINE1_2(pin 72)\n");
	ioctl(stdaudioin, IOCTL_AUDIO_SELECT_INPUT, (void *)(AID_LINE1_1|AID_LINE1_2));
	stereoMode = 2; inputFile = stdaudioin;
	muteLeft = SHORT_MUTE;
	break;
      case '5':
	printf("Input: MIC1(pins 72,73)\n");
	ioctl(stdaudioin, IOCTL_AUDIO_SELECT_INPUT, (void *)(AID_MIC1|AID_MIC2));
	stereoMode = 0; inputFile = stdaudioin;
	muteLeft = SHORT_MUTE;
	break;
      case '6':
	printf("Input: MIC2(pins 70,71)\n");
	ioctl(stdaudioin, IOCTL_AUDIO_SELECT_INPUT, (void *)(AID_MIC1|AID_MIC2));
	stereoMode = 1; inputFile = stdaudioin;
	muteLeft = SHORT_MUTE;
	break;
      case '7':
	printf("Input: LINE1_1(pin 73)\n");
	ioctl(stdaudioin, IOCTL_AUDIO_SELECT_INPUT, (void *)(AID_LINE1_1|AID_LINE1_3));
	stereoMode = 0; inputFile = stdaudioin;
	muteLeft = SHORT_MUTE;
	break;
      case '8':
	printf("Input: LINE1_2(pin 72)\n");
	ioctl(stdaudioin, IOCTL_AUDIO_SELECT_INPUT, (void *)(AID_LINE1_1|AID_LINE1_2));
	stereoMode = 1; inputFile = stdaudioin;
	muteLeft = SHORT_MUTE;
	break;
      case '9':
	printf("Input: LINE1_3(pin 69)\n");
	ioctl(stdaudioin, IOCTL_AUDIO_SELECT_INPUT, (void *)(AID_LINE1_1|AID_LINE1_3));
	stereoMode = 1; inputFile = stdaudioin;
	muteLeft = SHORT_MUTE;
	break;
      case '>':
      case '<':
	{
	  s_int16 t = ioctl(stdaudioout, IOCTL_AUDIO_GET_VOLUME, NULL)-256;
	  if (t >= -256) {
	    if (c == '>') {
	      if (t > 0) {
		t--;
	      }
	    } else {
	      if (t < 100) {
		t++;
	      }
	    }
	    ioctl(stdaudioout, IOCTL_AUDIO_SET_VOLUME, (void *)(t+256));
	    printf("~%04x=%d\n", UIMSG_S16_SET_VOLUME, t);
	  }
	}
	break;
      }
    }

    fread(myBuf32, sizeof(s_int32), BUFSIZE, inputFile);
    if (muteLeft) {
      muteLeft--;
      memset(myBuf32, 0, sizeof(s_int32)*BUFSIZE);
    } else {
      if (stereoMode != 2) {
	/* Mono mode, copy:
	   stereoMode == 0: left to right
	   stereoMode == 1: right to left */
	s_int32 *s = myBuf32+stereoMode;
	s_int32 *d = myBuf32+1-stereoMode;
	int i;
	for (i=0; i<BUFSIZE/2; i++) {
	  *d = *s;
	  d += 2;
	  s += 2;
	}
      }
    }
    fwrite(myBuf32, sizeof(s_int32), BUFSIZE, stdaudioout);
  } /* while (1) */
  printf("End looping audio.\n");

  retCode = EXIT_SUCCESS;

 finally:
  if (i2sFP) {
    /* Close file */
    RunLoadedFunction(i2sLib, ENTRY_4, (s_int16)i2sFP);
    i2sFP = NULL;
  }

  if (i2sLib) {
    DropLibrary(i2sLib);
    i2sLib = NULL;
  }

  if (inBits > 0) {
    ioctl(stdaudioin, IOCTL_AUDIO_SET_BITS, (void *)inBits);
  }

  if (outBits > 0) {
    ioctl(stdaudioout, IOCTL_AUDIO_SET_BITS, (void *)outBits);
  }
  
  return retCode;
}
