#include <vo_stdio.h>
#include <string.h>
#include <sysmemory.h>
#include <extSymbols.h>
#include "printsymbol.h"

u_int16 PrintSymbolRaw(const char *s) {
  u_int32 crc32 = SymbolCrunchStringCalcCrc32(s);
  int retVal = 0;

  {
    struct ExtSymbol __mem_y *h = SymbolFindByCrc(crc32);
    if (h) {
      printf("RAM CRC 0x%08lx, addr 0x%04x: %s\n", crc32, h->addr, s);
      retVal++;
    }
  }
  if (extSymbolSearchRom) {
    struct ExtSymbolRom *h = SymbolFindRomByCrc(crc32);
    if (h) {
      printf("ROM CRC 0x%08lx, addr 0x%04x: %s\n", crc32, h->addr, s);
      retVal++;
    }
  }

  return retVal;
}

void PrintSymbol(const char *s) {
  char *s2 = malloc(strlen(s)+5);
  u_int16 found;

  if (!strcmp(s, "-")) {
    struct ExtSymbol __mem_y *yp = extSymbol;
    struct ExtSymbolRom *xp = extSymbolRom;
    int i, n=0;
    for (i=0; i<extSymbolRomSize; i++) {
      if (xp->crc32) {
	printf("ROM CRC 0x%08lx, addr 0x%04x\n", xp->crc32, xp->addr);
	n++;
      }
      xp++;
    }
    printf("Total %d/%d ROM symbols\n", n, extSymbolRomSize);
    n = 0;
    for (i=0; i<CRC32_HASH_SIZE; i++) {
      if (yp->crc32) {
	printf("RAM CRC 0x%08lx, addr 0x%04x\n", yp->crc32, yp->addr);
	n++;
      }
      yp++;
    }
    printf("Total %d/%d RAM symbols\n", n, CRC32_HASH_SIZE);
    return;
  }

  found = PrintSymbolRaw(s);

  if (s2) {
    s2[0] = '_';
    strcpy(s2+1, s);
    found += PrintSymbolRaw(s2);

    strcpy(s2, "_vo_");
    strcat(s2+4, s+(s[0]=='_'));
    found += PrintSymbolRaw(s2);
  }

  if (!found) {
    printf("SYM %s not found\n", s);
  }
}
