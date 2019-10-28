/* For free support for VSIDE, please visit www.vsdsp-forum.com */

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <consolestate.h>
#include <vo_fat.h>
#include <string.h>
#include <swap.h>
#include <sysmemory.h>
#include <strings.h>
#include <kernel.h>
#include "audioinfo.h"


#define NAMELENGTH 255
#define SORT_ELEMENTS 1024

#define LF_SORT_BY_NAME	 1
#define LF_SORT_BY_TIME	 2
#define LF_SORT_BY_SIZE	 4
#define LF_SORT_REVERSE  8
#define LF_AUDIO_ONLY	16
#define LF_FAST		32 /* Only relevant when LF_AUDIO_ONLY selected */
#define LF_SHOWDATE	64
#define LF_VERBOSE	128
#define LF_SORT_MASK (LF_SORT_BY_NAME|LF_SORT_BY_TIME|LF_SORT_BY_SIZE)

struct SortDirEntry {
  int idx;
  int attr;
  u_int32 size;
  u_int16 date;
  u_int16 time;
  u_int32 secondsX10;
  u_int32 channelsAndRateX8;
  u_int32 firstCluster;
  unsigned char shortName[13];
  unsigned char longName[1];
};
struct SortDirEntry *sortDirEntry[SORT_ELEMENTS];

u_int32 FoundFileSize(VO_FILE *hSearch) {
  DirectoryEntry *de = &hSearch->sectorBuffer[((hSearch->pos-32) >> 1) & 0xff];
  return Swap32Mix(*(u_int32*)(&de->fileSizeLo));
}

u_int32 FoundFileTimeDate(VO_FILE *hSearch) {
  DirectoryEntry *de = &hSearch->sectorBuffer[((hSearch->pos-32) >> 1) & 0xff];
  return Swap32Mix(*(u_int32*)(&de->writeTime));
}

u_int32 FoundFileFirstCluster(VO_FILE *hSearch) {
  DirectoryEntry *de = &hSearch->sectorBuffer[((hSearch->pos-32) >> 1) & 0xff];
  return Swap16(*(u_int16*)(&de->firstClusLo)) |
    ((u_int32)Swap16(*(u_int16*)(&de->firstClusHi)) << 16);
}


void FreeAllEntries(void) {
  struct SortDirEntry **sde = sortDirEntry;
  while (*sde) {
    free(*sde);
    *sde++ = NULL;
  }
}

int sortFlags = 0;
int SortDirEntryCmp(const void *p1, const void *p2) {
  struct SortDirEntry *sde1 = *((struct SortDirEntry **)p1);
  struct SortDirEntry *sde2 = *((struct SortDirEntry **)p2);
  int res = 0;

  if (sortFlags & LF_SORT_BY_TIME) {
    if (sde1->date == sde2->date) {
      if (sde1->time > sde2->time) {
	res = 1;
      } else if (sde1->time < sde2->time) {
	res = -1;
      }
    } else {
      res = sde1->date-sde2->date;
    }
  } else if (sortFlags & LF_SORT_BY_SIZE) {
    if (sde1->size > sde2->size) {
      res = 1;
    } else if (sde1->size < sde2->size) {
      res = -1;
    }
  } else {
    res = strcasecmp(sde1->longName, sde2->longName);
  }
  return (sortFlags & LF_SORT_REVERSE) ? -res : res;
}

char GetFileChar(int attr) {
  if (attr & 0x08) {
    /* Label */
    return 'L';
  } else if (attr & 0x10) {
    /* Directory */
    return 'D';
  }
  /* File */
  return '-';
}

struct AudioInfo audioInfo;

void PrintEntry(register FILE *fp, register struct SortDirEntry *sde, register u_int16 flags) {
  if (flags & LF_AUDIO_ONLY) {
    printf("%c %4d. %-12s ",
	   GetFileChar(sde->attr), sde->idx, sde->shortName);
    if (sde->secondsX10 == 0xFFFFFFFFU) {
      printf("  -0:00.0");
    } else {
      int minutes = (int)(sde->secondsX10/600);
      double seconds = (sde->secondsX10-(u_int32)minutes*600)*0.1;
      printf("%4d:%04.1f",
	     minutes, seconds);
    }
    printf(" %6ld %1d",
	   sde->channelsAndRateX8 >> 3,
	   (int)sde->channelsAndRateX8 & 7);
  } else {
    printf("%c %4d. %-12s %10lu",
	   GetFileChar(sde->attr), sde->idx, sde->shortName, sde->size);
  }
  if (flags & LF_SHOWDATE) {
    u_int16 Y,M,D,h,m,s,t;
    t = sde->date;
    D = t & 31;
    t >>= 5;
    M = t & 15;
    Y = (1980+(t>>4));
    t = sde->time;
    s = (t & 31) * 2;
    t >>= 5;
    m = t & 63;
    h = t>>6;
    printf(" %02d-%02d-%02d %02d:%02d:%02d",
	   Y, M, D, h, m, s);
  }
  printf(" %s\n", sde->longName);
  if (flags & LF_VERBOSE)  {
    FatDeviceInfo *di = (FatDeviceInfo *)(fp->dev->deviceInfo);
    u_int32 totalClusters = 0;
    u_int32 bytesPerCluster = 512*di->fatSectorsPerCluster;
    u_int32 shouldBeTotalClusters =
      (sde->size + bytesPerCluster-1) / bytesPerCluster;
    int isDir = GetFileChar(sde->attr) == 'D';
#if 0
    printf("  firstCluster = 0x%08lx 2nd = %08lx, clusters %lx\n",
	   sde->firstCluster,
	   VoFatReadClusterRecord(fp, sde->firstCluster),
	   shouldBeTotalClusters);
#endif
    if (isDir || shouldBeTotalClusters) {
      u_int32 firstClusterInThisGroup = sde->firstCluster ? sde->firstCluster:2;
      u_int32 lastClusterInThisGroup = firstClusterInThisGroup;
      printf("        First block 0x%lx, ", firstClusterInThisGroup*di->fatSectorsPerCluster+di->dataStart);
      //      printf("        First block %lx", di->dataStart);
      printf(" Cluster list: 0x%lx", firstClusterInThisGroup);
      while (lastClusterInThisGroup < 0x0ffffff8) {
	u_int32 new = VoFatReadClusterRecord(fp, lastClusterInThisGroup);
	totalClusters++;
	if (new != ++lastClusterInThisGroup) {
	  if (--lastClusterInThisGroup != firstClusterInThisGroup) {
	    printf("-0x%lx", lastClusterInThisGroup);
	  }
	  firstClusterInThisGroup = lastClusterInThisGroup = new;
	  if (new < 0x0ffffff8) {
	    printf(", 0x%lx", new);
	  }
	}
      }
      printf("\n        Total cluster chain 0x%lx, file size 0x%lx, %s\n",
	     totalClusters, shouldBeTotalClusters,
	     (!isDir && totalClusters != shouldBeTotalClusters)?"*ERROR*":"OK");
    }
  }
}


s_int16 ListFiles(const char *path, int flags, int __mem_y *idx) {
  s_int16 n=0, nSort;
  static char filename[NAMELENGTH];
  int fileIndicesLeft = *idx; //Only valid if idx is non-NULL
  FILE *f = fopen(path, "s"); //Get search handle for the disk

  if (f) {
    s_int16 fileIdx;
    path += 2; //Remove drive letter and colon from path
  doItAgain:
    n = 0;
    fileIdx = 0;
    nSort = 0;
    if (FatFindFirst(f, path, filename, NAMELENGTH) == S_OK) {
#if 0
      printf("Entries in %s \"%s\"\n",
	     f->dev->Identify(f->dev,NULL,0), path-2);
#endif
      do {
	if (!(f->ungetc_buffer & __ATTR_VOLUMEID)) {
	  fileIdx++;
	  if (!(flags & LF_AUDIO_ONLY) ||
	      !GetAudioInfo(filename, &audioInfo, NULL,
			    (flags & LF_FAST) ? GAIF_FAST : 0)) {
	    struct SortDirEntry *sde =
	      calloc(1, sizeof(struct SortDirEntry) + strlen(filename));
	    n++;
	    if (!sde || nSort >= SORT_ELEMENTS-1) {
	      if (flags & LF_SORT_MASK) {
		FreeAllEntries();
		flags &= ~LF_SORT_MASK;
		goto doItAgain;
	      } else {
		printf("Out of memory\n");
		goto outOfMemory;
	      }
	    }

	    sde->idx = fileIdx;
	    sde->attr = f->ungetc_buffer;
	    sde->size = FoundFileSize(f);
	    sde->firstCluster = FoundFileFirstCluster(f);
	    if (audioInfo.seconds < 0.0) {
	      sde->secondsX10 = 0xFFFFFFFFU;
	    } else {
	      sde->secondsX10 = audioInfo.seconds*10.0+0.5;
	    }
	    sde->channelsAndRateX8 = audioInfo.channels+audioInfo.sampleRate*8;
	    {
	      u_int32 t = FoundFileTimeDate(f);
	      sde->time = (u_int16)t;
	      sde->date = (u_int16)(t>>16);
	    }
	    strcpy(sde->shortName, f->extraInfo);
	    strcpy(sde->longName, filename);

	    if (flags & LF_SORT_MASK) {
	      sortDirEntry[nSort++] = sde;
	    } else {
	      if (idx) {
		if (fileIndicesLeft > 0) {
		  fileIndicesLeft--;
		  *idx++ = fileIdx;
		}
	      } else {
		PrintEntry(f, sde, flags);
	      }
	    }
	    if (!nSort) {
	      free(sde);
	    }
	  }
	} /* !__ATTR_VOLUMEID */
	if (appFlags & APP_FLAG_QUIT) goto outOfMemory;
      } while (S_OK == FatFindNext(f,filename,255));
    } else {
      printf("Path not found\n");
    }
  outOfMemory:
    if ((flags & LF_SORT_MASK) && !(appFlags & APP_FLAG_QUIT)) {
      int i;
      sortFlags = flags;
      qsort(sortDirEntry, nSort, sizeof(sortDirEntry[0]), SortDirEntryCmp);
      for (i=0; i<nSort; i++) {
	struct SortDirEntry *sde = sortDirEntry[i];

	if (idx) {
	  if (fileIndicesLeft > 0) {
	    fileIndicesLeft--;
	    *idx++ = sde->idx;
	  }
	} else {
	  PrintEntry(f, sde, flags);
	}
      }
      FreeAllEntries();
    }
    fclose(f);
    f = NULL;
  }

  return (appFlags & APP_FLAG_QUIT) ? -1 : n;
}


ioresult main(char *parameters) {
  int nParam, nDirParam = 0, listFlags = LF_SORT_BY_NAME|LF_FAST|LF_SHOWDATE, i;
  int retCode = S_ERROR;

  nParam = RunProgram("ParamSpl", parameters);

  for (i=0; i<nParam; i++) {
    if (!strcmp(parameters, "-h")) {
      printf("Usage: dir [-s|-sn|-st|-ss|+s|-r|+r|-d|+d|-a|+a|-f|+f|-v|+v|-h] [path]\n"
	     "-s|-sn\tSort files by name\n"
	     "-st\tSort files by date\n"
	     "-ss\tSort files by size\n"
	     "+s\tDon't sort files (faster with large directories)\n"
	     "-r\tReverse sort (if sort selected)\n"
	     "+r\tForwards sort (if sort selected)\n"
	     "-d\tShow file date\n"
	     "+d\tDon't show file date\n"
	     "-a\tOnly audio files\n"
	     "+a\tAll files\n"
	     "-f\tFast listing (don't show play time for MP3 files with -a)\n"
	     "+f\tSlower listing (show play time also for MP3 files with -a)\n"
	     "-v\tVerbose on\n"
	     "+v\tVerbose off\n"
	     "-h\tShow this help page\n");
      goto finally;
    } else if (!strcmp(parameters, "-a")) {
      listFlags |= LF_AUDIO_ONLY;
    } else if (!strcmp(parameters, "+a")) {
      listFlags &= ~LF_AUDIO_ONLY;
    } else if (!strcmp(parameters, "-s") || !strcmp(parameters, "-sn")) {
      listFlags &= ~LF_SORT_MASK;
      listFlags |= LF_SORT_BY_NAME;
    } else if (!strcmp(parameters, "-st")) {
      listFlags &= ~LF_SORT_MASK;
      listFlags |= LF_SORT_BY_TIME;
    } else if (!strcmp(parameters, "-ss")) {
      listFlags &= ~LF_SORT_MASK;
      listFlags |= LF_SORT_BY_SIZE;
    } else if (!strcmp(parameters, "+s")) {
      listFlags &= ~LF_SORT_MASK;
    } else if (!strcmp(parameters, "-r")) {
      listFlags |= LF_SORT_REVERSE;
    } else if (!strcmp(parameters, "+r")) {
      listFlags &= ~LF_SORT_REVERSE;
    } else if (!strcmp(parameters, "-d")) {
      listFlags |= LF_SHOWDATE;
    } else if (!strcmp(parameters, "+d")) {
      listFlags &= ~LF_SHOWDATE;
    } else if (!strcmp(parameters, "-f")) {
      listFlags |= LF_FAST;
    } else if (!strcmp(parameters, "+f")) {
      listFlags &= ~LF_FAST;
    } else if (!strcmp(parameters, "-v")) {
      listFlags |= LF_VERBOSE;
    } else if (!strcmp(parameters, "+v")) {
      listFlags &= ~LF_VERBOSE;
    } else {
      char *dp = NULL;
      int parLen = strlen(parameters);
      int totLen = parLen+strlen(currentDirectory);
      if (parameters[1] != ':' && (dp = malloc(totLen+2))) {
	char *ep;
	strcpy(dp, currentDirectory);
	strcat(dp, parameters);
	ep = dp+totLen-1;
	if (*ep != '/' && *ep != ':') {
	  ep[1] = '/';
	  ep[2] = '\0';
	}
      } else if (parLen > 2 && parameters[parLen-1] != '/' && parameters[parLen-1] != '\\' && (dp = malloc(parLen+2))) {
	sprintf(dp, "%s%c", parameters, '/');
      }
      retCode = (ListFiles(dp ? dp : parameters, listFlags, NULL) < 0) ?
	S_ERROR : S_OK;
      if (dp) {
	free(dp);
	dp = NULL;
      }
      if (retCode != S_OK) {
	goto finally;
      }
      nDirParam++;
    }
    parameters += strlen(parameters)+1;
  }

  if (!nDirParam) {
    retCode = (ListFiles(currentDirectory, listFlags, NULL) < 0) ?
      S_ERROR : S_OK;
  }

 finally:
  return retCode;
}



DLLENTRY(MakePlayList)
ioresult MakePlayList(u_int16 __mem_y *idx) {
  return ListFiles(currentDirectory, LF_SORT_BY_NAME|LF_FAST|LF_AUDIO_ONLY, idx);
}
