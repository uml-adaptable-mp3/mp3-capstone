/// \file main.c VSOS3 Audio Device Driver with input/output (AUDIO.DL3)
/// \author Henrik Herranen, VLSI Solution Oy
// For further instructions on how to use the audio driver can be found
// in the VS1005 VSOS Programmer's Guide, available at
// http://www.vlsi.fi/en/support/software/vside.html

/* For free support for VSIDE, please visit www.vsdsp-forum.com */

// Starting point template for creating VSOS3 libraries and device drivers.
// This will create a <projectname>.DL3 file, which you can copy to 
// your VS1005 Developer Board's system disk's SYS subdirectory.

// DL3 files require VSOS3 kernel version 0.3x to run.


// If you add the libary name to S:CONFIG.TXT, and put the library 
// to S:SYS, it will be loaded and run during boot-up. 

/*
  NOTE!
  This driver is specific to VS1005g because it uses ROM symbols
  clockSpeed, AudioBufFree, and hwSampleRate.
 */
#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <string.h>
#include <mutex.h>
#include <kernel.h>
#include <aucommon.h>
#include <stdlib.h>
#include <kernel.h>
#include "auiadc.h"

FILE *stdaudioin_orig = NULL;
u_int16 audioMutex;


FILE *OpenAudioFile(const char *mode);

//void (*RomStart)(void) = (void (*)(void)) 0x8000;



const struct AudioChanToIdPattern __mem_y audioChanToIdPattern[] = {

  /*  {"i2s",    AID_I2S},
      {"spdif",  AID_SPDIF},*/
  {"dia1",   AID_DIA1},
  {"dia2",   AID_DIA2},
  {"dia3",   AID_DIA3},

  {"fm",     AID_FM},

  {"line1_1",AID_LINE1_1},
  {"line2_1",AID_LINE2_1},
  {"line3_1",AID_LINE3_1},
  {"mic1",   AID_MIC1},

  {"line1_2",AID_LINE1_2},
  {"line2_2",AID_LINE2_2},
  {"line3_2",AID_LINE3_2},
  {"mic2",   AID_MIC2},
  {"line1_3",AID_LINE1_3},
  {"dec6",   AID_DEC6},

  {NULL}
};



DLLENTRY(init)
ioresult init(char *parameters) {
  int nParam = RunProgram("ParamSpl", parameters);
  char *p = parameters;
  int i;
  s_int32 sampleRate = 0;
  u_int16 idPattern = 0;

  InitMutex(&audioMutex);
  stdaudioin_orig = stdaudioin;

  for (i=0; i<nParam; i++) {
    if (strtol(p, NULL, 0) > 0) {
      sampleRate = strtol(p, NULL, 0);
    } else if (!strcmp(p, "S") || !strcmp(p, "s")) {
      if (stdaudioin != &audioFile) {
	/* Getting mutex always succeeds because no-one else has had a chance
	   to grab hold of it yet. So we don't check the result. */
	AttemptMutex(&audioMutex);
	stdaudioin = &audioFile;
      }
    } else {
      struct AudioChanToIdPattern __mem_y *ai = audioChanToIdPattern;
      do {
	if (!strcmp(ai->name, p)) {
	  idPattern |= ai->id;
	}
	ai++;
      } while (ai->name);
    }
    p += strlen(p)+1;
  }

  audioFile.op->Ioctl(&audioFile, IOCTL_RESTART, 0);
  if (idPattern) {
    printf("  Input 0x%04x", idPattern);
    if (audioFile.op->Ioctl(&audioFile, IOCTL_AUDIO_SELECT_INPUT,
			    (void *)idPattern)) {
      printf(" BAD.");
    }
  }
  if (sampleRate) {
    audioFile.op->Ioctl(&audioFile, IOCTL_AUDIO_SET_IRATE, &sampleRate);
    audioFile.op->Ioctl(&audioFile, IOCTL_AUDIO_GET_IRATE, &sampleRate);
    printf("  Rate %ld", sampleRate);
  }

  return S_OK;
}


DLLENTRY(fini)
void fini(void) {
  // Add code here to force release of resources such as 
  // memory allocated with malloc, entry points, 
  // hardware locks or interrupt handler vectors.

  AudioClose();

  stdaudioin = stdaudioin_orig;
}


DLLENTRY(OpenAudioFile)
FILE *OpenAudioFile(const char *mode) {
  if (AttemptMutex(&audioMutex)) {
    return NULL;
  }
  return &audioFile;
}


DLLENTRY(CloseAudioFile)
ioresult CloseAudioFile(FILE *audioFP) {
  ReleaseMutex(&audioMutex);
}
