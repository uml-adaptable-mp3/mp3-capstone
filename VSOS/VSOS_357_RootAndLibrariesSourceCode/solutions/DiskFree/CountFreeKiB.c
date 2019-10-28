/// \file vo_fat.c FAT16/FAT32 Filesystem driver for VsOS
/// \author Panu-Kristian Poiksalo and Pasi Ojala, VLSI Solution Oy



#include <vo_stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "vsos.h"
#include "vo_fat.h"
//#include "forbid_stdout.h"
#include "strings.h"
#include <sysmemory.h>
#include <crc32.h>

#if 1
#define OPTIMIZE_SPEED
#endif

#define MIN(a,b) (((a)<(b))?(a):(b))

#define LINES 16
#define CHARS_PER_LINE 64
#define TOT_CHARS (LINES*CHARS_PER_LINE)

u_int32 VoFatCountFreeKiB(register VO_FILE *f, register u_int32 maxMiB,
			  register u_int16 verbose) {
  //  FatFileInfo *fi = (FatFileInfo*)f->fileInfo;
  FatDeviceInfo *di = (FatDeviceInfo*)f->dev->deviceInfo;
  u_int32 res = 0, k;
  // totalClusters includes non-existent clusters 0 and 1
  u_int32 end = di->totalClusters;
  u_int32 maxClusters = (maxMiB == 0xFFFFFFFFU) ?
    0xFFFFFFFFU : maxMiB*1024/di->fatSectorsPerCluster*2+2;
  u_int32 reportInterval;
  u_int32 nextReport = reportInterval+2;
  u_int16 charsOnThisLine = 0;
  u_int32 freeForThisReport = 0;

  end = MIN(end, maxClusters);

  reportInterval = nextReport = (end+(TOT_CHARS-1))/TOT_CHARS;
  if (!reportInterval) reportInterval = 1;
  //  printf("ReportInterval %ld\n", reportInterval);
  if (verbose) {
    if (reportInterval == 1) {
      printf("  "); charsOnThisLine = 2; nextReport = 3; freeForThisReport = 0;
    } else if (reportInterval == 2) {
      printf(" ");  charsOnThisLine = 1; nextReport = 4; freeForThisReport = 0;
    }
  }

  for (k=2; k<end; k++) {
    if (verbose && k>=nextReport) {
      int outCh = '#';
      if (freeForThisReport == reportInterval) outCh = '.';
      else if (freeForThisReport) outCh = '+';
      printf("%c", outCh);
      if (++charsOnThisLine >= CHARS_PER_LINE) {
	printf("\n");
	charsOnThisLine = 0;
      }
      nextReport += reportInterval;
      freeForThisReport = 0;
    }
    if (!VoFatReadClusterRecord(f, k)) {
      res++;
      freeForThisReport++;
    }
#ifdef OPTIMIZE_SPEED
    if (di->fatBits == 32 && !(k & 127) && end-k >= 128) {
      u_int32 *sB = (u_int32 *)(&f->sectorBuffer[2]);
      int i;
      if (!verbose) {
	for (i=1; i<128; i++) {
	  if (!(*sB++)) {
	    res++;
	  }
	}
      } else {
	for (i=1; i<128; i++) {
	  if (verbose && k+i>=nextReport) {
	    int outCh = '#';
	    if (freeForThisReport == reportInterval) outCh = '.';
	    else if (freeForThisReport) outCh = '+';
	    printf("%c", outCh);
	    if (++charsOnThisLine >= CHARS_PER_LINE) {
	      printf("\n");
	      charsOnThisLine = 0;
	    }
	    nextReport += reportInterval;
	    freeForThisReport = 0;
	  }

	  if (!(*sB++)) {
	    res++;
	    freeForThisReport++;
	  }
	}
      }
      k += 127;
    }
#endif
  }
  if (charsOnThisLine) {
    printf("\n");
  }

  if (di->fatSectorsPerCluster == 1) {
    return res / 2;
  }
  return res * (di->fatSectorsPerCluster / 2);
}
