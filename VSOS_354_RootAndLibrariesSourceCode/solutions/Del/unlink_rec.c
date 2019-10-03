#include <vo_stdio.h>
#include <unistd.h>
#include <string.h>
#include <vo_fat.h>
#include <vo_fatdirops.h>
#include <consolestate.h>
#include <apploader.h>

#define NAMELENGTH 32
char fileName[NAMELENGTH];

#define PATHLENGTH 256
char currPath[PATHLENGTH];
char origDir[PATHLENGTH];

/* unlink returns 0 on success, -1 on failure */
int unlink_recursive(register const char *pathname, register s_int16 verbose) {
  char *p;
  static char s[PATHLENGTH];
  int res = -1;
  FILE *f = NULL;

  strcpy(origDir, currentDirectory);

  sprintf(s, "+v %s", pathname);
  if (!RunProgram("cd", s)) {
    int depth=0;
    if (strlen(currentDirectory) <= 2) {
      printf("E: Cannot delete root directory\n");
      goto finally;
    }
    f = fopen(currentDirectory, "s");
    if (!f) {
      printf("E: Cannot read directory\n");
      goto finally;
    }
    while (depth >= 0) {
      int cancel = 0;
      if (FatFindFirst(f, currentDirectory+2, currPath, PATHLENGTH-1) == S_OK) {
#if 0
	printf("Entries in %s\n",
	       f->dev->Identify(f->dev,NULL,0));
#endif
	do {
	  if (!strcmp(f->extraInfo, ".") || !strcmp(f->extraInfo, "..")) {
	    /* Do nothing */
	  } else if (f->ungetc_buffer & __ATTR_DIRECTORY) {
	    sprintf(s, "+v %s%s", currentDirectory, f->extraInfo);
	    if (RunProgram("cd", s)) {
	      printf("E: Cannot enter directory %s\n", s+3);
	      goto finally;
	    }
	    depth++;
	    cancel = 1;
#if 0
	    printf("New dir %s, depth %d\n", currentDirectory, depth);
#endif
	  } else {
	    FILE *fp;

	    sprintf(s, "%s%s", currentDirectory, f->extraInfo);
	    if (verbose) printf("Del %s\n", s);

	    fp = fopen(s, "U");
	    if (!fp) {
	      goto finally;
	    }
	    fclose(fp);

#if 0
	    printf("DELETE#1 %s", s);
#endif
	  }
#if 0
	  printf(" attr 0x%04x, shortName %s\n",
		 f->ungetc_buffer, f->extraInfo);
#endif
	} while (!cancel && FatFindNext(f, currPath, PATHLENGTH-1) == S_OK);
      }
      if (!cancel) {
	FILE *fp;
	strcpy(s, currentDirectory);
	s[strlen(s)-1] = '\0';
#if 0
	printf("DELETE#2 %s\n", s);
#endif

	if (verbose) printf("Del %s\n", s);
	fp = fopen(s, "U");
	if (!fp) {
	  goto finally;
	}
	fclose(fp);

	if (--depth >= 0) {
	  if (RunProgram("cd", "..")) {
	    printf("E: Cannot enter parent directory for %s\n", currentDirectory);
	    goto finally;
	  }
#if 0
	  printf("Up1 dir %s, depth %d\n", currentDirectory, depth);
#endif
	}
      }
    }
    fclose(f);
    f = NULL;
  } else {
    FILE *fp = fopen(pathname, "U");
    if (!fp) {
      goto finally;
    }
    fclose(fp);
  }

  res = 0;

 finally:
  if (f) {
    fclose(f);
    f = NULL;
  }
  strcpy(currentDirectory, origDir);

  return res;
}
