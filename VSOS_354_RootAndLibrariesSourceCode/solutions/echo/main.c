/* For free support for VSIDE, please visit www.vsdsp-forum.com */

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <consolestate.h>
#include <string.h>
#include <consolestate.h>
#include <kernel.h>

ioresult main(char *parameters) {
  int noMoreParams=0, nParam=0, printNewLine=1;
  int hasPrintedAnything=0;
  char *p = parameters;
  int i;

  nParam = RunProgram("ParamSpl", parameters);

  for (i=0; i<nParam; i++) {
    if (!noMoreParams) {
      if (!strcmp(p, "-n")) {
	printNewLine = 0;
      } else if (!strcmp(p, "+n")) {
	printNewLine = 1;
      } else if (!strcmp(p, "-e")) {
	appFlags &= ~APP_FLAG_ECHO;
      } else if (!strcmp(p, "+e")) {
	appFlags |= APP_FLAG_ECHO;
      } else if (!strcmp(p, "-h")) {
	printf("Usage: echo [-n|+n|-e|+e|-|-h] string\n"
	       "-n\tNo newline\n"
	       "+n\tOutput newline\n"
	       "-e\tTurn shell interactive echo mode off\n"
	       "+e\tTurn shell interactive echo mode on\n"
	       "-\tEnd of parameters\n"
	       "-h\tShow this help\n"
	       "Note: String may contain escape codes such as \\\" and \\n");
      } else if (!strcmp(p, "-")) {
	noMoreParams = 1;
      } else {
	if (hasPrintedAnything) {
	  printf(" ");
	}
	printf("%s", p);
	hasPrintedAnything = 1;
      }
    } else {
	if (hasPrintedAnything) {
	  printf(" ");
	}
	printf("%s", p);
	hasPrintedAnything = 1;
    }
    p += strlen(p)+1;
  }
  if (printNewLine) {
    printf("\n");
  }
}
