/*

  DiskFree - returns number of free disk blocks.

*/
#include <vo_stdio.h>
#include <volink.h>     /* Linker directives like DLLENTRY */
#include <apploader.h>  /* RunLibraryFunction etc */
#include <timers.h>
#include <libaudiodec.h>
#include <vsostasks.h>
#include <consolestate.h>
#include <ctype.h>
#include <uimessages.h>
#include <string.h>
#include <stdlib.h>
#include <audio.h>
#include <aucommon.h>
#include <limits.h>
#include <sysmemory.h>
#include <kernel.h>
#include <vo_fat.h>
#include <crc32.h>


int verbose = 0;

u_int32 VoFatCountFreeKiB(register VO_FILE *f, register u_int32 maxMiB,
			  register u_int16 verbose);


ioresult PrintClusters(register const char *dirName, register u_int32 *wPointer, register u_int32 maxMiB) {
  FILE *fp;
  u_int32 resKiB = 0xFFFFFFFFU;
  ioresult res = S_ERROR;
  static int titleLine = 0;

  if (strlen(dirName) < 2 || dirName[1] != ':') {
    if (!wPointer) {
      printf("E: Illegal file name %s\n", dirName);
    }
    goto finally;
  }

  fp = fopen(dirName, "s");
  if (fp) {
    if (!__F_CHARACTER_DEVICE(fp)) {
      resKiB = VoFatCountFreeKiB(fp, maxMiB, verbose > 1 && !wPointer);

      if ((!titleLine && verbose) || verbose > 1) {
	titleLine = 1;
	printf("Drv  Used/KiB   Free/KiB  Total/KiB  Use%%  Name\n");
      }

      if (!wPointer) {
	printf("%c: ", toupper(dirName[0]));
	if (!verbose) {
	  printf("%10ld KiB free%s, %s\n", resKiB,
		 (maxMiB != 0xFFFFFFFFU) ? " (or more)" : "",
		 fp->dev->Identify(fp->dev, NULL, 0));
	} else {
	  FatDeviceInfo *di=(FatDeviceInfo*)fp->dev->deviceInfo;
	  // di->totalClusters includes non-existent clusters 0 and 1
	  u_int32 totalClusters = di->totalClusters-2;
	  u_int16 sectorsPerCluster = di->fatSectorsPerCluster;
	  u_int32 totalKiB = totalClusters * di->fatSectorsPerCluster / 2;
	  if (maxMiB >= ULONG_MAX/1024 || maxMiB*1024 >= totalKiB) {
	    printf("%10ld %10ld %10ld  %3d%%  %s\n",
		   totalKiB-resKiB,
		   resKiB,
		   totalKiB,
		   (int)(100.5-100.0*resKiB/totalKiB),
		   fp->dev->Identify(fp->dev, NULL, 0));
	  } else {
	    printf("%10ld %10ld %10ld    -%%  %s\n",
		   maxMiB*1024L-resKiB,
		   resKiB,
		   totalKiB,
		   fp->dev->Identify(fp->dev, NULL, 0));
	  }
	}
      }
      fclose(fp);
    }
  } else {
    if (!wPointer) {
      printf("E: Illegal device %c:\n", toupper(dirName[0]));
      goto finally;
    }
  }
  res = S_OK;

 finally:
  if (wPointer) {
    *wPointer = resKiB;
  }
  return res;
}

ioresult main(char *parameters) {
  int nParam, i;
  FILE *fp = NULL;
  char *p = parameters;
  int nextIsWPointer = 0, nextIsMaxMiB = 0, noOp = 1;
  ioresult retCode = S_OK;
  u_int32 *wPointer = NULL, maxMiB = 0xFFFFFFFFU;

  nParam = RunProgram("ParamSpl", parameters);
  for (i=0; i<nParam; i++) {
    if (nextIsWPointer) {
      wPointer = (void *)(strtol(p, NULL, 0));
      nextIsWPointer = 0;
    } else if (nextIsMaxMiB) {
      maxMiB = strtol(p, NULL, 0);
      nextIsMaxMiB = 0;
    } else if (!strcmp(p, "-h")) {
      printf("Usage: DiskFree [-p x|-m x|-v|-h] [D:]\n"
	     "D:\tPrint results for drive D (:: means all drives)\n"
	     "\tThere may be multiple drive parameters.\n"
	     "\tDrive parameters must be after -p/-m options.\n"
	     "-p x\tStore last result in KiB to X-memory 32-bit pointer"
	     " pointed to by x\n"
	     "\t(Usable for calling with RunLibraryFunction(); "
	     "makes operation silent)\n"
	     "-m x\tLook only until x MiB (faster operation)\n"
	     "-v\tVerbose (giving twice makes even more verbose)\n"
	     "-h\tShow this help\n");
      goto finally;
    } else if (!strcmp(p, "-p")) {
      nextIsWPointer = 1;
    } else if (!strcmp(p, "-m")) {
      nextIsMaxMiB = 1;
    } else if (!strcmp(p, "-v")) {
      verbose++;
    } else if (p[0] && p[1] == ':') {
      if (p[0] == ':') {
	int ii;
	for (ii='A'; ii<='Z'; ii++) {
	  if (vo_pdevices[ii-'A']) {
	    p[0] = ii;
	    retCode += PrintClusters(p, wPointer, maxMiB);
	  }
	}
      } else {
	retCode += PrintClusters(p, wPointer, maxMiB);
      }
      noOp = 0;
    } else {
      printf("E: Unknown parameter \"%s\"\n", p);
      retCode = S_ERROR;
      goto finally;
    }
    p += strlen(p)+1;
  }

  if (noOp) {
    retCode += PrintClusters(currentDirectory, wPointer, maxMiB);
  }
  
 finally:
  return retCode;
}
