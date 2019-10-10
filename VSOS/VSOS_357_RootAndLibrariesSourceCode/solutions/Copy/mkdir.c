#include <vo_stdio.h>
#include <vsos.h>
#include "unistd.h"
#include "vo_fatdirops.h"
#include <vo_fat.h>
#include <swap.h>
#include <stdlib.h>
#include <string.h>
#include <sysmemory.h>

DirectoryEntry dots[2] = {
  {{0x2e20,0x2020,0x2020,0x2020,0x2020},
   0x2010,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
  {{0x2e2e,0x2020,0x2020,0x2020,0x2020},
   0x2010,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0}
};

#define NAMELENGTH 32
char fileName[NAMELENGTH];



int mkdir(const char *pathname) {
  FILE *dirFp = NULL;
  ioresult res = S_ERROR;
  DirectoryEntry *dirEnt;
  FatFileInfo *dirFi;
  u_int32 dirDirectoryEntrySector;
  u_int32 mainStartCluster;
  int i;
  int isRoot = 0;
  int pathLen = strlen(pathname);
  char *pathOnly = calloc(pathLen+2, sizeof(char));
  u_int16 *sectorBuf = malloc(256*sizeof(u_int16));
  char *nameOnly;

  if (!pathOnly || !sectorBuf) {
    res = E_OUT_OF_MEMORY;
    goto finally;
  }

  memcpy(pathOnly, pathname, pathLen+1);
  nameOnly = FindLastSlash(pathOnly);
  if (!nameOnly) {
    if (pathOnly[1] == ':') {
      memmove(pathOnly+2,pathOnly+1,pathLen-1);
      nameOnly = pathOnly+2;
    } else {
      res = E_ILLEGAL_FILE_NAME;
      goto finally;
    }
  }
  *nameOnly++ = '\0';

#if 0
  fprintf(stderr, "pathname \"%s\"\npathOnly \"%s\"\nnameOnly \"%s\"\n",
	  pathname, pathOnly, nameOnly);
#endif

  /* Refuse to create if too short, too long, or contains '.' */
  if (!nameOnly[0] || strlen(nameOnly) > 8 || strchr(nameOnly, '.')) {
    res = E_ILLEGAL_FILE_NAME;
    goto finally;
  }

  /* Gather information from main directory: is it root? */
  {
    FILE *mainFp = NULL;
    FatFileInfo *mainFi;
    if (SetDirectory(&mainFp, pathOnly) || !mainFp) {
      // SetDirectory calls EndDirectory() if it fails, no need to redo that;
      return E_CANNOT_SET_DIRECTORY;
    }
    FatFindFirst(mainFp, pathOnly+2, fileName, NAMELENGTH);
    if (strcmp(mainFp->extraInfo, ".")) {
      isRoot = 1;
    }
    mainFi = (FatFileInfo *)mainFp->fileInfo;
    mainStartCluster = mainFi->startCluster;
    EndDirectory(&mainFp);
  }


  /* Refuse to do anything if already exists */
  dirFp = fopen(pathname, "rb");
  if (dirFp) {
    res = E_FILE_ALREADY_EXISTS;
    goto finally;
  }
  fclose(dirFp);

  /* Try to open for writing */
  dirFp = fopen(pathname, "wb");
  if (!dirFp) {
    res = E_FILE_NOT_FOUND;
    goto finally;
  }
  dirFi = (FatFileInfo *)dirFp->fileInfo;

  dirFp->dev->BlockRead(dirFp->dev, dirFi->directoryEntrySector, 1,
			      sectorBuf);
  dirEnt = (DirectoryEntry *)sectorBuf+dirFi->directoryEntryNumber;
  dirDirectoryEntrySector = dirFi->directoryEntrySector;
#if 0
  fprintf(stderr, "dirDirectoryEntrySector %ld, dirEntNum %d\n",
	  dirDirectoryEntrySector, dirFi->directoryEntryNumber);
#endif

  dots[0].firstClusLo = Swap16((u_int16)dirFi->startCluster);
  dots[0].firstClusHi = Swap16((u_int16)(dirFi->startCluster>>16));
  if (!isRoot) {
    dots[1].firstClusLo = Swap16((u_int16)mainStartCluster);
    dots[1].firstClusHi = Swap16((u_int16)(mainStartCluster>>16));
  } else {
    dots[1].firstClusLo = 0;
    dots[1].firstClusHi = 0;
  }

  memset(sectorBuf, 0, 256);
  fwrite(dots, sizeof(dots), 1, dirFp);
  fwrite(sectorBuf, 256-sizeof(dots), 1, dirFp); /* First block is written */

  {
    FatDeviceInfo *fatDi = (FatDeviceInfo *)(dirFp->dev->deviceInfo);
    i = fatDi->fatSectorsPerCluster;
  }
  while (--i) { /* e.g. if 8 sectors per cluster, write 7 empty blocks more */
    fwrite(sectorBuf, 256, 1, dirFp);
  }

  dirFp->dev->BlockRead(dirFp->dev, dirDirectoryEntrySector, 1,
			       sectorBuf);
  dirEnt->attr |= __ATTR_DIRECTORY;
  dirEnt->fileSizeLo = 0;
  dirFp->dev->BlockWrite(dirFp->dev, dirDirectoryEntrySector, 1,			       sectorBuf);

  res = S_OK;
 finally:
  if (dirFp) {
    fclose(dirFp);
    dirFp = NULL;
  }

  if (pathOnly) {
    free(pathOnly);
    pathOnly = NULL;
  }

  if (sectorBuf) {
    free(sectorBuf);
    sectorBuf = NULL;
  }

  return res;
}
