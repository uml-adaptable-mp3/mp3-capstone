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
#include "auxsyncs.h"

u_int16 (*stdaudioout_write_orig)(register __i0 VO_FILE *self, void *buf,
				  u_int16 sourceIndex, u_int16 bytes) = NULL;
IOCTL_RESULT (*stdaudioin_ioctl_orig)(register __i0 VO_FILE *self,
				      s_int16 request, IOCTL_ARGUMENT arg)=NULL;
char (*stdaudioin_identify_orig)(register __i0 void *self, char *buf, u_int16 bufSize)=NULL;
char (*stdaudioout_identify_orig)(register __i0 void *self, char *buf, u_int16 bufSize)=NULL;

#if 0
u_int16 audioMutex;
#endif


#if 0
FILE *OpenAudioFile(const char *mode);

void (*RomStart)(void) = (void (*)(void)) 0x8000;
#endif

DLLENTRY(init)
ioresult init(char *paramStr) {
#if 0
  InitMutex(&audioMutex);
#endif

  stdaudioout_write_orig = stdaudioout->op->Write;
  stdaudioout->op->Write = AudioWrite;
  stdaudioout_identify_orig = stdaudioout->Identify;
  stdaudioout->Identify = Identify;

  stdaudioin_ioctl_orig = stdaudioin->op->Ioctl;
  stdaudioin->op->Ioctl = AudioIoctl;
  stdaudioin_identify_orig = stdaudioin->Identify;
  stdaudioin->Identify = Identify;
  AudioIoctl(stdaudioin, IOCTL_RESTART, 0);

#if 0
  if (strspn(paramStr, "sS")) {
    /* Getting mutex always succeeds because no-one else has had a chance
       to grab hold of it yet. So we don't check the result. */
    AttemptMutex(&audioMutex);
#endif

#if 0
  }
#endif
  return S_OK;
}


DLLENTRY(fini)
void fini(void) {
  // Add code here to force release of resources such as 
  // memory allocated with malloc, entry points, 
  // hardware locks or interrupt handler vectors.

#if 0
  AudioClose();
#endif

  stdaudioin->Identify = stdaudioin_identify_orig;
  stdaudioin->op->Ioctl = stdaudioin_ioctl_orig;

  stdaudioout->Identify = stdaudioout_identify_orig;
  stdaudioout->op->Write = stdaudioout_write_orig;
}


#if 0
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
#endif
