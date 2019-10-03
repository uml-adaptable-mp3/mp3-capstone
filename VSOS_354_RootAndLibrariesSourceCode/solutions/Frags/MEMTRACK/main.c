/// \file main.c VSOS3 MemTrack
/// \author Henrik Herranen, VLSI Solution Oy
#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <devAudio.h>
#include <timers.h>
#include <string.h>
#include <mutex.h>
#include <kernel.h>
#include <devAudio.h>
#include <aucommon.h>
#include <extSymbols.h>
#include <kernel.h>
#include "memtrack.h"

void (*RomStart)(void) = (void (*)(void)) 0x8000;



struct ExtSymbol __mem_y *memTrackSym = NULL;

DLLENTRY(init)
ioresult init(char *paramStr) {
  SwapMemTrackVectors();
  memTrackSym = AddSymbol("memTrack", NULL, (u_int16)memTrack);

  return S_OK;
}


DLLENTRY(fini)
void fini(void) {
  // Add code here to force release of resources such as 
  // memory allocated with malloc, entry points, 
  // hardware locks or interrupt handler vectors.
  if (memTrackSym) {
    SymbolDelete(memTrackSym->crc32);
  }

  SwapMemTrackVectors();
}



void DelMemTrack(register u_int16 page, register u_int16 addr) {
  if (page > 2) {
    DelMemTrack(1, addr);
    DelMemTrack(2, addr);
  } else {
    int i, y = (page==2);
    struct MemTrackElement __mem_y *elem = memTrack;
    for (i=0; i<MEM_TRACK_LIST_SIZE; i++) {
      if (elem->addr == addr && y == (elem->sizeAndY>>15)) {
	elem->addr = 0;
      }
      elem++;
    }
  }
}

void AllocMemTrack(register u_int16 page, register u_int16 addr,
		   register u_int16 size, register u_int16 regLR0) {
  if (page > 2) {
    AllocMemTrack(1, addr, size, regLR0);
    AllocMemTrack(2, addr, size, regLR0);
  } else if (addr) {
    int i;
    struct MemTrackElement __mem_y *elem = memTrack;
    DelMemTrack(page, addr);
    for (i=0; i<MEM_TRACK_LIST_SIZE; i++) {
      if (!elem->addr) {
	elem->addr = addr;
	elem->sizeAndY = size + ((page==2)<<15);
	elem->owner = regLR0;
	goto finally;
      }
      elem++;
    }
  }

 finally:
#if 0
  {
    int i;
    struct MemTrackElement __mem_y *elem = memTrack;
    for (i=0; i<MEM_TRACK_LIST_SIZE; i++) {
      if (elem->addr) {
	printf("%02x: %04x %04x %04x\n",
	       i, elem->addr, elem->sizeAndY, elem->owner);
      }
      elem++;
    }
  }
#else
  {}
#endif
}




u_int16 MyMalloc(register __a0 u_int16 addr, u_int16 size, u_int16 caller, u_int16 page) {
#if 0
  printf("MyMalloc: page %p, addr %p, size %p, caller %p\n", page, addr, size, caller);
#endif
  AllocMemTrack(page, addr, size, caller);
  return addr;
}

u_int16 MyCalloc(register __a0 u_int16 addr, u_int16 nmemb, u_int16 size) {
  u_int16 caller = USEY(&nmemb), page = USEY(&size);
#if 0
  printf("MyCalloc: page %p, addr %p, nmemb %p * size %p, caller %p\n", page, addr, nmemb, size, caller);
#endif
  AllocMemTrack(page, addr, size*nmemb, caller);
  return addr;
}


u_int16 MyReAlloc(register __a0 u_int16 addr, u_int16 ptr, u_int16 size) {
  u_int16 caller = USEY(&ptr), page = USEY(&size);
#if 0
  printf("MyReAlloc: page %p, addr %p, size %p, caller %p\n", page, addr, size, caller);
#endif
  DelMemTrack(page, addr);
  AllocMemTrack(page, addr, size, caller);
  return addr;
}

void MyFree(u_int16 caller, u_int16 page, u_int16 addr) {
#if 0
  printf("MyFree:   page %p, addr %p, caller %p\n", page, addr, caller);
#endif
  DelMemTrack(page, addr);
}


void *MyAllocMem(register __a0 void *res, u_int16 p1, u_int16 p2) {
  u_int16 regLR0 = USEY(&p1), p4 = USEY(&p2);
  char page=p4&3;
  u_int16 op = p4>>2;
#if 0
  const char xyi[4] = {'I', 'X', 'Y', 'Z'};
  char pageChar = xyi[page];
#endif

  switch(op) {
  case 0:
#if 0
    printf("AllocMem%c(size=0x%04x, align=0x%04x) = res %04x, lr0 %04x\n",
	   pageChar, p1, p2, res, regLR0);
#endif
    AllocMemTrack(page, (u_int16)res, p1, regLR0);
    break;
  case 1:
#if 0
    printf("###ERROR### FreeMem%c(ptr=0x%04x, size=0x%04x), lr0 %04x\n",
	   pageChar, p1, p2, !regLR0);
#endif
    DelMemTrack(page, p1);
    break;
  case 2:
#if 0
    printf("ReAllocMem%c(ptr=0x%04x, oldSize=0x%04x, newSize=0x%04x) = res %04x, lr0 %04x\n",
	   pageChar, p1, p2, USEX(&p2-1), res, regLR0);
#endif
    if (res) {
      AllocMemTrack(page, (u_int16)res, USEX(&p2-1), regLR0);
    }
    break;
  case 3:
#if 0
    printf("AllocMemAbs%c(addr=0x%04x, size=0x%04x) = res %04x, lr0 %04x\n",
	   pageChar, p1, p2, res, regLR0);
#endif
    AllocMemTrack(page, (u_int16)res, p2, regLR0);
    break;
  default:
#if 0
    printf("ERROR #%d\n", p4);
#endif
    break;
  }
  return res;
}

void MyFreeMem(u_int16 p1, u_int16 p2) {
  u_int16 regLR0 = USEY(&p1), p4 = USEY(&p2);
  char page=p4&3;
  u_int16 op = p4>>2;
#if 0
  const char xyi[4] = {'I', 'X', 'Y', 'Z'};
  char pageChar = xyi[page];
  printf("FreeMem%c(ptr=0x%04x, size=0x%04x), lr0 %04x\n",
	 pageChar, p1, p2, !regLR0);
#endif
  DelMemTrack(page, p1);
}



struct MemTrackElement __mem_y memTrack[MEM_TRACK_LIST_SIZE];
