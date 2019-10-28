/* For free support for VSIDE, please visit www.vsdsp-forum.com */

// Starting point template for creating VSOS3 libraries and device drivers.
// This will create a <projectname>.DL3 file, which you can copy to 
// your VS1005 Developer Board's system disk's SYS subdirectory.

// If init(), main() or fini() are not needed, remove them from the solution.
// There's no need to have any unneeded functions in the library.

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <string.h>
#include <kernel.h>

ioresult main(char *parameters) {
  char *p = parameters;
  int nParam = RunProgram("ParamSpl", parameters);
  int printInfo = 0, i;
  int retCode = S_OK;

  for (i=0; i<nParam; i++) {
    if (!strcmp(p, "-c")) {
      printInfo = 1;
    } else if (!strcmp(p, "+c")) {
      printInfo = 0;
    } else if (!strcmp(p, "-h")) {
      printf("Usage: type [-c|+c|-h] [file1 [file2 [...]]]\n"
	     "-c\tPrint file information line\n"
	     "+c\tDon't print file information line\n"
	     "-h\tShow this help\n");
      goto finally;
    } else {
      FILE *fp = fopen(p, "r");
      if (!fp) {
	printf("File \"%s\" not found.\n", p);
	retCode = S_ERROR;
      } else {
	fseek(fp, 0, SEEK_END);
	if (printInfo) {
	  printf("--------\n");
	  printf("File: %ld bytes, name \"%s\"\n",
		 ftell(fp), fp->Identify(fp, NULL, 0));
	  printf("--------\n");
	}
	fseek(fp, 0, SEEK_SET);
	while (!feof(fp)) {
	  static char s[255];
	  if (fgets(s, 255, fp)) {
	    fputs(s, stdout);
	  }
	}
	fclose(fp);
      }
    }
    p += strlen(p)+1;
  }

 finally:
  return retCode;
}

