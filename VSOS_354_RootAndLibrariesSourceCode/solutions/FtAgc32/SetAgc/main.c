#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <string.h>
#include <mutex.h>
#include <kernel.h>
#include <aucommon.h>
#include <stdlib.h>
#include <agc.h>


struct FilterEqualizer fltEqu = {0};


void PrintFilter(register FILE *fp, register s_int16 n,
		 register s_int16 maxFilters) {
  fltEqu.filterNumber = n;
  if (n < 0 || n >= maxFilters) {
    printf("%2d / %2d  RANGE ERROR\n", n+1, maxFilters);
  } else if (ioctl(fp, IOCTL_AUDIO_GET_EQU_FILTER, (void *)(&fltEqu)) == S_OK) {
    printf("%2d / %2d  0x%04x %10.2f %8.2f %9.2f\n",
	   n+1, maxFilters, fltEqu.flags, fltEqu.centerFrequencyHz,
	   fltEqu.gainDB, fltEqu.qFactor);
  } else {
    printf("%2d / %2d  IOCTL ERROR\n", n+1, maxFilters);
  }
}

struct Agc32 agc32T = {
  -32768, -32768, -32768, -32768, -32768, -32768
};
struct Agc32 agc32;

DLLENTRY(main)
ioresult main(char *parameters) {
  int nParam = RunProgram("ParamSpl", parameters);
  char *p = parameters;
  int i, fields = 0, maxFilters = 0;
  FILE *fp = stdaudioin;
  int nextSet = 0, anyValuesSet = 0;
  s_int16 dcBlockT = -32768, dcBlock = 0;
  int requireAgc = 0, isAgc = 0;

  for (i=0; i<nParam; i++) {
    if (nextSet) {
      s_int32 val = strtol(p, NULL, 0);
      switch (nextSet) {
      case 1:
	agc32T.attack = val;
	requireAgc = 1;
	break;
      case 2:
	agc32T.decay = val;
	requireAgc = 1;
	break;
      case 3:
	agc32T.targetLevel = val;
	requireAgc = 1;
	break;
      case 4:
	agc32T.maxGain = val;
	requireAgc = 1;
	break;
      case 5:
	agc32T.minGain = val;
	requireAgc = 1;
	break;
      case 6:
	dcBlockT = val;
	break;
      }
      nextSet = 0;
      anyValuesSet++;
    } else if (!strcmp(p, "-o")) {
      fp = stdaudioout;
    } else if (!strcmp(p, "-i")) {
      fp = stdaudioin;
    } else if (!strcmp(p, "-a")) {
      nextSet = 1;
    } else if (!strcmp(p, "-d")) {
      nextSet = 2;
    } else if (!strcmp(p, "-t")) {
      nextSet = 3;
    } else if (!strcmp(p, "-max")) {
      nextSet = 4;
    } else if (!strcmp(p, "-min")) {
      nextSet = 5;
    } else if (!strcmp(p, "-b")) {
      nextSet = 6;
    } else if (!strcmp(p, "-h")) {
      printf("Usage: SetAgc [-i|-o] [-a x|-d x|-t x|-max x|-min x|-d x] [-h]\n"
	     "-i\tSet stdaudioin (default)\n"
	     "-o\tSet stdaudioout\n"
	     "-a x\tSet attack (ms)\n"
	     "-d x\tSet decay (ms)\n"
	     "-t x\tSet target level (dB)\n"
	     "-max x\tSet maximum gain (dB)\n"
	     "-min x\tSet minimum gain (dB)\n"
	     "-b x\tSet DC Block Filter (0x4000 = HiFi, 0x8000 = Speech,\n"
	     "\t                     0x0    = Auto, 0xC00x = Set to x)\n"
	     "-h\tShow this help\n"
	     "\nWith no parameters SetAgc will show current values\n");
      return S_OK;
    } else {
      printf("E: Unknown parameter\n");
      return S_ERROR;
    }
    p += strlen(p)+1;
  }

  if (ioctl(fp, IOCTL_AUDIO_GET_AGC32, &agc32) != S_OK) {
    if (requireAgc) {
      printf("E: Could not find AGC driver in audio path\n");
      return S_ERROR;
    }
  } else {
    isAgc = 1;
  }

  if (dcBlock = ioctl(fp, IOCTL_AUDIO_GET_DC_BLOCK, NULL) == S_ERROR) {
    printf("E: Could not find DC_BLOCK driver in audio path\n");
    return S_ERROR;
  }

  if (!anyValuesSet) {
    if (isAgc) {
      printf("AGC:\n"
	     "  Attack      %6d ms  (1 .. 255) \n"
	     "  Decay       %6d ms  (1 .. 255) \n"
	     "  targetLevel %6d dB  (-30 .. 0)\n"
	     "  maxGain     %6d dB  (0 .. 30)\n"
	     "  minGain     %6d dB  (0 .. 30)\n"
	     "  sample rate %6ld Hz  (8000 .. 192000)\n",
	     agc32.attack,
	     agc32.decay,
	     agc32.targetLevel,
	     agc32.maxGain,
	     agc32.minGain,
	     agc32.sampleRate);
    }
    printf("DC_BLOCK:\n"
	   "  DC Block    0x%04x     "
	   "(0x0, 0x4000, 0x8000, 0xC00x, see -h for details)\n",
	   dcBlock);
  } else {
    if (agc32T.attack != -32768) {
      agc32.attack = agc32T.attack;
    }
    if (agc32T.decay != -32768) {
      agc32.decay = agc32T.decay;
    }
    if (agc32T.targetLevel != -32768) {
      agc32.targetLevel = agc32T.targetLevel;
    }
    if (agc32T.maxGain != -32768) {
      agc32.maxGain = agc32T.maxGain;
    }
    if (agc32T.minGain != -32768) {
      agc32.minGain = agc32T.minGain;
    }
    if (agc32T.sampleRate != -32768) {
      agc32.sampleRate = agc32T.sampleRate;
    }
    if (dcBlockT != -32768) {
      if (ioctl(fp, IOCTL_AUDIO_SET_DC_BLOCK, (void *)dcBlockT) != S_OK) {
	printf("E: Couldn't set DC_BLOCK\n");
	return S_ERROR;
      }
    }
    if (requireAgc && ioctl(fp, IOCTL_AUDIO_SET_AGC32, &agc32) != S_OK) {
      printf("E: Couldn't set AGC\n");
      return S_ERROR;
    }
  }

  return S_OK;
}
