#include <vo_stdio.h>	// MUST be first include!!!
#include <string.h>
#include <vo_fat.h>
#include <vsos.h>
#include <codecmpg.h>
#include <codec.h>
#include <timers.h>
#include <exec.h>
#include <swap.h>
#include <ctype.h>
#include <sysmemory.h>
#include "stdAudio.h"
#include "uimessages.h"
#include "strings.h"
#include "vo_fatdirops.h"

//void Delay(u_int16);






#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN    0x02
#define ATTR_SYSTEM    0x04
#define ATTR_VOLUME_ID 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE   0x20
#define ATTR_LONG_NAME (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)

const u_int16 __mem_y fatEntryIdx[13] = {
  1, 3, 5, 7, 9, 14, 16, 18, 20, 22, 24, 28, 30
};

/* name must be at least 16 characters long */
enum EntryType CheckEntryType(FILE *fp, unsigned char *name) {
  u_int16 firstByte, flagByte;
  
#if 0
  printf("    CheckEntryType(%p, %p), pos %ld\n",
	 fp, name, fp->pos);
  Delay(200);
#endif

  fp->pos -= 32;
  if (fp->op->Read(fp, name, 0, 32) != 32) {
    return etEnd;
  }



  firstByte = name[0] >> 8;
  flagByte = name[5] & 0xFF;
  if (firstByte == 0xe5) {
    return etDeleted;
  } else if (firstByte == 0x00) {
    return etEnd;
  } else if (flagByte == ATTR_LONG_NAME) {
    int ready = 0, i;

    /* Convert long name into short */
    for (i=0; i<13; i++) {
      u_int16 idx, ch;
      idx = fatEntryIdx[i];
      ch = Swap16(name[idx>>1]);
      if (idx & 1) {
	ch = (ch >> 8) |
	  (name[(idx>>1)+1] & 0xFF00);
      }
      name[i] = ch;
      if (!ch)
	ready = 1;
    }
    name[13] = '\0';

#if 0
    printf("    LONG -> \"%s\"\n", name);
#endif

    if (firstByte & 0x40)
      return etLongNameLast;
    else
      return etLongName;
  } else {
    int i;
    u_int16 ch;
    char *d = name, *s = name+7;

    if (firstByte == 0x05)
      name[0] |= 0xE000U;	// Special FAT char[0] 0x05 -> 0xe5 conversion

    memmove(s, d, 6);
    for (i=0; i<11; i++) {
      if (i == 8)
	*d++ = '.';
      if (!(i&1)) {
	ch = *s >> 8;
      } else {
	ch = *s++ & 0xFF;
      }
      if (ch > 0x20)
	*d++ = ch;
    }
    if (d != name && d[-1] == '.') {
      d--;
    }
    *d = '\0';

#if 0
    printf("    NAME -> \"%s\"\n", name);
#endif

    if (flagByte == ATTR_DIRECTORY)
      return etDir;
    if (flagByte & ATTR_HIDDEN)
      return -1;
    else
      return etFile;
  }
}


auto s_int16 ResetFindFile(FILE *fp) {
  struct FatDirExt *fde;
  if (!fp)
    return -1;
  fde = fp->extraInfo;
  if (!fde)
    return -1;
  fp->pos = 0;
  fde->lastGoodEntry = 0;
  return 0;
}

auto s_int16 FindFile(FILE *fp, char *name, s_int16 nameSize,
		 const char *suffixes,
		 enum OpenFileMode openFileMode, enum EntryType entryType) {
  int ready = 0;
  int addFileNumber = 1;
  char *searchName = NULL;
  int numOfFiles = 1;
  u_int16 lastGoodEntry;
  volatile s_int32 lastGoodEntryPos; // volatile to prevent lcc bug
  struct FatDirExt *fde;

  if (!fp || !name || nameSize < 16) {
    return -2;
  }
  
  fde = fp->extraInfo;
  lastGoodEntry = fde->lastGoodEntry;
  lastGoodEntryPos = fp->pos;

  if (!lastGoodEntry && openFileMode != ofmFirst) {
    //printf("#0: Make it find first first!\n");
    u_int16 t;
    if ((t = FindFile(fp, name, nameSize, suffixes, ofmFirst, entryType)) < 0)
      return -3;
    if (openFileMode == ofmNext)
      return t;
    lastGoodEntry = fde->lastGoodEntry;
    lastGoodEntryPos = fp->pos;
    //    printf("#0 finished: %d %ld\n", lastGoodEntry, lastGoodEntryPos);
  }
#if 0
  printf("  FF: ungetc %d, pos %ld, cmd %d, suf %s\n",
	 fp->ungetc_buffer, fp->pos, openFileMode, suffixes);
#endif

  if (openFileMode > 0) {
    //    printf("currFile %d, openFileMode %d\n", currFile, openFileMode);
    if (openFileMode == 1) {
      openFileMode = ofmFirst;
    } else if (openFileMode < lastGoodEntry) {
      numOfFiles = lastGoodEntry - openFileMode;
      //printf("** Prev %d\n", numOfFiles);
      openFileMode = ofmPrev;
    } else if (openFileMode == lastGoodEntry) {
      openFileMode = ofmSame;
      //printf("** Same\n");
    } else {
      numOfFiles = openFileMode - lastGoodEntry;
      //printf("** Next %d\n", numOfFiles);
      openFileMode = ofmNext;
    }
  }

  if (openFileMode == ofmSame) {
    if (lastGoodEntry & 0x8000U) {
      return -1;
    }
    if (!fde->lastGoodEntry) {
      openFileMode = ofmNext;
      addFileNumber = 1;
    } else {
      addFileNumber = 0;
    }
  } else if (openFileMode == ofmPrev) {
    if (lastGoodEntry & 0x8000U) {
      openFileMode = ofmSame;
      addFileNumber = 0;
    } else {
      addFileNumber = -1;
    }
  } else if (openFileMode == ofmFirst) {
    lastGoodEntry = fde->lastGoodEntry = 0;
    lastGoodEntryPos = fp->pos = 0;
    openFileMode = ofmNext;
  } else if (openFileMode == ofmLast) {
    while (FindFile(fp, name, nameSize, suffixes, ofmNext, entryType) >= 0)
      ;
    lastGoodEntry = fde->lastGoodEntry;
    lastGoodEntryPos = fp->pos;
    openFileMode = ofmSame;
    addFileNumber = 0;
  } else if (openFileMode == ofmName) {
    int nameLenPlus1 = strlen(name)+1;
    fde->lastGoodEntry = 0;
    fp->pos = 0;
    openFileMode = ofmNext;
    searchName = name;
    nameSize -= nameLenPlus1;
    name += nameLenPlus1;
#if 0
    printf("searchName %p %s, name %p %s\n",
	   searchName, searchName, name, name);
#endif
  }
  name[0] = '\0';

  do {
#if 0
    printf("#1: lastGood %x %ld, ofm %d, pos %ld -> ",
	   lastGoodEntry, lastGoodEntryPos, openFileMode, fp->pos);
#endif
    if (openFileMode == ofmPrev) {
      if (fp->pos < 64) {
	lastGoodEntry = 0;
	lastGoodEntryPos = 0;
	ready = -1;
      } else {
	fp->pos -= 32;
      }
    } else if (openFileMode == ofmNext) {
      fp->pos += 32;
    }
#if 0
    printf("%ld\n", fp->pos);
#endif

    if (!ready) {
      enum EntryType currEntryType;

      currEntryType = CheckEntryType(fp, name);
      if (currEntryType == etEnd) {
	lastGoodEntry |= 0x8000U;
	ready = -1;
      } else if (currEntryType == entryType) {
	s_int32 oldPos = fp->pos;
	char *nameLong;
	int nameLeft;

	/* Check whether we can find a long name */
	ready = 0;
	nameLeft = nameSize - 1 - strlen(name);
	nameLong = name + strlen(name)+1;
	if (fp->pos < 64) {
	  //	  printf("###### fp->pos %ld\n", fp->pos);
	  ready = 2;
	} else {
	  while (!ready && nameLeft > 15 && fp->pos >= 64) {
	    enum EntryType currEntryType;
	    fp->pos -= 32;
	    currEntryType = CheckEntryType(fp, nameLong);
	    //	    printf("#3: pos %ld, cet %d\n", fp->pos, currEntryType);
	    if (currEntryType != etLongName &&
		currEntryType != etLongNameLast) {
	      nameLong[0] = '\0';
	      ready = 2;
	    } else {
	      nameLeft -= strlen(nameLong);
	      nameLong += strlen(nameLong);
	    if (currEntryType == etLongNameLast)
	      ready = 2;
	    }
	  }
	}

#if 1
	//	printf("nm %s:%s\n", name, name+strlen(name)+1);
	/* If there is no longname... */
	if (nameLong == name+strlen(name)+1) {
	  int i = 0;
	  /* Then lowercase copy short name to long name */
	  do {
	    nameLong[i] = tolower(name[i]);
	  } while (name[i++]);
	} else {
	  nameLong = name+strlen(name)+1;
	}
	//	printf("#4: name %s/%s\n", name, nameLong);
#endif
	fp->pos = oldPos;

	if (!ready) {
	  ready = -1;
	} else if (suffixes) {
#if 1
	  /* Now we've established a long name. It's time to see if we
	     must dispose of it because the suffixes don't match. */
	  char *suf = suffixes;
	  char *currSuf = strrchr(nameLong, '.');
	  //	  printf("Compare suf %s for %s/%s\n", suffixes, name, nameLong);
	  ready = 0;
	  if (currSuf) {
	    currSuf++;
	    while (!ready && *suf) {
	      int sufLen;
	      {
		char *sufEnd = strchr(suf,',');
		if (sufEnd) {
		  sufLen = sufEnd - suf;
		} else {
		  sufLen = strlen(suf);
		}
	      }
#if 0
	      printf("  Compare %s[%d] with %s\n",
		     suf, sufLen, currSuf);
#endif
	      /* Do we have a suffix match? */
	      if (sufLen == strlen(currSuf) &&
		  !strncasecmp(suf, currSuf, sufLen)) {
#if 0
		printf("#6: suf-match: %s & %s\n", currSuf, suf);
#endif
		ready = 2;
	      } else {
		suf += sufLen;
		if (*suf == ',')
		    suf++;
	      }
	    }
	  } /* if (currSuf) */
#endif
	} /* if (suffixes) */
	if (ready > 0) {
	  /* Update file number */
	  lastGoodEntry = (lastGoodEntry & 0x7FFFU) + addFileNumber;
	  lastGoodEntryPos = oldPos;
#if 0
	  printf("#7: ready %d, %d %ld\n",
		 ready, lastGoodEntry, lastGoodEntryPos);
#endif

	  /* If a name search was to be performed */
	  if (searchName) {
#if 0
	    printf("  Compare %p %p %s %s\n",
		   searchName, name, searchName, name);
#endif
	    if (!strcasecmp(searchName, name) ||
		!strcmp(searchName, nameLong)) {
	      /* If match found, move result to right place. */
	      memmove(searchName, name, strlen(name)+strlen(nameLong)+2);
	    } else {
	      /* Else invalidate search. */
	      ready = 0;
	    }
	  } else if (--numOfFiles) {
	    /* If we should skip files... */
	    ready = 0;
	  }
#if 0
	  printf("#7.5: ready %d, %d %ld\n",
		 ready, lastGoodEntry, lastGoodEntryPos);
#endif
	}
      } /* if (currEntryType == entryType) */

#if 0
      printf("#7.7: ready %d, %d %ld\n",
	     ready, lastGoodEntry, lastGoodEntryPos);
#endif

    } /* if (!ready) */

#if 0
    printf("#7.8: ready %d, %d %ld\n",
	   ready, lastGoodEntry, lastGoodEntryPos);
#endif
  } while (!ready && openFileMode != ofmSame);

  fde->lastGoodEntry = lastGoodEntry;
  fp->pos = lastGoodEntryPos;

#if 0
  printf("#9: ready %d, %x %ld, 666=%d\n",
	 ready, lastGoodEntry, lastGoodEntryPos, 666);
#endif

  if (ready > 0) {
    return fde->lastGoodEntry;
  } else {
    return -1;
  }
}



auto s_int16 InitDirectory(FILE **fpp) {
  FILE *fp;
  struct FatDirExt *fde;

  EndDirectory(fpp);

  return SetDirectory(fpp, NULL);
}

auto s_int16 AddDirPath(struct FatDirExt *fde, const char *s, int n) {
  int len = strlen(fde->currentDir);
#if 0
  printf("  ADP(fde, \"%s\", %d)\n", s, n);
  printf("    <- \"%s\"\n", fde->currentDir);
#endif
  if (!s || !len)
    return 0;
  if (!strcmp(s, "."))
    return 0;
  if (!strcmp(s, "..")) {
    if (len > 2) {
      char *slash;
      fde->currentDir[len-1] = '\0';
      slash = strrchr(fde->currentDir, '/');
      if (!slash) {
	fde->currentDir[2] = '\0';
      } else {
	slash[1] = '\0';
      }
    }
    return 0;
  }


  if (len+n+1 >= FAT_DIR_EXT_CHARS)
    return -1;
  strncat(fde->currentDir, s, n);
  fde->currentDir[len+n] = '\0';
  if (strlen(fde->currentDir) > 2) {
    strcat(fde->currentDir, "/");
  }
#if 0
  printf("    -> \"%s\"\n", fde->currentDir);
#endif
  return 0;
}

auto s_int16 SetDirectory(FILE **fpp, const char *s) {
  FILE *fp = *fpp;
  struct FatDirExt *fde = NULL;
  //  printf("#S1, cD \"??\", \"%s\"\n", s ? s : "");
  if (fp) {
    fde = fp->extraInfo;
    fclose(fp);
    *fpp = NULL;
  }
  //  printf("#S2, cD \"??\", \"%s\"\n", s ? s : "");
  if (!fde) {
    if (!(fde = malloc(sizeof(struct FatDirExt)))) {
      return -1;
    }
    strcpy(fde->currentDir, "E:");
  }


  //  printf("#2.1 cD \"%s\"\n", fde->currentDir);
  if (s) {
    if (s[0] == ':') {
      fde->currentDir[2] = '\0';
      s++;
    } else if (strlen(s) >= 2 && s[1] == ':') {
      fde->currentDir[0] = s[0];
      fde->currentDir[2] = '\0';
      s += 2;
    }
    while (*s) {
      char *slash = strchr(s, '\\');
      //      printf("#2.2 s \"%s\"\n", s);
      {
	char *slash2 = strchr(s, '/');
	if (slash2) {
	  if (slash) {
	    if (slash2 < slash) {
	      slash = slash2;
	    }
	  } else {
	    slash = slash2;
	  }
	}
      }
      if (strlen(s)) {
#if 0
	printf("  SLASH %p, len %d\n", slash,
	       slash ? slash-s : strlen(s));
#endif
	if (AddDirPath(fde, s, slash ? slash-s : strlen(s))) {
	  free(fde);
	  return -1;
	}
      }
      if (slash) {
	s = slash+1;
      } else {
	s += strlen(s);
      }
    }
  }

  //  printf("#2.5 cD \"%s\"\n", fde->currentDir);
  fp = fopen(fde->currentDir, "s");
  if (!fp) {
    free(fde);
    return -1;
  }
  //  printf("#S3\n");
  fp->flags |= __MASK_PRESENT;
  fp->extraInfo = fde;
  if (strlen(fde)+14 >= FAT_DIR_EXT_CHARS) {
    *fpp = fp;
    return -1;
  }
  //  printf("#S4, cD \"%s\"\n", fde->currentDir);

  if (FatFindFirst(fp, ">", fde->currentDir+strlen(fde)+1, 13)) {
    free(fde);
    fclose(fp);
    *fpp = NULL;
    return -1;
  } else {
    fp->pos = 0;
    fp->extraInfo = fde;
  }
  //  printf("#S5, cD \"%s\", %p %p\n", fde->currentDir, fde, fp->extraInfo);

  *fpp = fp;
  return 0;
}

auto const char *GetDirectory(FILE **fpp) {
  FILE *fp = *fpp;
  struct FatDirExt *fde;
  if (fp) {
    fde = fp->extraInfo;
    return fde->currentDir;
  }
  return NULL;
}

auto s_int16 EndDirectory(FILE **fpp) {
  FILE *fp = *fpp;
  if (fp) {
    struct FatDirExt *fde = fp ? fp->extraInfo : NULL;
    if (fde)
      free(fde);
    fclose(fp);
  }
  *fpp = NULL;
}
