#ifndef ABSTRACT_PLAYLIST_H
#define ABSTRACT_PLAYLIST_H
//
#include <vstypes.h>
#include <vo_stdio.h>
//
#ifndef STDWIDGET_H
typedef unsigned wchar; // default character format
#endif
//
typedef s_int16 (*PLSIZEMETHOD)(register struct ABSTRACT_PLAYLIST*);
typedef u_int16 (*PLNAMEMETHOD)(register struct ABSTRACT_PLAYLIST*,register s_int16 itemIndex,register wchar* buffer,register s_int16 bufferSz);
typedef FILE* (*PLOPENMETHOD)(register struct ABSTRACT_PLAYLIST*,register s_int16 itemIndex);
typedef void (*PLRELEASEMETHOD)(register struct ABSTRACT_PLAYLIST*); 
//
// playlist flags (note that it is not guaranteed that all playlists support all of the flags)
//
#define PF_SHUFFLE		(1<<0) // the order of the list should be shuffled
#define PF_LOOPING		(1<<1) // loop from end to the beginning of the list and vice versa
#define PF_AUTOADVANCE	(1<<2) // start next track automatically
#define PF_SIZE_CHANGED 	(1<<3) // a hint to query size again because list has changed
//
typedef struct ABSTRACT_PLAYLIST {
	PLSIZEMETHOD Size; 	// Returns the number of elements in the list. This operation may be slow!
	PLNAMEMETHOD Name; 	// Obtain a list item's (decorative) filename (when applicable)
							// Returns zero if selected file is a directory, non-zero otherwise
							// (If file does not exist, empty string is written to the buffer.)
	PLOPENMETHOD Open;	// Returns a file pointer to specified element of the list (or NULL)
							// NOTE: remember to fclose() the handle after its no longer needed!
	PLRELEASEMETHOD Delete; // Delete the list object. Always call when the list is no longer needed.
	u_int16 flags; // PF_* (some flags may be used internally!)
} ABSTRACT_PLAYLIST;
//
typedef u_int16 (*CommonPlaylistConstructor)(register void*,register wchar*);
#define CreateCommonPlaylist(lib,playlist,path) (*(((CommonPlaylistConstructor*)(lib))+2+ENTRY_1))(playlist,path)


#endif