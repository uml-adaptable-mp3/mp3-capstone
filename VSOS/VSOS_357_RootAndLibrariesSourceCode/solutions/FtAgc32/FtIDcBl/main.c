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
#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <string.h>
#include <mutex.h>
#include <kernel.h>
#include <aucommon.h>
#include <stdlib.h>
#include <agc.h>
#include "ftAgc32.h"

#if 0
u_int16 audioMutex;
#endif
FILE *audio_orig = NULL;

//void (*RomStart)(void) = (void (*)(void)) 0x8000;


DLLENTRY(init)
ioresult init(char *parameters) {
#if 0
  int nParam = RunProgram("ParamSpl", parameters);
  char *p = parameters;
  int i;
  int in = 0, out = 0;
#endif


#if defined(USE_STDAUDIOOUT)
  audio_orig = stdaudioout;
  stdaudioout = &audioFile;
#elif defined(USE_STDAUDIOIN)
  audio_orig = stdaudioin;
  stdaudioin = &audioFile;
#endif

  ioctl(&audioFile, IOCTL_RESTART, 0);
  {
    u_int32 sampleRate = 48000;
#if defined(USE_STDAUDIOOUT)
    if (ioctl(&audioFile, IOCTL_AUDIO_GET_ORATE, &sampleRate) != S_ERROR) {
#ifdef USE_AGC
      agc32.sampleRate = sampleRate;
#else
      sampleRate = sampleRate;
#endif
    }      
#elif defined(USE_STDAUDIOIN)
    if (ioctl(&audioFile, IOCTL_AUDIO_GET_IRATE, &sampleRate) != S_ERROR) {
#ifdef USE_AGC
      agc32.sampleRate = sampleRate;
#else
      sampleRate = sampleRate;
#endif
    }      
#endif

#if 0
    printf("  Rate %ld", sampleRate);
#endif
  }

#ifdef USE_AGC
  InitAgc32(&agc32);
#endif

  return S_OK;
}


DLLENTRY(fini)
void fini(void) {
  // Add code here to force release of resources such as 
  // memory allocated with malloc, entry points, 
  // hardware locks or interrupt handler vectors.
#if defined(USE_STDAUDIOOUT)
    stdaudioout = audio_orig;
#elif defined(USE_STDAUDIOIN)
    stdaudioin = audio_orig;
#endif
}
