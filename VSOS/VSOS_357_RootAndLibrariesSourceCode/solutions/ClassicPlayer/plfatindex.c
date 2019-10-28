/// \file plfatindex.c Indexed FAT Playlist

/// Functionality for playlist which only needs 3 words of memory per playlist entry!
/// With just that amount of data it can locate, open (and identify roughly) a file on a FAT disk.
/// (it stores the disk sector and byte offset of the directory entry of the file)
/// This allows playlists that can be modified in realtime while music is playing in the background.
/// (there's not much of a playlist editor yet - the one in Classic Player 0.1 just forms playlists from complete folders)

/// \author Panu-Kristian Poiksalo, VLSI Solution Oy


#include <vo_stdio.h>
#include "plfatindex.h"
#include <swap.h>
#include <uimessages.h>
#include <abstract_playlist.h>
#include <string.h>
#include "usemodel.h"
#include "lcdstuff.h"


DirEntryInfo files[PLAYLIST_ENTRIES];
u_int16 nFiles;
char path[256];
char currentFileName[255];
s_int16 currentSongNumber;

s_int16 PLGetSize(register struct ABSTRACT_PLAYLIST*);
u_int16 PLGetName(register struct ABSTRACT_PLAYLIST*,register s_int16 itemIndex,register wchar* buffer,register s_int16 bufferSz);
FILE *PLOpen(register struct ABSTRACT_PLAYLIST*,register s_int16 itemIndex);
void PLDelete(register struct ABSTRACT_PLAYLIST*);

ABSTRACT_PLAYLIST plFatIndex = {
	PLGetSize,
	PLGetName,
	PLOpen,
	PLDelete,
	PF_AUTOADVANCE | PF_SIZE_CHANGED
};

s_int16 PLGetSize(register struct ABSTRACT_PLAYLIST *pl) {
	// since we have only 1 playlist for both browsing and playing, we should return playlist size zero
	// if we're in the file browser dialog - this hack needs lcdstuff.h
	if (currentScreen != NORMAL_SCREEN) return 0; 
	return nFiles;
}

u_int16 PLGetName(register struct ABSTRACT_PLAYLIST *pl, register s_int16 itemIndex, 
				  register wchar* buffer, register s_int16 bufferSz) {
	u_int16 n;
	FILE *f;
	if (itemIndex > nFiles) return 0;
	f = OpenFileByDirEntryInfo(path, &files[itemIndex-1]);
	fclose(f);
	buffer[0]=0;
	strncpy(buffer,currentFileName,bufferSz-1);
	return (!files[itemIndex-1].flags & DIR_FLAG_DIR);
}
	
FILE *PLOpen(register struct ABSTRACT_PLAYLIST *pl,register s_int16 itemIndex) {
	if (itemIndex > nFiles) return NULL;
	return OpenFileByDirEntryInfo(path, &files[itemIndex-1]);
}


void PLDelete(register struct ABSTRACT_PLAYLIST* pl) {

}



FILE *OpenFileByDirEntryInfo(register char *driveLetterAndColon, register DirEntryInfo *e) {
	FILE *f = fopen(driveLetterAndColon,"s");
	DirectoryEntry *entry = (DirectoryEntry*)(&f->sectorBuffer[e->flags>>1]);
	f->currentSector = e->dirSector;
	f->dev->BlockRead(f->dev,e->dirSector,1,f->sectorBuffer);
	f->pos = e->flags+32;
	FatNameFromDir(entry,currentFileName); 
	FatOpenEntry(f,entry);
	if (e->flags & DIR_FLAG_DIR) {
		f->flags = __MASK_FILE | __MASK_OPEN | __MASK_PRESENT;
	} else {
		f->flags = __MASK_FILE | __MASK_OPEN | __MASK_READABLE | __MASK_SEEKABLE | __MASK_PRESENT;
	}
	return f;
}

void ID3InfoCallBack(s_int16 index, u_int16 message, u_int32 value) {
		if ((message) == UIMSG_TEXT_SONG_NAME) {
		strncpy(currentFileName, (char*)value, sizeof(currentFileName)-1); 
	}
}
char *GetPlayListItemInfo(u_int16 songNumber) {
	FILE *f;
	if (songNumber > nFiles) return NULL;
	f = OpenFileByDirEntryInfo(path,&files[songNumber-1]);
	if (f) {
		model.DecodeID3(f, ID3InfoCallBack);
		fclose(f);
		return currentFileName;
	}
	return NULL;
}









/*

/// Returns the size of a file to which a FAT search handle is currently pointing to
u_int32 FoundFileSize(VO_FILE *hSearch) {
   DirectoryEntry *de = &hSearch->sectorBuffer[((hSearch->pos-32) >> 1) & 0xff];
   return Swap32Mix(*(u_int32*)(&de->fileSizeLo));
}
void PrintSearchInfo(FILE *f) {
	//fprintf(vo_stderr, "Name: %-20s ",filename);
	fprintf(vo_stderr, "dir:%ld ",f->currentSector);
	fprintf(vo_stderr, ".%ld ",(f->pos-32)&511);
	fprintf(vo_stderr, "Attr: %02x ",f->ungetc_buffer);
	fprintf(vo_stderr, "Short: %-13s ",f->extraInfo);
	fprintf(vo_stderr, "Extension: '%s' ",&f->extraInfo[13]);
	fprintf(vo_stderr, "Size: %ld ",FoundFileSize(f));
	fprintf(vo_stderr, "\n");
}

*/


