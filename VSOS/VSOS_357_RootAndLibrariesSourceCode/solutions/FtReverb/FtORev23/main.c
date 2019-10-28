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
#include <speedshift.h>
#include <sysmemory.h>
#include "reverb_int.h"
#include "vs23s0x0_misc.h"

extern SIMPLE_FILE audioFile;
extern s_int16 __mem_y *fDelay;
extern u_int16 fDelayMod, fDelayLen;

#if 0
u_int16 audioMutex;
#endif
FILE *audio_orig = NULL;
extern struct RoomReverb23 __mem_y room;

//void (*RomStart)(void) = (void (*)(void)) 0x8000;


DLLENTRY(init)
ioresult init(char *parameters) {
  ioresult retVal = S_ERROR;

#if defined(USE_STDAUDIOOUT)
  audio_orig = stdaudioout;
#elif defined(USE_STDAUDIOOIN)
  audio_orig = stdaudioin;
#endif

  if (!(room.memSizeWords = InitVS23S0x0()/2)) {
    printf("E: No VS23S0x0\n");
    goto finally;
  }
  printf("VS23S0x0 size %ld KiB\n", room.memSizeWords>>9);
  DesignRoom23(&room);

#if defined(USE_STDAUDIOOUT)
  stdaudioout = &audioFile;
#elif defined(USE_STDAUDIOOIN)
  stdaudioin = &audioFile;
#endif

  audioFile.Identify = audio_orig->Identify;

  ioctl(&audioFile, IOCTL_RESTART, (void *)-1);
  {
    u_int32 sampleRate = 48000;
#if defined(USE_STDAUDIOOUT)
    ioctl(&audioFile, IOCTL_AUDIO_GET_ORATE, &sampleRate);
#elif defined(USE_STDAUDIOOIN)
    ioctl(&audioFile, IOCTL_AUDIO_GET_IRATE, &sampleRate);
#endif
    ioctl(&audioFile, IOCTL_AUDIO_SET_ORATE, &sampleRate);

#if 0
    printf("  Rate %ld", sampleRate);
#endif
  }
  retVal = S_OK;
 finally:
  return retVal;
}


DLLENTRY(fini)
void fini(void) {
  // Add code here to force release of resources such as 
  // memory allocated with malloc, entry points, 
  // hardware locks or interrupt handler vectors.
  FreeRoom23(&room, 0);
  if (fDelay) {
    FreeMemY(fDelay, fDelayLen);
    fDelay = NULL;
  }

#if defined(USE_STDAUDIOOUT)
  stdaudioout = audio_orig;
#elif defined(USE_STDAUDIOOIN)
  stdaudioin = audio_orig;
#endif
}
