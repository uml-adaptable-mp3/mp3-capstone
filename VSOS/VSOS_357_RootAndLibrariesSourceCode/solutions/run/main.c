/* For free support for VSIDE, please visit www.vsdsp-forum.com */

/// \file run.c lib to run other libs from config.sys without them remaining in the memory
/// \author Panu-Kristian Poiksalo, VLSI Solution OY

#include <apploader.h>
#include <string.h>
#include <kernel.h>

ioresult main(char *program) {
  char *p;
  printf(program);
  p = strchr(program, ' ');
  // Note: if strchr fails to find whitespace, it returns a NULL pointer.
  // This is not a problem because VSOS requires that X:0 always contains
  // a NUL character, so the parameter list will be empty.
  while (*p == ' ') {
    *p++ = '\0';
  }
  return RunProgram(program, p);
}
