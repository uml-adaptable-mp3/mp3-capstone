/* For free support for VSIDE, please visit www.vsdsp-forum.com */

/// \file liblist.c Print out a list of loaded libraries and free mem to stdout
/// \author Panu-Kristian Poiksalo, VLSI Solution Oy

#if 1
#define USE_MEM_TRACK
#endif

#include <vo_stdio.h>
#include <volink.h>
#include <apploader.h>
#include <lcd.h>
#include <stdbuttons.h>
#include <string.h>
#include <stdlib.h>
#include <lists.h>
#include <sysmemory.h>
#include <ctype.h>
#include <kernel.h>
#ifdef USE_MEM_TRACK
#include "memtrack.h"
#endif


extern const void *pIFlist;
struct LIST *myiFlist;
extern struct LIST *xFlist;
extern struct LISTY __mem_y *yFlist;
#ifdef USE_MEM_TRACK
struct MemTrackElement __mem_y *memTrackP = NULL;
#endif


struct MEMORYNODE {
	struct MINNODE node;
	u_int16 size;
};

struct MEMORYNODEY {
	struct MINNODEY node;
	u_int16 size;
};

s_int16 IsAllocated(register u_int16 ixy, register u_int16 addr) {
  s_int16 sum = 0;
  s_int16 isFree = 0;
  myiFlist = pIFlist;
  switch (ixy) {
  case 0:
    {
      volatile void *m = HeadNodeI(myiFlist);
      while (m) {
	u_int16 size = GetSizeNodeI(m);
	if (addr >= (u_int16)IMEMADDR(m) && addr < (u_int16)IMEMADDR(m)+size) {
	  isFree = 1;
	}
	sum += size;
	m = NextNodeI(m);			
      }
    }
    break;
  case 1:
    {
      struct MEMORYNODE *m = (void *)HeadNode(xFlist);
      while (m) {
	u_int16 size = m->size;
	if (addr >= (u_int16)m && addr < (u_int16)m+size) {
	  isFree = 1;
	}
	sum += size;
	m = (void *)NextNode((void *)m);
      }
    }
    break;
  case 2:
    {
      struct MEMORYNODEY __mem_y *m = (void __mem_y *)HeadNodeY(yFlist);
      while (m) {
	u_int16 size = m->size;
	if (addr >= (u_int16)m && addr < (u_int16)m+size) {
	  isFree = 1;
	}
	sum += size;
	m = (void __mem_y *)NextNodeY((void __mem_y *)m);
      }
    }
    break;
  default:
    return -1;
    break;
  }
  return (1-isFree) * sum;
}



s_int16 IsAllocatedByLib(register u_int16 ixy, register u_int16 addr,
			 register u_int16 *lib) {
  u_int16 sections;
  int i;

  lib += *lib+6; /* lib += entries + 6 */
  sections = *lib++;

  for (i=0; i<sections; i++) {
    u_int16 secSize = (*lib >> 2) << 1;
    u_int16 secPage = *lib++&3;
    u_int16 secAddr = *lib++;

    /* See if this address was inside a section. */
    if (secPage == ixy && addr >= secAddr && addr < secAddr+secSize) {
      return 1;
    }

#ifdef USE_MEM_TRACK
    /* If code section, and memTrack is on, see if this matches with
       tracked memory. */
    if (memTrackP && secPage == 0 && (ixy == 1 || ixy == 2)) {
      int j, y = (ixy==2);
      struct MemTrackElement __mem_y *elem = memTrackP;
      for (j=0; j<MEM_TRACK_LIST_SIZE; j++) {
	if (y == (elem->sizeAndY>>15) && elem->addr &&
	    addr >= elem->addr && addr < elem->addr+(elem->sizeAndY&0x7FFF) &&
	    elem->owner >= secAddr && elem->owner < secAddr+secSize) {
	  return 1;
	}
	elem++;
      }
    }
#endif
  }
  return 0;
}



const char *GetLibName(register u_int16 *lib) {
  static char s[9];
  char *p;
  if (!lib) {
    s[0] = '\0';
  } else {
    lib += *lib + 6;
    strcpy(s, (char *)(lib + *lib * 2 + 2));
    if ((p = strchr(s, '.'))) {
      *p = '\0';
    }
  }
  return s;
}

#define MAX_LIB_NAME_LEN 8

char libChars[MAX_LIB+1] = {0};
char libNames[MAX_LIB][MAX_LIB_NAME_LEN+1];

ioresult main(char *parameters) {
  int nParam = RunProgram("ParamSpl", parameters);
  char *p = parameters;
  u_int16 i, currChar = '0';
  u_int16 step = 64;
  u_int16 ixy = 0;
  u_int16 res = S_ERROR;
  u_int16 verbose = 1;
  static char ixyStr[3] = {'I','X','Y'};

#ifdef USE_MEM_TRACK
  memTrackP = GetSymbolAddress("memTrack");
#endif

  for (i=0; i<nParam; i++) {
    if (!strcmp(p, "-v")) {
      verbose = 1;
    } else if (!strcmp(p, "+v")) {
      verbose = 0;
    } else if (!strcmp(p, "-h")) {
      printf("Usage: Frags [-v|+v|-h]\n"
	     "-v|+v\tVerbose on|off\n"
	     "-h\tShow this help\n");
      res = S_OK;
      goto finally;
    }
  }

  if (!verbose) {
    goto notVerbose;
  }

  /* Clear library name character list. */
  for (i=0; i<MAX_LIB; i++) {
    char *p = libNames[i];
    libChars[i] = '.';
    strcpy(p, GetLibName(loadedLib[i]));
    while (*p) {
      *p = tolower(*p);
      p++;
    }
  }

  /* Assign characters to each library name.
     Try to be intelligent about it: first check each available name's
     first character and assign if free, then second character, etc. */
  for (i=0; i<MAX_LIB_NAME_LEN; i++) {
    int j;
    for (j=0; j<MAX_LIB; j++) {
      char testChar = toupper(libNames[j][i]);
      if (libChars[j] == '.' && strlen(libNames[j]) > i && !strchr(libChars, testChar)) {
	libChars[j] = testChar;
      }
    }
  }

#if 0
  /* To test following piece of code. */
  libChars[1] = libChars[4] = '.';
#endif

  /* If there still are blank characters (highly unlikely), fill them
     with numbers. */
  for (i=0; i<MAX_LIB; i++) {
    if (libChars[i] == '.') {
      while (strchr(libChars, currChar)) {
	currChar++;
      }
      libChars[i] = currChar;
    }
  }

  printf("Libs: ");
  for (i=0; i<MAX_LIB; i++) {
    if (loadedLib[i]) {
      char *p, c;
      if ((p = strchr(libNames[i], tolower(c=libChars[i])))) {
	*p = c;
	printf("%s%s", i?" ":"", libNames[i]);
      } else {
	printf("%s%c=%s", i?" ":"", libChars[i], libNames[i]);
      }
    }
  }

#if 0
  if (memTrackP) {
    int i;
    struct MemTrackElement __mem_y *elem = memTrackP;
    for (i=0; i<MEM_TRACK_LIST_SIZE; i++) {
      if (elem->addr) {
	printf("%02x: %04x %04x %04x\n",
	       i, elem->addr, elem->sizeAndY, elem->owner);
      }
      elem++;
    }
  }
#endif


  for (ixy=0; ixy<3; ixy++) {
    u_int16 addr, column = 0;
    printf("\n");
    for (addr=0; addr<32768; addr+=step) {
      if (!column) {
	printf("%c %04x: ", ixyStr[ixy], addr);
      } else if (!(column & 15)) {
	printf(" ");
      }
      if (IsAllocated(ixy, addr)) {
	char allocChar = '#';
	/* Ok, we know this piece is allocated by someone. But by whom? */
	for (i=0; i<MAX_LIB; i++) {
	  if (loadedLib[i] && IsAllocatedByLib(ixy, addr, loadedLib[i])) {
	    allocChar = libChars[i];
	  }
	}
	printf("%c", allocChar);
      } else {
	/* Memory is free */
	printf(".");
      }
      if (++column >= 64) {
	column = 0;
	printf("\n");
      }
    }
  }

  printf("\n");
 notVerbose:
  printf("Free: I: %5d words (%d%%), X: %5d words (%d%%), "
	 "Y: %5d words (%d%%)\n",
	 IsAllocated(0, 0), (int)((u_int32)IsAllocated(0, 0)*100/32768),
	 IsAllocated(1, 0), (int)((u_int32)IsAllocated(1, 0)*100/32768),
	 IsAllocated(2, 0), (int)((u_int32)IsAllocated(2, 0)*100/32768));


  res = S_OK;
 finally:
  return res;
}
