/* For free support for VSIDE, please visit www.vsdsp-forum.com */

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <consolestate.h>
#include <string.h>
#include <strings.h>
#include <consolestate.h>
#include <ctype.h>
#include <stdlib.h>
#include <imem.h>
#include "disassemble.h"
#include "printbits.h"


int fastMode = 0;

const u_int16 __mem_y mask[16] = {
  0x1, 0x3, 0x7, 0xf,
  0x1f, 0x3f, 0x7f, 0xff,
  0x1ff, 0x3ff, 0x7ff, 0xfff,
  0x1fff, 0x3fff, 0x7fff, 0xffff,
};

void PrintBitsY(register const char *symName,
		register u_int16 low, register u_int16 high,
		register u_int16 bitMode,
		register s_int16 verbose) {
  FILE *fp = verbose ? fopen("S:SYS/PREG.DAT", "rb") : NULL;
  u_int16 addr, fileAddr = 0;
  static char s[64];
  static char currRegName[64];
  static char sp16[17] = "                ";
  static char five[5] = {'\0','\0','\0','\0','\0'};

  currRegName[0] = '\0';
 
#if 0
  printf("PrintBitsY(%p=\"%s\", l=%x, h=%x, b=%d, v=%d)\n", symName, symName, low, high, bitMode, verbose);
#if 0
  if (verbose) return;
#endif
#endif

  if (high == 0xFFFF) {
    high = 0xFFFE;
  }

  if (bitMode && symName && verbose > 1) {
    printf("Y-Addr Register name   Bits  Val   Bit name\n");
  }

  if (symName) {
    char *p = symName;
    while (*p) {
      *p = toupper(*p);
      p++;
    }
  }

  s[0] = '\0';
  if (!fp) {
    if (symName || verbose < 0) {
      return;
    }
    for (addr=low; addr<=high; addr++) {
      printf("Y:0x%04x=0x%04x\n", addr, USEY(addr));
    }
    return;
  }

  for (addr=low; addr<=high; addr++) {
    while (fileAddr < addr && fp) {
      if (!fgets(s, 64, fp)) {
	fclose(fp);
	fp = NULL;
      } else {
	if (s[0] == 'F') {
	  memcpy(five, s, 4);
	  strcpy(currRegName, s+4);
	  currRegName[strlen(currRegName)-1] = '\0';
	  fileAddr = (int)strtol(five, NULL, 16);
	}
      }
    }
    if (!symName && fileAddr != addr || !fileAddr || !fp) {
      if (!bitMode) {
	if (verbose >= 0) {
	  printf("Y:0x%04x=0x%04x\n", addr, USEY(addr));
	}
      }
    } else if (!symName || bitMode ||
	       (!bitMode && addr == fileAddr &&
		(strstr(currRegName, symName) || !strcmp(symName, "<LIST>")))) {
      /* Print one complete register. */
      u_int16 data = 0;
      if (!bitMode) {
	if (verbose < 0) {
	  s[4+strlen(s+5)] = '\0';
	  printf("%s", s+4);
	} else {
	  data = USEY(addr);
	  printf("Y:0x%04x=0x%04x %s", addr, data, s+4);
	}
      }
      if (addr == fileAddr) {
	s[0] = ' ';
      }
      if (!fgets(s, 64, fp)) {
	fclose(fp);
	fp = NULL;
      } else {
	while (fp && s[0] != 'F') {
	  int bits, loBit, hiBit;
	  u_int16 localData;
	  five[0] = s[0];
	  five[1] = '\0';
	  loBit = (int)strtol(five, NULL, 16);
	  five[0] = s[1];
	  bits = (int)strtol(five, NULL, 16)+1;
	  hiBit = loBit+bits-1;
	  localData = (data>>loBit) & mask[bits-1];
	  if (verbose > 1 && (!bitMode || strstr(s+2, symName))) {
	    if (bitMode) {
	      printf("0x%04x %-14s", addr, currRegName);
	    }
	    if (bits == 1) {
	      printf("%s%2d", sp16+6+bitMode*6, loBit);
	    } else {
	      printf("%s%2d:%d", sp16+9+bitMode*6-(loBit<10), hiBit, loBit);
	    }
	    if (bits <= 4) {
	      printf("    %01x", localData);
	    } else if (bits <= 8) {
	      printf("   %02x", localData);
	    } else if (bits <= 12) {
	      printf("  %03x", localData);
	    } else {
	      printf(" %04x", localData);
	    }
	    printf("   %s", s+2);
	  }
	  if (!fgets(s, 64, fp)) {
	    fclose(fp);
	    fp = NULL;
	  }
	}
	if (fp && s[0] == 'F') {
	  memcpy(five, s, 4);
	  strcpy(currRegName, s+4);
	  currRegName[strlen(currRegName)-1] = '\0';
	  fileAddr = (int)strtol(five, NULL, 16);
	}
      }
    }
  }

  if (fp) {
    fclose(fp);
  }
}




void PrintBitsStr(register char *s, register u_int16 memType,
		  register u_int16 bitMode,
		  register s_int16 verbose) {
  char *endP;
  u_int16 addr = (int)strtol(s, &endP, 0);
  u_int16 endAddr = addr;
  u_int16 diff = endAddr-addr;
  if (*endP == '-') {
    endAddr = (int)strtol(endP+1, &endP, 0);
  } else if (*endP == '+') {
    if (endP[1] == '-') {
      int t = (int)strtol(endP+2, &endP, 0);
      endAddr = addr + t;
      addr -= t;
    } else {
      endAddr = addr+(int)strtol(endP+1, &endP, 0);
    }
  }
  if (memType == 2 && *endP) {
    PrintBitsY(s, 0xfc00, 0xffff, bitMode, verbose);
    return;
  }
  /* If *endP is not 0, then the parameter was not a proper number */
  if (*endP) {
    printf("E: \"%s\" not a proper number and no register by this name\n", s);
  } else if (!memType) {
    if (!fastMode) {
      RunLibraryFunction("TRACE", ENTRY_1, addr);
      printf(":\n");
    }

    do {
      PrintDisassembled(addr, ReadIMem(addr), fastMode);
    } while (addr++ != endAddr);
  } else if (memType == 1) {
    printf("X:0x%04x=0x%04x\n", addr, USEX(addr));
    while (addr != endAddr) {
      ++addr;
      printf("X:0x%04x=0x%04x\n", addr, USEX(addr));
    }
  } else {
    PrintBitsY(NULL, addr, endAddr, 0, verbose);
  }
}

void SetValue(register char *s, register u_int16 memType,
	      register s_int16 verbose) {
  char *leftEnd = strchr(s, '='), *rightStart = leftEnd+1;
  u_int16 doOr, doAnd, doXor, doNot=0;
  u_int16 addr = (int)strtol(s, NULL, 0);
  s_int32 valXor = 0;
  u_int32 val, oldVal = 0;

  if (rightStart[0] == '~') {
    rightStart++;
    valXor = 0xFFFFFFFFU;
  }
  val = valXor ^ strtol(rightStart, NULL, 0);

  doOr = (strstr(s, "|=") == leftEnd-1);
  doAnd = (strstr(s, "&=") == leftEnd-1);
  doXor = (strstr(s, "^=") == leftEnd-1);

  if (doOr || doAnd || doXor) {
    leftEnd--;
    if (!memType) {
      oldVal = ReadIMem(addr);
    } else if (memType == 1) {
      oldVal = (u_int32)USEX(addr);
    } else {
      oldVal = (u_int32)USEY(addr);
    }
  }
  *leftEnd = '\0';

  if (doOr) {
    val |= oldVal;
  } else if (doAnd) {
    val &= oldVal;
  } else if (doXor) {
    val ^= oldVal;
  }

  if (!memType) {
    WriteIMem(addr, val);
  } else if (memType == 1) {
    USEX(addr) = (u_int16)val;
  } else {
    USEY(addr) = (u_int16)val;
  }
  PrintBitsStr(s, memType, 0, verbose);
}
