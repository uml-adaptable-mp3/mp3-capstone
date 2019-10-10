/*
  NOTE!
  This driver is specific to VS1005g
*/
#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <string.h>
#include <mutex.h>
#include <kernel.h>
#include <aucommon.h>
#include <stdlib.h>

/* These defines are missing from VSOS releases prior to 3.42 */
#ifndef IOCTL_AUDIO_GET_PITCH
#define IOCTL_AUDIO_GET_PITCH		270
#define IOCTL_AUDIO_SET_PITCH		271
#define IOCTL_AUDIO_GET_SPEED		272
#define IOCTL_AUDIO_SET_SPEED		273
#endif /* !IOCTL_AUDIO_GET_SPEED */

DLLENTRY(main)
ioresult main(char *parameters) {
  int nParam = RunProgram("ParamSpl", parameters);
  char *p = parameters;
  int i, fields = 0;
  FILE *fp = stdaudioout;
  double speed = -1.0, pitch = -1.0;
  int nothingToDo = 1;

  for (i=0; i<nParam; i++) {
    if (!strcmp(p, "-o")) {
      fp = stdaudioout;
    } else if (!strcmp(p, "-i")) {
      fp = stdaudioin;
    } else if (!strcmp(p, "-h")) {
      printf("Usage: SetPitch [-i|-o] [-sx] [-px] [-h]\n"
	     "-i\tSet stdaudioin\n"
	     "-o\tSet stdaudioout (default)\n"
	     "-sx\tSet speed to x times normal (0.68 - 1.64 if pitch=1.0)\n"
	     "-px\tSet pitch to x times normal (0.61 - 1.47 if speed=1.0)\n"
	     "-h\tShow this help\n"
	     "\nNote:\n"
	     "Correct playback requires that 0.68 <= speed/pitch <= 1.644.\n");
      return S_OK;
    } else if (!strncmp(p, "-s", 2)) {
      speed = atof(p+2);
      {
	u_int16 t = speed*16384.0 + 0.5;
	if (ioctl(fp, IOCTL_AUDIO_SET_SPEED, (void *)t) == S_ERROR) {
	  printf("E: Couldn't set speed (no speed shifter driver?)\n");
	}
	nothingToDo = 0;
      }
    } else if (!strncmp(p, "-p", 2)) {
      pitch = atof(p+2);
      {
	u_int16 t = pitch*16384.0 + 0.5;
	if (ioctl(fp, IOCTL_AUDIO_SET_PITCH, (void *)t) == S_ERROR) {
	  printf("E: Couldn't set pitch (no pitch shifter driver?)\n");
	}
	nothingToDo = 0;
      }
    }
    p += strlen(p)+1;
  }

  if (nothingToDo) {
    u_int16 t;
    if ((t = ioctl(fp, IOCTL_AUDIO_GET_PITCH, NULL)) != 0xFFFFU) {
      nothingToDo = 0;
      printf("Pitch %5.3f\n", t*(1.0/16384.0));
    }
    if ((t = ioctl(fp, IOCTL_AUDIO_GET_SPEED, NULL)) != 0xFFFFU) {
      nothingToDo = 0;
      printf("Speed %5.3f\n", t*(1.0/16384.0));
    }
    if (nothingToDo) {
      printf("E: No pitch or speed shifter library found\n");
    }
  }

  return S_OK;
}
