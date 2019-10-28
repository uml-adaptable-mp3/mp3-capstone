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
#include "auxi2sm.h"

FILE *stdaudioin_orig = NULL;
FILE *stdaudioout_orig = NULL;
u_int16 audioMutex;


FILE *OpenAudioFile(const char *mode);

void (*RomStart)(void) = (void (*)(void)) 0x8000;

DLLENTRY(init)
ioresult init(char *paramStr) {
  InitMutex(&audioMutex);

  stdaudioin_orig = stdaudioin;
  stdaudioout_orig = stdaudioout;

  audioFile.op->Ioctl(&audioFile, IOCTL_RESTART, 0);

  if (strspn(paramStr, "sS")) {
    /* Getting mutex always succeeds because no-one else has had a chance
       to grab hold of it yet. So we don't check the result. */
    AttemptMutex(&audioMutex);
    stdaudioin = stdaudioout = &audioFile;
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
  stdaudioout = stdaudioout_orig;
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
