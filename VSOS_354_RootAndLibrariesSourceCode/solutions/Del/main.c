/*

  Del - deletes a file or folder (recursively).

*/
#include <vo_stdio.h>
#include <volink.h>     /* Linker directives like DLLENTRY */
#include <apploader.h>  /* RunLibraryFunction etc */
#include <string.h>
#include <unistd.h>
#include <kernel.h>

int unlink_recursive(register const char *pathname, register s_int16 verbose);

ioresult main(char *parameters) {
  int i, nParam, verbose = 0;
  char *p = parameters;
  ioresult retCode = S_OK;

  nParam = RunProgram("ParamSpl", parameters);
  for (i=0; i<nParam; i++) {
    if (!strcmp(p, "-h")) {
      printf("Usage: Del [-v|+v|-h] file|dir\n"
	     "-v|+v\tVerbose on/off\n"
	     "-h\tShow this help\n");
      goto finally;
    } else if (!strcmp(p, "-v")) {
      verbose = 1;
    } else if (!strcmp(p, "+v")) {
      verbose = 0;
    } else {
      if (retCode = unlink_recursive(p, verbose)) {
	printf("E: Couldn't remove \"%s\"\n", p);
	goto finally;
      }
    }
    p += strlen(p)+1;
  }

 finally:
  return retCode;
}
