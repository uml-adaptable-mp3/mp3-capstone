/*

  MkDir - create directory in a file system.

*/
#include <vo_stdio.h>
#include <volink.h>     /* Linker directives like DLLENTRY */
#include <apploader.h>  /* RunLibraryFunction etc */
#include <string.h>
#include <unistd.h>
#include <kernel.h>


ioresult main(char *parameters) {
  ioresult retCode = S_OK;

  if (!strcmp(parameters, "-h") || !parameters[0]) {
    printf("Usage: MkDir [fulPath|-h]\n"
	     "fulPath\tFull path to the directory to create\n"
	     "-h\tShow this help\n");
      goto finally;
  } else {
    if (retCode = mkdir(parameters)) {
      printf("E: Couldn't create \"%s\"\n", parameters);
    }
  }

  
 finally:
  return retCode;
}
