#include <vo_stdio.h>
#include <vsos.h>
#include "unistd.h"
#include "vo_fatdirops.h"
#include <vo_fat.h>
#include <swap.h>
#include <stdlib.h>
#include <string.h>
#include <sysmemory.h>
#include <ByteManipulation.h>
#include <time.h>

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
  DEVICE *dirDev;
  time_t tt;

  if (!pathOnly || !sectorBuf) {
    res = E_OUT_OF_MEMORY;
    goto finally;
  }

  memcpy(pathOnly, pathname, pathLen+1);
  nameOnly = FindLastSlash(pathOnly);
  if (!nameOnly) {
    if (pathOnly[1] == ':') {
      memmove(pathOnly+2, pathOnly+1, pathLen-1);
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
  dirDev = dirFp->dev;

  dirDev->BlockRead(dirDev, dirFi->directoryEntrySector, 1,
		    sectorBuf);
  dirEnt = (DirectoryEntry *)sectorBuf+dirFi->directoryEntryNumber;
  dirDirectoryEntrySector = dirFi->directoryEntrySector;
#if 0
  fprintf(stderr, "dirDirectoryEntrySector %ld, dirEntNum %d\n",
	  dirDirectoryEntrySector, dirFi->directoryEntryNumber);
#endif

  if ((tt = time(NULL)) != TIME_NOT_FOUND && tt != TIME_NOT_SET) {
    struct tm *tm = localtime(&tt);
    u_int16 dat = ((tm->tm_year-80)<<9) | ((tm->tm_mon+1)<<5) | tm->tm_mday;
    u_int16 tim = (tm->tm_hour<<11) | (tm->tm_min<<5) | (tm->tm_sec>>1);

    dots[0].writeDate = dots[0].lastAccessDate = dots[0].creatDate =
      Swap16(dat);
    dots[0].writeTime = dots[0].creatTime = Swap16(tim);
    memcpy((u_int16 *)(dots+1)+0xe/2, (u_int16 *)(dots)+0xe/2, 6);
  }

  {
    u_int32 t32 = Swap32(dirFi->startCluster);
    dots[0].firstClusLo = (u_int16)(t32 >> 16);
    dots[0].firstClusHi = (u_int16)t32;
    if (!isRoot) {
      t32 = Swap32(mainStartCluster);
      dots[1].firstClusLo = (u_int16)(t32 >> 16);
      dots[1].firstClusHi = (u_int16)t32;
    } else {
      dots[1].firstClusLo = 0;
      dots[1].firstClusHi = 0;
    }
  }

  memset(sectorBuf, 0, 256);
  fwrite(dots, sizeof(dots), 1, dirFp);
  fwrite(sectorBuf, 256-sizeof(dots), 1, dirFp); /* First block is written */

  {
    FatDeviceInfo *fatDi = (FatDeviceInfo *)(dirDev->deviceInfo);
    i = fatDi->fatSectorsPerCluster;
  }
  while (--i) { /* e.g. if 8 sectors per cluster, write 7 empty blocks more */
    fwrite(sectorBuf, 256, 1, dirFp);
  }

  if (dirFp) {
    fclose(dirFp);
    dirFp = NULL;
    dirFi = NULL;
  }

  dirDev->BlockRead(dirDev, dirDirectoryEntrySector, 1, sectorBuf);
  /* Update to directory status */
  dirEnt->attr |= __ATTR_DIRECTORY;
  /* Force size to 0 */
  dirEnt->fileSizeHi = dirEnt->fileSizeLo = 0;
  dirDev->BlockWrite(dirDev, dirDirectoryEntrySector, 1, sectorBuf);
  /* Force flush operation */
  dirDev->BlockRead(dirDev, 0, 0, NULL);

  res = S_OK;
 finally:
#if 0
  if (dirFp) {
    fclose(dirFp);
    dirFp = NULL;
  }
#endif

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
