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
#include <taskandstack.h>
#include <consolestate.h>
#include <timers.h>
#include <mutex.h>
#include <ctype.h>
#include "auxplay.h"

u_int16 __mem_y quitAudioTask = 0;
struct TaskAndStack *taskAndStack = NULL;
u_int16 __mem_y verbose = 1;




#define BUFSIZE 64

void AudioTask(void) {
  static s_int16 myBuf[BUFSIZE];
  u_int16 firstTime = 1;
  u_int32 lastOFlow = 0, lastUFlow = 0, cnt = 0;

  while (!quitAudioTask) {
    u_int32 oFlow, uFlow;
    fread(myBuf, sizeof(s_int16), BUFSIZE, stdaudioin);
    fwrite(myBuf, sizeof(s_int16), BUFSIZE, stdaudioout);
    ioctl(stdaudioin,  IOCTL_AUDIO_GET_OVERFLOWS,  (void *)(&oFlow));
    ioctl(stdaudioout, IOCTL_AUDIO_GET_UNDERFLOWS, (void *)(&uFlow));
    if (verbose && (!(++cnt & 255))) {
      if (!firstTime) {
	if (oFlow != lastOFlow) {
	  printf("AUXSPLAY: In overflow +%ld\n", oFlow-lastOFlow);
	}
	if (uFlow != lastUFlow) {
	  printf("AUXSPLAY: Out underflow +%ld\n", uFlow-lastUFlow);
	}
      }
      firstTime = 0;
      lastOFlow = oFlow;
      lastUFlow = uFlow;
    }
  } /* while (!quitAudioTask) */
}


DLLENTRY(init)
ioresult init(char *paramStr) {
  if (tolower(*paramStr) == 'q') {
    verbose = 0;
  }

  if (!stdaudioin || !stdaudioout ||
      !(taskAndStack = CreateTaskAndStack(AudioTask, "AUXPLAY", 256, 2))) {
    return S_ERROR;
  }

  return S_OK;
}


DLLENTRY(fini)
void fini(void) {
  /* Remove task running AudioTask() */
  if (taskAndStack) {
    quitAudioTask = 1;

    while (taskAndStack->task.tc_State &&
	   taskAndStack->task.tc_State != TS_REMOVED) {
      Delay(TICKS_PER_SEC/100);
    }
    FreeTaskAndStack(taskAndStack);
  }
}
