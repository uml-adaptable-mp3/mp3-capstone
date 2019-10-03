#include <vo_stdio.h>
#include <vsos.h>
#include <string.h>
#include <unistd.h>
#include <vo_fat.h>
#include <vo_fatdirops.h>
#include <audio.h>
#include <timers.h>
#include <sysmemory.h>
#include <copy.h>

#define MIN(a,b) (((a)<(b))?(a):(b))


#define FILE_NAME_CHARS 256
#define MAX_DEPTH 8

ioresult CopyRec(const char *d, const char *s) {
  FILE *sDirFp=NULL, *dDirFp=NULL;
  int t;
  ioresult res = S_ERROR;
  char *sName = malloc(FILE_NAME_CHARS);
  char *sPath = malloc(FILE_NAME_CHARS);
  char *dPath = malloc(FILE_NAME_CHARS);
  s_int16 dirIdx[MAX_DEPTH];
  static s_int16 currDepth;

  currDepth = 0;
  dirIdx[0]=1;

  if (!sName || !sPath || !dPath) {
    res = E_OUT_OF_MEMORY;
    goto finally;
  }

  /* It is important to potentially make the destination directory
     before trying to find the source directory. Otherwise file
     traversing functions may become out of sync, and random,
     sometimes horrible things will happen. */
  t = SetDirectory(&dDirFp, d);
  if (t) { /* Directory doesn't exist: try to create it */
    t = mkdir(d);
#if 0
    fprintf(stderr, "MAKE DEST DIR d=%s, t = %d\n", d, t);
#endif
    if ((t != S_OK && t != E_FILE_ALREADY_EXISTS) || SetDirectory(&dDirFp, d)) {
      res = E_FILE_NOT_FOUND;
      goto finally;
    }
  }

#if 0
  fprintf(stderr, "\n\nSetDir(%s)\n", s);
#endif
  t = SetDirectory(&sDirFp, s);
  if (t) {
    res = E_FILE_NOT_FOUND;
    goto finally;
  }

#if 0
  fprintf(stderr, "SetDir(\"%s\") = %d\n", s, t);
#endif
  ResetFindFile(sDirFp);
  while (currDepth >= 0) {
    while (FindFile(sDirFp, sName, FILE_NAME_CHARS, NULL, dirIdx[currDepth],
		    etDir) >= 0) {
#if 0
      fprintf(stderr,
	      "Found dI[%d]=%d DIR  \"%-12s\" \"%s\"\n",
	      currDepth, dirIdx[currDepth], sName, sName+strlen(sName)+1);
#endif
      if (strcmp(sName, ".") && strcmp(sName, "..")) {
#if 0
	fprintf(stderr, "Deeper %d:%s\n", currDepth+1, sName);
#endif
	if (++currDepth >= MAX_DEPTH) {
	  res = S_ERROR;
	  goto finally;
	}
	dirIdx[currDepth]=0;
	SetDirectory(&sDirFp, sName);
	ResetFindFile(sDirFp);
	strcpy(dPath, GetDirectory(&dDirFp));
	strcat(dPath, sName);
	t = mkdir(dPath);
	if ((t != S_OK && t != E_FILE_ALREADY_EXISTS) ||
	    SetDirectory(&dDirFp, dPath)) {
	  res = E_FILE_NOT_FOUND;
	  goto finally;
	}
#if 0
	fprintf(stderr, " # Dest   dir %s\n # Source dir %s\n",
		GetDirectory(&dDirFp), GetDirectory(&sDirFp));
#endif
      }
      dirIdx[currDepth]++;
    }
    ResetFindFile(sDirFp);
    while (FindFile(sDirFp, sName, FILE_NAME_CHARS, NULL, ofmNext, etFile)>=0) {
      u_int32 copyTime;
#if 0
      fprintf(stderr,
	      "      File \"%-12s\" \"%s\"\n", sName, sName+strlen(sName)+1);
#endif
      strcpy(dPath, GetDirectory(&dDirFp));
      strcat(dPath, sName);
      strcpy(sPath, GetDirectory(&sDirFp));
      strcat(sPath, sName);
      copyTime = ReadTimeCount();
      t = CopyFile(dPath, sPath);
      copyTime = ReadTimeCount()-copyTime;
      if (t) {
	res = t;
	goto finally;
      }
    }
#if 0
    fprintf(stderr, "Shallower %d\n", currDepth-1);
#endif
    currDepth--;
    dirIdx[currDepth]++;
    SetDirectory(&sDirFp, "..");
    ResetFindFile(sDirFp);
    SetDirectory(&dDirFp, "..");
    ResetFindFile(dDirFp);
  }


  res = S_OK;
 finally:
  EndDirectory(&sDirFp);
  EndDirectory(&dDirFp);

  if (dPath) {
    free(dPath);
    dPath = NULL;
  }

  if (sPath) {
    free(sPath);
    sPath = NULL;
  }

  if (sName) {
    free(sName);
    sName = NULL;
  }

  return res;
}
