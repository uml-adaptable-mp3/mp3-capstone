#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <string.h>
#include <stdlib.h>
#include <copyfile.h>
#include <kernel.h>


DLLENTRY(main)
ioresult main(char *parameters) {
  int nParam = RunProgram("ParamSpl", parameters);
  char *p = parameters, *srcFileName=NULL, *dstFileName=NULL;
  int i;
  ioresult retVal = S_ERROR;

  for (i=0; i<nParam; i++) {
    if (!strcmp(p, "-h")) {
      printf("Usage: CopyF [-h] [src dst]\n"
	     "src\tFile to be copied\n"
	     "dst\tDestination file\n"
	     "-h\tShow this help\n");
      retVal = S_OK;
      goto finally;
    } else {
      if (!srcFileName) {
	srcFileName = p;
      } else if (!dstFileName) {
	dstFileName = p;
      } else {
	printf("E: Extraneous parameter \"%s\"\n", p);
	goto finally;
      }
    }
    p += strlen(p)+1;
  }

  if (!dstFileName) {
    printf("E: File name(s) missing\n");
    goto finally;
  }

  if ((retVal = CopyFile(dstFileName, srcFileName))) {
    printf("E: Couldn't copy \"%s\" to \"%s\"\n", srcFileName, dstFileName);
  }
 finally:
  return retVal;
}
