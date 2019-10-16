#ifndef M3U_PLAYLIST_H
#define M3U_PLAYLIST_H

#include "abstract_playlist.h"

//
// M3U playlist traverses the contents of a M3U playlist (ASCII text file)
//
typedef struct M3U_PLAYLIST {
	ABSTRACT_PLAYLIST a; // inherit abstract playlist
	// (fields below are for internal use -- do not manipulate directly)
	FILE* handle;
	s_int16 current;
} M3U_PLAYLIST;

#endif