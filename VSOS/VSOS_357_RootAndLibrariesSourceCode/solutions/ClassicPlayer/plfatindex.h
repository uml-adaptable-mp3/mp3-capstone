#ifndef PLFATINDEX_H
#define PLFATINDEX_H

#include <volink.h>
#include <vo_stdio.h>
#include <vo_fat.h>
#include <abstract_playlist.h>

#define PLAYLIST_ENTRIES 100
#define NAMELENGTH 20 //leaves 2 characters for long name.

typedef struct DirEntryInfoStruct {
	u_int32 dirSector;
	s_int16 flags;
	u_int16 order;	
} DirEntryInfo;


ioresult FatOpenEntry(register __i0 VO_FILE *f, DirectoryEntry *de); //Import kernel symbol
DLLIMPORT(FatNameFromDir) 
char *FatNameFromDir(const u_int16 *dirEntry, char *longName);
DLLIMPORT(FatOpenEntry) 

#define DIR_FLAG_DIR (1<<0)

extern DirEntryInfo files[PLAYLIST_ENTRIES];
extern u_int16 nFiles;
extern char path[256];
extern char currentFileName[255];
extern s_int16 currentSongNumber;
extern ABSTRACT_PLAYLIST plFatIndex; 

FILE *OpenFileByDirEntryInfo(register char *driveLetterAndColon, register DirEntryInfo *e);
char *GetPlayListItemInfo(u_int16 plIndex);
#endif