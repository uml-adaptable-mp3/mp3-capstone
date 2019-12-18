#ifndef _PLAYLIST_H_
#define _PLAYLIST_H_

#include <vstypes.h>
#include <sysmemory.h>

__mem_y struct Playlist_Entry {
    char __mem_y *filename;
    __mem_y struct Playlist_Entry *next;
    __mem_y struct Playlist_Entry *prev;
};

typedef __mem_y struct Playlist_Entry Playlist_Entry;

typedef __mem_y struct {
    Playlist_Entry __mem_y *head;
    Playlist_Entry __mem_y *current;
    Playlist_Entry __mem_y *last;
    u_int16 __mem_y length;
    char __mem_y title[50];
} Playlist;

Playlist __mem_y* create_new_playlist();
Playlist __mem_y* create_playlist_from_file(register const char* filename);
void destroy_playlist(Playlist __mem_y* __mem_y* ph_playlist);

// Playlist Methods
void add_song(Playlist __mem_y* h_playlist, const char* filename);
void delete_song(Playlist __mem_y* h_playlist, Playlist_Entry __mem_y* p_entry);
void shuffle_playlist(Playlist __mem_y* h_playlist);

#endif  // _PLAYLIST_H_
