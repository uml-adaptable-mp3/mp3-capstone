#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <string.h>
#include <kernel.h>
#include <aucommon.h>
#include <stdlib.h>
#include <audio.h>

/* These defines are missing from VSOS releases prior to 3.60 */
#ifndef IOCTL_AUDIO_GET_NOISE_KILLER
#define IOCTL_AUDIO_GET_NOISE_KILLER	290
#define IOCTL_AUDIO_SET_NOISE_KILLER	291
#endif /* !IOCTL_AUDIO_GET_NOISE_KILLER */

DLLENTRY(main)
ioresult main(char *parameters) {
  int nParam = RunProgram("ParamSpl", parameters);
  char *p = parameters;
  int i, fields = 0;
  FILE *fp = stdaudioin;
  double speed = -1.0, pitch = -1.0;
  int nothingToDo = 1, verbose = 0;
  ioresult errCode = S_ERROR;

  for (i=0; i<nParam; i++) {
    if (!strcmp(p, "-o")) {
      fp = stdaudioout;
    } else if (!strcmp(p, "-i")) {
      fp = stdaudioin;
    } else if (!strncmp(p, "-n", 2)) {
      s_int16 noiseLevel = (s_int16)(atof(p+2));
      if (noiseLevel < 0) noiseLevel = 0;
      if (ioctl(fp, IOCTL_AUDIO_SET_NOISE_KILLER, (void *)(noiseLevel)) < 0) {
	printf("E: Setting parameter \"%s\" failed\n", p);
      }
      nothingToDo = 0;
    } else if (!strcmp(p, "-v")) {
      verbose = 1;
    } else if (!strcmp(p, "+v")) {
      verbose = 0;
    } else if (!strcmp(p, "-h")) {
      printf("Usage: SetNoise [-i|-o] [-v|+v] [-nx] [-h]\n"
	     "-i\tSet stdaudioin (default)\n"
	     "-o\tSet stdaudioout\n"
	     "-nx\tSet noise killer level (default: 50 dB, 0 = off)\n"
	     "-v|+v\tVerbose on/off\n"
	     "-h\tShow this help\n");
      return S_OK;
    } else {
      printf("E: Unknown parameter \"%s\"\n", p);
    }
    p += strlen(p)+1;
  }

  if (nothingToDo || verbose) {
    s_int16 dB = ioctl(fp, IOCTL_AUDIO_GET_NOISE_KILLER, NULL);
    if (dB < 0) {
    printf("E: No noise killer library found\n");
      goto finally;
    }
    printf("Noise killer at %d dB\n", dB);
  }

  errCode = S_OK;
 finally:
  return errCode;
}
