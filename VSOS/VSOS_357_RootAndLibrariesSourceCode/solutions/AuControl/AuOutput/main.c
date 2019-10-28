/// \file main.c VSOS3 Audio Output control program AuOutput.dl3
/// \author Henrik Herranen, VLSI Solution Oy

// DL3 files require VSOS3 kernel version 0.3x to run.
#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <string.h>
#include <mutex.h>
#include <kernel.h>
#include <aucommon.h>
#include <stdlib.h>
#include <math.h>
#include <consolestate.h>


void PrintTrace(register void *trace, register u_int16 entryNumber,
		register void *ptr) {
  if (trace) {
    RunLoadedFunction(trace, entryNumber, (u_int16)ptr);
  } else {
    printf("<no TRACE.DL3>");
  }
}


DLLENTRY(main)
ioresult main(char *parameters) {
  int nParam = RunProgram("ParamSpl", parameters);
  char *p = parameters;
  int i;
  s_int32 sampleRate = 0;
  int bits = 0, bufSize = 0, bufFill = 0;
  ioresult res = S_OK;
  s_int16 volume = -32768;
  s_int16 verbose = 0;
  FILE *audioFP = stdaudioout;
  char *audioLibName = NULL;
  u_int16 *audioLib = NULL;
  FILE *audioLibFP = NULL;
  void *trace = LoadLibrary("TRACE");

  for (i=0; i<nParam; i++) {
    if (!strcmp(p, "-v")) {
      verbose = 1;
    } else if (!strcmp(p, "+v")) {
      verbose = 0;
    } else if (!strncmp(p, "-d", 2)) {
      audioLibName = p+2;
    } else if (!strncmp(p, "-p", 2)) {
      audioFP = strtol(p+2, NULL, 0);
    } else if (!strncmp(p, "-r", 2)) {
      sampleRate = strtol(p+2, NULL, 0);
    } else if (!strncmp(p, "-b", 2)) {
      bits = strtol(p+2, NULL, 0);
    } else if (!strncmp(p, "-s", 2)) {
      bufSize = strtol(p+2, NULL, 0);
    } else if (!strncmp(p, "-l", 2)) {
      double t = atof(p+2);
      if (t >= 127.5) {
	volume = 255;
      } else if (t < -127.5) {
	volume = -255;
      } else {
	volume = (s_int16)(2*t+256)-256;
      }
    } else if (!strcmp(p, "-h")) {
      printf("Usage: AuOutput [-ddrv|-pfp|-rrate|-bbits|-sbufSize|-lvol|-v|+v|-h]\n"
	     "-ddrv\tConnect to audio driver DRV.DL3\n"
	     "-pfp\tSet output audio file pointer to fp (use with caution!)\n"
	     "-rrate\tSet sample rate to rate\n"
	     "-bbits\tNumber of bits (16 or 32)\n"
	     "-sbufSz\tSet buffer size to bufSz 16-bit words\n"
	     "-lvol\tVolume Level of maximum (vol = -128 .. 127.5)\n"
	     "-v|+v\tVerbose on|off\n"
	     "-h\tShow this help\n"
	     );
      goto finally;
    } else {
      printf("E: Bad parameter \"%s\"\n", p);
      res = S_ERROR;
      goto finally;
    }
    p += strlen(p)+1;
  }

  if (audioLibName) {
    audioLib = LoadLibrary(audioLibName);
    if (!audioLib) {
      res = S_ERROR;
      goto finally;
    }
    /* Check reference count: if 1, library wasn't already opened */
    if (audioLib[1] < 2) {
      printf("E: Library %s was not already in memory\n", audioLibName);
      res = S_ERROR;
      goto finally;
    }
    if (!(audioLibFP = (FILE *)RunLoadedFunction(audioLib, ENTRY_3, NULL))) {
      printf("E: Can't open file pointer for library %s\n", audioLibName);
      res = S_ERROR;
      goto finally;
    }
    audioFP = audioLibFP; /* Replace audio file pointer with library */
  }

  if (!audioFP) {
    printf("E: No stdaudioout or NULL ptr\n");
    res = S_ERROR;
    goto finally;
  }

  if (!sampleRate && !bits && !bufSize && volume == -32768) {
    u_int32 sampleCounter = 0, underflows = 0;
    char name[9] = "";
    if (audioFP == stdaudioout) {
      printf("stdaudioout:     0x%04x, ", audioFP);
    } else {
      printf("audioFP:         0x%04x, ", audioFP);
    }
    PrintTrace(trace, ENTRY_2, audioFP);
    if (audioFP->Identify) {
      printf("\n  ->Identify():  0x%04x, ", audioFP->Identify);
      PrintTrace(trace, ENTRY_1, audioFP->Identify);
      printf(" returns \"%s\"", audioFP->Identify(audioFP, name, 9));
    } else {
      printf("\n  ->Identify():  0x0000, <NULL>");
    }
    printf("\n  ->op:          0x%04x, ", audioFP->op);
    PrintTrace(trace, ENTRY_2, audioFP->op);
    if (audioFP->op) {
      printf("\n    ->Ioctl():   0x%04x, ", audioFP->op->Ioctl);
      PrintTrace(trace, ENTRY_1, audioFP->op->Ioctl);
      printf("\n    ->Write():   0x%04x, ", audioFP->op->Write);
      PrintTrace(trace, ENTRY_1, audioFP->op->Write);
    }
    printf("\n");
    if (ioctl(audioFP, IOCTL_AUDIO_GET_ORATE, (void *)(&sampleRate))) {
      printf("Sample rate:     unknown (probably slave mode driver)\n");
    } else {
      printf("Sample rate:     %ld\n", sampleRate);
    }
    if ((bits = ioctl(audioFP, IOCTL_AUDIO_GET_BITS, NULL)) < 0) {
      printf("Bits per sample: unknown (assuming 16)\n");
      bits = 16;
    } else {
      printf("Bits per sample: %d\n", bits);
    }
    if ((bufSize = ioctl(audioFP, IOCTL_AUDIO_GET_OUTPUT_BUFFER_SIZE, NULL)) < 0) {
      printf("Buffer size:     unknown\n");
    } else {
      printf("Buffer size:     %d 16-bit words (%d %d-bit stereo samples)\n",
	     bufSize, bufSize/(bits>>3), bits);
    }
    if ((bufFill = ioctl(audioFP, IOCTL_AUDIO_GET_OUTPUT_BUFFER_FREE, NULL)) < 0) {
      printf("Buffer fill:     unknown\n");
    } else {
      bufFill = bufSize - bufFill;
      printf("Buffer fill:     %d 16-bit words (%d %d-bit stereo samples)\n",
	     bufFill, bufFill/(bits>>3), bits);
    }
    if (ioctl(audioFP, IOCTL_AUDIO_GET_SAMPLE_COUNTER, &sampleCounter) < 0) {
      printf("Sample counter:  unknown\n");
    } else {
      printf("Sample counter:  %lu\n", sampleCounter);
    }
    if (ioctl(audioFP, IOCTL_AUDIO_GET_UNDERFLOWS, &underflows) < 0) {
      printf("Underflows:      unknown\n");
    } else {
      printf("Underflows:      %lu\n", underflows);
    }
    if ((volume = ioctl(audioFP, IOCTL_AUDIO_GET_VOLUME, NULL)) < 0) {
      printf("Volume:          unknown\n");
    } else {
      volume = 256-volume;
      printf("Volume:          %+3.1f dB of maximum level\n", 0.5*volume);
    }
    goto finally;
  }
  if (sampleRate && bits) {
    s_int32 t32 = sampleRate * ((bits == 32) ? -1 : 1);
    if (!ioctl(audioFP, IOCTL_AUDIO_SET_RATE_AND_BITS, (void *)(&t32))) {
      if (ioctl(audioFP, IOCTL_AUDIO_GET_ORATE, (void *)(&sampleRate))) {
	res = S_ERROR;
	printf("E: Couldn't set/get rate\n");
	goto finally;
      }
      if (verbose) printf("Rate %ld, bits %d\n", sampleRate, bits);
      sampleRate = 0;
      bits = 0;
    }
  }
  if (sampleRate) {
    if (ioctl(audioFP, IOCTL_AUDIO_SET_ORATE, (void *)(&sampleRate)) ||
	ioctl(audioFP, IOCTL_AUDIO_GET_ORATE, (void *)(&sampleRate))) {
      res = S_ERROR;
      printf("E: Couldn't set rate\n");
      goto finally;
    }
    if (verbose) printf("Rate %ld\n", sampleRate);
  }
  if (bits) {
    if (ioctl(audioFP, IOCTL_AUDIO_SET_BITS, (void *)(bits))) {
      res = S_ERROR;
      printf("E: Couldn't set bits\n");
      goto finally;
    }
    if (verbose) printf("Bits %d\n", bits);
  }
  if (bufSize) {
    if (ioctl(audioFP, IOCTL_AUDIO_SET_OUTPUT_BUFFER_SIZE, (void *)bufSize) < 0) {
      res = S_ERROR;
      printf("E: Couldn't set buffer\n");
      goto finally;
    }
    if (verbose) printf("Buffer %d words\n", bufSize);
  }
  if (volume != -32768) {
    if (ioctl(audioFP, IOCTL_AUDIO_SET_VOLUME, (void *)(256-volume)) < 0 ||
	(volume = ioctl(audioFP, IOCTL_AUDIO_GET_VOLUME, NULL)) < 0) {
      res = S_ERROR;
      printf("E: Couldn't set volume\n");
      goto finally;
    }
    volume = 256-volume;
    if (verbose) printf("Volume %+3.1f dB of maximum level\n", 0.5*volume);
  }


 finally:
  if (trace) {
    DropLibrary(trace);
    trace = NULL;
  }
  if (audioLibFP) {
    RunLoadedFunction(audioLib, ENTRY_4, (u_int16)audioLibFP);
    audioLibFP = NULL;
  }
  if (audioLib) {
    DropLibrary(audioLib);
    audioLib = NULL;
  }

  return S_OK;
}
