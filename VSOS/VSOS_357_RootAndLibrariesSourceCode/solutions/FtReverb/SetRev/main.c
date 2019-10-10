#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <string.h>
#include <mutex.h>
#include <kernel.h>
#include <aucommon.h>
#include <stdlib.h>
#include <audio.h>

/* These defines are missing from VSOS releases prior to 3.55 */
#ifndef IOCTL_AUDIO_GET_REV_ROOM
#define IOCTL_AUDIO_GET_REV_ROOM	280
#define IOCTL_AUDIO_SET_REV_ROOM	281

struct Room {
  /* These fields filled in by the caller of DesignRoom() */
  u_int16 firstReflectionMS; /* How many milliseconds before first reflection */
  u_int16 roomSizeCM;	/* Room size in cm */
  u_int16 reverbTimeMS;	/* Room reverb time in ms = -60 dB attenuation time */
  u_int16 softness;	/* Room softness, 0 = hard, 65535 = soft */
  u_int32 sampleRate;	/* sample rate used for making room design */
  s_int16 dryGain;	/* 1024 = 1 */
  s_int16 wetGain;	/* 1024 = 1 */
  s_int16 unused1;	/* To make partially compatible with Room23 */
  s_int16 unused2;	/* To make partially compatible with Room23 */
  u_int32 unused3;	/* To make partially compatible with Room23 */
  /* Rest of the fields filled in by the room designer */
  s_int16 delays;	/* How many delays per channel used */
};

#endif /* !IOCTL_AUDIO_GET_REV_ROOM */

struct Room myRoom;

char __mem_y helpText[] =
  "Usage: SetRev [-i|-o] [-rx] [-sx] [-tx] [-fx] [-dx] [-wx] [-v|+v] [-h]\n"
  "-i\tSet stdaudioin\n"
  "-o\tSet stdaudioout (default)\n"
  "-rx\tSet first reflection time in milliseconds\n"
  "-sx\tSet room size to x cm (200-1200 recommended)\n"
  "-tx\tSet reverb Time to x ms (100-5000 recommended)\n"
  "-fx\tSet room wall soFtness (0 = hard, 65535 = soft)\n"
  "-dx\tSet Dry gain (0-65535, 1024 = 1)\n"
  "-wx\tSet Wet gain (0-65535, 1024 = 1)\n"
  "-v|+v\tVerbose on/off\n"
  "-h\tShow this help\n"
  "\nNote:\n"
  "It is recommended that Dry gain + Wet gain would not be much\n"
  "over 1024 to avoid distortion.\n";


DLLENTRY(main)
ioresult main(char *parameters) {
  int nParam = RunProgram("ParamSpl", parameters);
  char *p = parameters;
  int i, fields = 0;
  FILE *fp = stdaudioout;
  double speed = -1.0, pitch = -1.0;
  int nothingToDo = 1, verbose = 0;
  ioresult errCode = S_ERROR;

  if (ioctl(fp, IOCTL_AUDIO_GET_REV_ROOM, (void *)(&myRoom)) < 0) {
    printf("E: No reverb library found\n");
    goto finally;
  }

  for (i=0; i<nParam; i++) {
    if (!strcmp(p, "-o")) {
      fp = stdaudioout;
    } else if (!strcmp(p, "-i")) {
      fp = stdaudioin;
    } else if (!strncmp(p, "-r", 2)) {
      myRoom.firstReflectionMS = (u_int16)(atof(p+2));
      nothingToDo = 0;
    } else if (!strncmp(p, "-s", 2)) {
      myRoom.roomSizeCM = (u_int16)(atof(p+2));
      nothingToDo = 0;
    } else if (!strncmp(p, "-t", 2)) {
      myRoom.reverbTimeMS = (u_int16)(atof(p+2));
      nothingToDo = 0;
    } else if (!strncmp(p, "-f", 2)) {
      myRoom.softness = (u_int16)(atof(p+2));
      nothingToDo = 0;
    } else if (!strncmp(p, "-d", 2)) {
      myRoom.dryGain = (u_int16)(atof(p+2));
      nothingToDo = 0;
    } else if (!strncmp(p, "-w", 2)) {
      myRoom.wetGain = (u_int16)(atof(p+2));
      nothingToDo = 0;
    } else if (!strcmp(p, "-v")) {
      verbose = 1;
    } else if (!strcmp(p, "+v")) {
      verbose = 0;
    } else if (!strcmp(p, "-h")) {
      /* Help string moved to Y memory, because FTOREV23.DLL eats up
	 X memory as well as it can, so there may not be enough space
	 for a long message in X memory. */
      char __mem_y *c = helpText;
      while (*c) {
	fputc(*c++, stdout);
      }
      return S_OK;
    } else {
      printf("E: Unknown parameter \"%s\"\n", p);
    }
    p += strlen(p)+1;
  }

  if (!nothingToDo) {
    if (ioctl(fp, IOCTL_AUDIO_SET_REV_ROOM, (void *)(&myRoom)) < 0) {
      printf("E: Setting parameters failed\n");
      goto finally;
    }
  }

  if (nothingToDo || verbose) {
    ioctl(fp, IOCTL_AUDIO_GET_REV_ROOM, (void *)(&myRoom));
    printf("ROOM:\n");
    printf("  (-r) First reflection: %6u ms\n",
	   myRoom.firstReflectionMS);
    printf("  (-s) Room size       : %6u cm (1-32767)\n",
	   myRoom.roomSizeCM);
    printf("  (-t) Reverb time     : %6u ms (1-65535)\n",
	   myRoom.reverbTimeMS);
    printf("  (-f) Room softness   : %6u    (0=hard-65535=soft)\n",
	   myRoom.softness);
    printf("  (-d) Dry gain        : %6d    (0-32767, 1024=1)\n",
	   myRoom.dryGain);
    printf("  (-w) Wet gain        : %6d    (0-32767, 1024=1)\n",
	   myRoom.wetGain);
    printf("       Sample rate     : %6ld Hz\n",
	   myRoom.sampleRate);
    printf("       Delay pairs     : %6d\n",
	   myRoom.delays);
    printf("       Ext. mem. size  : %6ld bytes\n", myRoom.memSizeWords*2);
    printf("       Ext. mem. read  : 0x%04x ", myRoom.Read);
    if (myRoom.Read) RunLibraryFunction("TRACE", ENTRY_1, (s_int16)myRoom.Read);
    printf("\n       Ext. mem. write : 0x%04x ", myRoom.Write);
    if (myRoom.Write) RunLibraryFunction("TRACE", ENTRY_1, (s_int16)myRoom.Write);
    printf("\n");
  }

  errCode = S_OK;
 finally:
  return errCode;
}
