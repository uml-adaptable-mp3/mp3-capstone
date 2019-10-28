/* For free support for VSIDE, please visit www.vsdsp-forum.com */

// This program determines the length of an audio file.

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <string.h>
#include <stdlib.h>
#include <vo_gpio.h>
#include <timers.h>
#include <swap.h>
#include <kernel.h>
#include "audioinfo.h"

struct AudioInfo audioInfo;

/*
  AuInfo gives audio file basic information: format, channel and sample rate
  configuration, playback time, bitrate, etc.
 */
int main(char *parameters) {
  int nParam = RunProgram("ParamSpl", parameters);
  char *p = parameters;
  int res = S_ERROR;
  int flags = 0, verbose = 1;
  int i, nextIsPointer = 0;
  struct AudioInfo *addrP = NULL;

  memset(&audioInfo, 0, sizeof(audioInfo));

  for (i=0; i<nParam; i++) {
    if (nextIsPointer) {
      nextIsPointer = 0;
      addrP = (void *)strtol(p, NULL, 0);
    } else if (!strcmp(p, "-h")) {
      printf("Usage: AuInfo [-f|+f|-v|+v|-p x] [-h] [files]\n"
	     "-f|+f\tFast mode on/off (don't determine MP2/MP3 file playback time)\n"
	     "-v|+v\tVerbose mode on/off\n"
	     "-p x\tCopy audioInfo data to x\n"
	     "-h\tShow this help\n"
	     );
      res = S_OK;
      goto finally;
    } else if (!strcmp(p, "-f")) {
      flags |= GAIF_FAST;
    } else if (!strcmp(p, "+f")) {
      flags &= ~GAIF_FAST;
    } else if (!strcmp(p, "-v")) {
      verbose = 1;
    } else if (!strcmp(p, "+v")) {
      verbose = 0;
    } else if (!strcmp(p, "-p")) {
      nextIsPointer = 1;
    } else {
      GetAudioInfo(p, &audioInfo, verbose ? stdout : NULL, flags);
      if (addrP) {
	memcpy(addrP, &audioInfo, sizeof(*addrP));
      }
    }
    p += strlen(p)+1;
  }

  res = S_OK;

 finally:
  return res;
}
