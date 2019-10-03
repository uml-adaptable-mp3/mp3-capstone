/// \file main.c VSOS3 Audio Input control program AuInput.dl3
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
#include <devAudio.h>

const struct AudioChanToIdPattern __mem_y audioChanToIdPattern[] = {

  /*  {"i2s",    AID_I2S},
      {"spdif",  AID_SPDIF},*/
  {"dia1",   AID_DIA1},
  {"dia2",   AID_DIA2},
  {"dia3",   AID_DIA3},

  {"fm",     AID_FM},

  {"line1_1",AID_LINE1_1},
  {"line2_1",AID_LINE2_1},
  {"line3_1",AID_LINE3_1},
  {"mic1",   AID_MIC1},

  {"line1_2",AID_LINE1_2},
  {"line2_2",AID_LINE2_2},
  {"line3_2",AID_LINE3_2},
  {"mic2",   AID_MIC2},
  {"line1_3",AID_LINE1_3},
  {"dec6",   AID_DEC6},

  {NULL}
};



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
  u_int16 idPattern = 0;
  ioresult res = S_OK;
  s_int16 verbose = 0;
  FILE *audioFP = stdaudioin;
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
    } else if (!strcmp(p, "-h")) {
      printf("Usage: AuInput [-ddrv|-pfp|-rrate|-bbits|-sbufsize|chconf|-v|+v-h]\n"
	     "-ddrv\tConnect to audio driver DRV.DL3\n"
	     "-pfp\tSet output audio driver pointer to fp (use with caution!)\n"
	     "-rrate\tSet sample rate to rate\n"
	     "-bbits\tNumber of bits (16 or 32)\n"
	     "-sbufSz\tSet buffer size to bufSz 16-bit words\n"
	     "-v|+v\tVerbose on|off\n"
	     "chconf\tAudio channel config (only with AUIADC driver, see definitions below)\n"
	     "-h\tShow this help\n"
	     "\nchconf needs either one stereo element, "
	     "or one left and one right element.\n"
	     "Stereo elements:\n"
	     "- fm\n"
	     "Left elements:\n"
	     "- line1_1, line2_1, line3_1, mic1, dia1\n"
	     "Right elements:\n"
	     "- line1_2, line2_2, line3_2, line1_3, mic2, dia2, dia3\n"
	     "\nExample:\n"
	     "auinput line1_1 line1_3\n"
	     );
      goto finally;
    } else {
      struct AudioChanToIdPattern __mem_y *ai = audioChanToIdPattern;
      int found = 0;
      do {
	if (!strcmp(ai->name, p)) {
	  idPattern |= ai->id;
	  found = 1;
	}
	ai++;
      } while (ai->name);
      if (!found) {
	printf("E: Bad parameter \"%s\"\n", p);
	res = S_ERROR;
	goto finally;
      }
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
    printf("E: No stdaudioin or NULL ptr\n");
    res = S_ERROR;
    goto finally;
  }

  if (!idPattern && !sampleRate && !bits && !bufSize) {
    u_int32 sampleCounter = 0, overflows = 0;
    char name[9] = "";
    if (audioFP == stdaudioin) {
      printf("stdaudioin:      0x%04x, ", audioFP);
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
      printf("\n    ->Read():    0x%04x, ", audioFP->op->Read);
      PrintTrace(trace, ENTRY_1, audioFP->op->Read);
    }
    printf("\n");
    if (ioctl(audioFP, IOCTL_AUDIO_GET_IRATE, (void *)(&sampleRate))) {
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
    if ((bufSize = ioctl(audioFP, IOCTL_AUDIO_GET_INPUT_BUFFER_SIZE, NULL)) < 0) {
      printf("Buffer size:     unknown\n");
    } else {
      printf("Buffer size:     %d 16-bit words (%d %d-bit stereo samples)\n",
	     bufSize, bufSize/(bits>>3), bits);
    }
    if ((bufFill = ioctl(audioFP, IOCTL_AUDIO_GET_INPUT_BUFFER_FILL, NULL)) < 0) {
      printf("Buffer fill:     unknown\n");
    } else {
      printf("Buffer fill:     %d 16-bit words (%d %d-bit stereo samples)\n",
	     bufFill, bufFill/(bits>>3), bits);
    }
    if (ioctl(audioFP, IOCTL_AUDIO_GET_SAMPLE_COUNTER, &sampleCounter) < 0) {
      printf("Sample counter:  unknown\n");
    } else {
      printf("Sample counter:  %lu\n", sampleCounter);
    }
    if (ioctl(audioFP, IOCTL_AUDIO_GET_OVERFLOWS, &overflows) < 0) {
      printf("Overflows:       unknown\n");
    } else {
      printf("Overflows:       %lu\n", overflows);
    }
    goto finally;
  }
  if (idPattern) {
    if (ioctl(audioFP, IOCTL_AUDIO_SELECT_INPUT, (void *)idPattern)) {
      printf("E: Input 0x%04x BAD.\n", idPattern);
      res = S_ERROR;
      goto finally;
    } else {
      if (verbose) printf("Input 0x%04x OK.\n", idPattern);
    }
  }
  if (sampleRate && bits) {
    s_int32 t32 = sampleRate * ((bits == 32) ? -1 : 1);
    if (!ioctl(audioFP, IOCTL_AUDIO_SET_RATE_AND_BITS, (void *)(&t32))) {
      if (ioctl(audioFP, IOCTL_AUDIO_GET_IRATE, (void *)(&sampleRate))) {
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
    if (ioctl(audioFP, IOCTL_AUDIO_SET_IRATE, (void *)(&sampleRate)) ||
	ioctl(audioFP, IOCTL_AUDIO_GET_IRATE, (void *)(&sampleRate))) {
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
    if (ioctl(audioFP, IOCTL_AUDIO_SET_INPUT_BUFFER_SIZE, (void *)bufSize) < 0) {
      res = S_ERROR;
      printf("E: Couldn't set buffer\n");
      goto finally;
    }
    if (verbose) printf("Buffer %d words\n", bufSize);
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
