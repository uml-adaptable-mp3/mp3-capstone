/* For free support for VSIDE, please visit www.vsdsp-forum.com */

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <consolestate.h>
#include <string.h>
#include <ctype.h>
#include <kernel.h>

ioresult main(char *parameters) {
  int escape=0, lastZero=1, inParen=0, nParam=0, printNewLine=1;
  int hasPrintedAnything=0;
  char *p = parameters;
  int i;

  if (!parameters) {
    goto finally;
  }

  while (*parameters) {
    if (escape) {
      escape = 0;
      if (lastZero) {
	lastZero = 0;
	nParam++;
      }
      switch (*parameters) {
      case 'a':
	*p++ = '\a';
	break;
      case 'b':
	*p++ = '\b';
	break;
      case 'n':
	*p++ = '\n';
	break;
      case 'r':
	*p++ = '\r';
	break;
      case 't':
	*p++ = '\t';
	break;
      default:
	*p++ = *parameters;
      }
    } else if (*parameters == '"') {
      inParen = !inParen;
    } else if (!inParen && *parameters == ' ') {
      if (!lastZero) {
	lastZero = 1;
	*p++ = '\0';
      }
    } else if (*parameters == '\\') {
      escape = 1;
    } else {
      if (lastZero) {
	lastZero = 0;
	nParam++;
      }
      *p++ = *parameters;
    }
    parameters++;
  }
  *p = '\0';

 finally:
  return nParam;
}
