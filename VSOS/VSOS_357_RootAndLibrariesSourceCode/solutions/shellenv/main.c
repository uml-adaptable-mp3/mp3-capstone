/* For free support for VSIDE, please visit www.vsdsp-forum.com */

// Starting point template for creating VSOS3 applications.
// This will create a <projectname>.AP3 file, which you can copy to 
// your VS1005 Developer Board and run it from there.

// AP3 files require VSOS3 kernel 0.3x to run.

// If you rename your application to INIT.AP3, then the kernel will
// load it automatically after booting.

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <consolestate.h>
#include <kernel.h>
#include <string.h>
#include <mutex.h>
#include <imem.h>

struct ShellEnv {
  u_int32 fopenIMem;
  VO_FILE *(*fopenOld)(const char *filename, const char *mode);
  char *currentDirectoryOld;
  u_int16 curDirFOpenMutex; /* InitMutex(1) */
  char currentDirectoryBuf[256];
} shellEnv;


VO_FILE *CurDirFOpen(const char *filename, const char *mode) {
  VO_FILE *fp;

  if (filename[1] == ':') {
    if (filename[2] == '/' || filename[2] == '\\') {
      strcpy(&filename[2], &filename[3]);
    }
    fp = shellEnv.fopenOld(filename, mode);
  } else {
    int cdLen = strlen(currentDirectory);
    char *cdEndP = currentDirectory+cdLen;

    ObtainMutex(&shellEnv.curDirFOpenMutex);
    strncpy(cdEndP, filename, (sizeof(shellEnv.currentDirectoryBuf)-1)-cdLen);
    fp = shellEnv.fopenOld(currentDirectory, mode);
    *cdEndP = '\0';
    ReleaseMutex(&shellEnv.curDirFOpenMutex);
  }

  return fp;
}

void init(void) {
  shellEnv.curDirFOpenMutex = 1;
  shellEnv.currentDirectoryOld = currentDirectory;
  currentDirectory = shellEnv.currentDirectoryBuf;
  shellEnv.fopenIMem = ReadIMem((u_int16)vo_fopen);
  shellEnv.fopenOld = (void *)(shellEnv.fopenIMem>>6);
  SetHandler(vo_fopen, CurDirFOpen); 	
}
 
void fini(void) {
  currentDirectory = shellEnv.currentDirectoryOld;
  WriteIMem((u_int16)vo_fopen, shellEnv.fopenIMem);
}
