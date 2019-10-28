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
#include <kernel.h>
#include "disassemble.h"
#include "printbits.h"
#include "printsymbol.h"

#if 0
u_int16 myX;
u_int16 myY;
static u_int16 myStaticX;
static u_int16 myStaticY;

__align u_int16 myAlignX[5];
__align u_int16 myAlignY[5];
__align static u_int16 myStaticAlignX[10];
__align static u_int16 myStaticAlignY[10];

u_int16 myBufX[7];
u_int16 myBufY[7];
static u_int16 myStaticBufX[12];
static u_int16 myStaticBufY[12];
#endif

void PrintLibSymbol(const char *s);

ioresult main(char *parameters) {
  int nParam=0;
  char *p = parameters;
  int i;
  int verbose = 2;
  int bitMode = 0;

#if 0
  i = myX+myY+myStaticX+myStaticY+(int)myAlignX+(int)myAlignY+(int)myStaticAlignX+(int)myStaticAlignY+(int)myBufX+(int)myBufY+(int)myStaticBufX+(int)myStaticBufY;
  printf("%d\n", i);
#endif

  nParam = RunProgram("ParamSpl", parameters);

  for (i=0; i<nParam; i++) {
    s_int16 pLen = strlen(p);
    if (!strcmp(p, "-h")) {
      printf("Usage: preg [-v|+v|-b|+b|-f|+f|-sSym|-h] [reg]|[addr=val|addr|=val|addr&=val|addr^=val]\n"
	     "reg\tnumber, number-number, name\n"
	     "val\tvalue, may be preceded with ~ for bitwise not\n"
	     "\tFor memory outside of Y space, precede with X: or I:\n"
	     "-l\tList all registers (decrease verbosity to not get all bits)\n"
	     "-v/+v\tIncrease / decrease verbosity\n"
	     "-b/+b\tBit mode on/off\n"
	     "-f/+f\tFast mode on/off (removes disassembly symbols)\n"
	     "-sSym\tFind public symbol Sym and print its address ('-' show all)\n"
	     "-h\tShow this help\n\n"
	     "Examples:\n"
	     "  preg i:0x20-0x3f\n"
	     "  preg ana_cf\n"
	     "  preg -b 2g\n"
	     "  preg +v -l\n"
	     "  preg y:0xfec0=0x1010\n"
	     "  preg 0xfca1&=~0x0200\n"
	     );
    } else if (!strcmp(p, "-l")) {
      PrintBitsStr("<LIST>", 2, 0, verbose);
    } else if (!strcmp(p, "-v")) {
      verbose++;
    } else if (!strcmp(p, "+v")) {
      if (verbose) verbose--;
    } else if (!strcmp(p, "-b")) {
      bitMode = 1;
    } else if (!strcmp(p, "+b")) {
      bitMode = 0;
    } else if (!strcmp(p, "-f")) {
      fastMode = 1;
    } else if (!strcmp(p, "+f")) {
      fastMode = 0;
    } else if (!strncmp(p, "-s", 2)) {
      if (!p[2]) {
	printf("E: No symbol name for option \"-s\"\n");
      } else {
	PrintSymbol(p+2);
#if 0
	PrintLibSymbol(p+2);
#endif
      }
    } else {
      int memType = 2;
      int comment = 0;
      char *pp = p;
      if (*pp) {
	if (pp[0] == '/' && pp[1] == '/') {
	  comment = 1;
	} else if (pp[1] == ':') {
	  char c = toupper(pp[0]);
	  if (c == 'I') {
	    memType = 0;
	  } else if (c == 'X') {
	    memType = 1;
	  } else if (c == 'Y') {
	    memType = 2;
	  } else {
	    memType = -1;
	  }
	  pp += 2;
	}
      }
      if (comment) {
	i = nParam;
      } else if (memType < 0) {
	printf("E: Bad memory type '%x'\n", p);
      } else if (strchr(pp, '=')) {
	SetValue(pp, memType, verbose);
      } else {
	PrintBitsStr(pp, memType, bitMode, verbose);
      }
    }
    p += pLen+1;
  }

  return S_OK;
}
