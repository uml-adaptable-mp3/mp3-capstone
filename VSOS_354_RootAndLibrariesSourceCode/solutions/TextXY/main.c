/* For free support for VSIDE, please visit www.vsdsp-forum.com */

// Starting point template for creating VSOS3 libraries and device drivers.
// This will create a <projectname>.DL3 file, which you can copy to 
// your VS1005 Developer Board's system disk's SYS subdirectory.

// If init(), main() or fini() are not needed, remove them from the solution.
// There's no need to have any unneeded functions in the library.

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <kernel.h>		// Kernel symbols
#include <consolestate.h> // appFlags etc
#include <string.h>
#include <lcd.h>
#include <stdlib.h>

#if 0
DLLENTRY(ExampleLibraryFunction)
int ExampleLibraryFunction(int i) {
	printf("You called ExampleLibaryFunction with parameter: %d.\n",i);
}
#endif

#if 0
void init(void) {
}
#endif

// Startup code for each instance of the library
// If CONFIG.TXT has several instance of the same driver, this is called for each line.
ioresult main(char *parameters) {
  int nParam, i;
  char *p = parameters;
  ioresult retVal = S_ERROR;
  int x = lcd0.x, y = lcd0.y;

  nParam = RunProgram("ParamSpl", parameters);
  for (i=0; i<nParam; i++) {
    if (!strcmp(p, "-h")) {
      printf("Usage: TextXY [-c|-h] -xX -yY text\n"
	     "-c\tClear display\n"
	     "-xX\tSet x coordinate to X\n",
	     "-yY\tSet y coordinate to Y\n",
	     "-h\tShow this help\n");
      retVal = S_OK;
      goto finally;
    } else if (!strcmp(p, "-c")) {
      LcdFilledRectangle(0, 0, lcd0.width-1, lcd0.height-1, NULL,
			 lcd0.backgroundColor);
      x = y = 0;
    } else if (!strncmp(p, "-x", 2)) {
      char *ep = NULL;
      x = (int)strtol(p+2, &ep, 0);
      if (*ep || x < 0) {
	printf("Bad X coordinate\n");
	goto finally;
      }
    } else if (!strncmp(p, "-y", 2)) {
      char *ep = NULL;
      y = (int)strtol(p+2, &ep, 0);
      if (*ep || y < 0) {
	printf("Bad Y coordinate\n");
	goto finally;
      }
    } else {
      char tc[2];
      char *pCopy = p;
      int lastWasLF = 0;
      tc[1] = '\0';
      while (tc[0] = *pCopy++) {
	if (tc[0] == '\n') {
	  lastWasLF = 1;
	  x = 0;
	  y += 8;
	} else {
	  lastWasLF = 0;
	  if (x > lcd0.width-7) {
	    x = 0;
	    y += 8;
	  }
	  if (y <= lcd0.height-8) {
	    LcdTextOutXY(x, y, tc);
	    x += 7;
	  }
	}
      }
      if (!lastWasLF) {
	LcdFilledRectangle(x, y, x+7, y+7, NULL, lcd0.backgroundColor);
	x += 7;
      }
    }
    p += strlen(p)+1;
  }

  lcd0.x = x;
  lcd0.y = y;

  retVal = S_OK;
 finally:
  return retVal;
}

#if 0
void fini(void) {
	printf("Library Unloading.\n");
}
#endif
