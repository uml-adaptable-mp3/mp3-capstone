/// \file main.c VSOS3 S/PDIF Audio Device Driver AUOSPDA.DL3
/// \author Henrik Herranen, VLSI Solution Oy
// For further instructions on how to use the audio driver can be found
// in the VSOS Audio Subsystem Guide, available at
// http://www.vlsi.fi/en/products/vs1005.html


/*
  NOTE!
  This driver is specific to VS1005g because it uses some ROM symbols.
 */
#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <string.h>
#include <mutex.h>
#include <kernel.h>
#include <aucommon.h>
#include <clockspeed.h>
#include "auspdif.h"

u_int16 audioMutex;
IOCTL_RESULT (*ioctl_orig)(register __i0 VO_FILE *self,
			   s_int16 request, IOCTL_ARGUMENT arg) = NULL;


FILE *OpenAudioFile(const char *mode);

void (*RomStart)(void) = (void (*)(void)) 0x8000;
s_int32 lockSampleRate = 0;

DLLENTRY(init)
ioresult init(char *parameters) {
  s_int32 sampleRate = 96000;
  int nParam = RunProgram("ParamSpl", parameters);
  char *p = parameters;
  int i;

  audioFile.op->Ioctl(&audioFile, IOCTL_RESTART, 0);

  for (i=0; i<nParam; i++) {
    if (!strcmp(p, "48000")) {
      lockSampleRate = 48000;
    } else if (!strcmp(p, "96000")) {
      lockSampleRate = 96000;
    } else if (!strcmp(p, "V") || !strcmp(p, "v")) {
      if (!ioctl_orig) {
	ioctl_orig = stdaudioout->op->Ioctl;
	stdaudioout->op->Ioctl = AudioIoctl;
      }
    }
    p += strlen(p)+1;
  }

  InitMutex(&audioMutex);

  audioFile.op->Ioctl(&audioFile, IOCTL_AUDIO_SET_ORATE, &sampleRate);

  if (clockSpeed.masterClkSrc > MASTER_CLK_SRC_PLL ||
      (clockSpeed.peripClkHz>>clockSpeed.xtalPrescaler) != 12288000 ||
      (clockSpeed.div256 | clockSpeed.div2)) {
    printf("W: Bad S/PDIF clock\n");
  }

  return S_OK;
}


DLLENTRY(fini)
void fini(void) {
  // Add code here to force release of resources such as 
  // memory allocated with malloc, entry points, 
  // hardware locks or interrupt handler vectors.
  if (ioctl_orig) {
    stdaudioout->op->Ioctl = ioctl_orig;
  }

  AudioClose();
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
