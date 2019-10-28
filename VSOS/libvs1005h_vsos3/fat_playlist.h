#ifndef FAT_PLAYLIST_H
#define FAT_PLAYLIST_H

#include "abstract_playlist.h"
//
// FAT playlist traverses files stored on a FAT directory entry (no particular order is guaranteed!)
//
typedef struct FAT_PLAYLIST {
	ABSTRACT_PLAYLIST a; // inherit abstract playlist
//private:
	FILE* handle;
	s_int16 current;
	u_int16 attr;
	char* path;
	char* cfn;
} FAT_PLAYLIST;

#endif