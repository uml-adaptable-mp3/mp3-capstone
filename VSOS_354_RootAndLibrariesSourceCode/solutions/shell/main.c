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
#include <string.h>
#include <kernel.h>
#include <mutex.h>
#include <imem.h>

#if 1
#define NEVER_EXITS
#endif

#define CMD_LINE_CHARS 256

struct ShellEnv {
#ifndef NEVER_EXITS
  u_int32 fopen_save;
#endif
  VO_FILE *(*old_fopen)(const char *filename, const char *mode);
  u_int16 curDirFOpenMutex; /* InitMutex(1) */
  char cmd[CMD_LINE_CHARS];
  char currentDirectoryBuf[128];
} shellEnv;


VO_FILE *CurDirFOpen(const char *filename, const char *mode) {
  int cdLen;
  char *cdEndP, *p;
  VO_FILE *fp;

  ObtainMutex(&shellEnv.curDirFOpenMutex);

  cdLen = strlen(currentDirectory);
  cdEndP = currentDirectory+cdLen;
  p = filename;

  if (filename[1] == ':') {
    if (filename[2] == '/' || filename[2] == '\\') {
      strcpy(&filename[2], &filename[3]);
    }
  } else {
    strncpy(cdEndP, filename, (sizeof(shellEnv.currentDirectoryBuf)-1)-cdLen);
    p = currentDirectory;
  }
  fp = shellEnv.old_fopen(p, mode);
  *cdEndP = '\0';

  ReleaseMutex(&shellEnv.curDirFOpenMutex);

  return fp;
}


#ifndef NEVER_EXITS
void init(void) {
  shellEnv.fopen_save = ReadIMem((u_int16)vo_fopen);
  shellEnv.old_fopen = (void *)((u_int16)(shellEnv.fopen_save>>6L));
  SetHandler(vo_fopen, CurDirFOpen); 	
}
#endif

ioresult shell_main(char *parameters) {
#ifdef NEVER_EXITS
  shellEnv.old_fopen = (void *)((u_int16)(ReadIMem((u_int16)vo_fopen)>>6L));
  SetHandler(vo_fopen, CurDirFOpen); 	
#endif
  shellEnv.curDirFOpenMutex = 1;
  shellEnv.cmd[0] = CMD_LINE_CHARS;


  currentDirectory = shellEnv.currentDirectoryBuf;
  appFlags = APP_FLAG_ECHO;	

  strcpy(currentDirectory, "S:");
  printf("\nVSOS SHELL\n");

  RunProgram("PLAYFILES", "");

  return S_OK;
}


#ifndef NEVER_EXITS
void fini(void) {
  WriteIMem((u_int16)vo_fopen, fopen_save);
}
#endif
