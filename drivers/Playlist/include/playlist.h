#ifndef _PLAYLIST_H_
#define _PLAYLIST_H_

struct Playlist_Entry {
    char* filename;
    struct Playlist_Entry* next;
    struct Playlist_Entry* prev;
};

typedef struct Playlist_Entry Playlist_Entry;

typedef struct {
    Playlist_Entry* head;
    Playlist_Entry* current;
    Playlist_Entry* last;
    u_int16 length;
    char title[50];
} Playlist;

Playlist* create_new_playlist();
Playlist* create_playlist_from_file(register const char* filename);
void destroy_playlist(Playlist** ph_playlist);

// Playlist Methods
void add_song(Playlist* h_playlist, const char* filename);
void delete_song(Playlist* h_playlist, Playlist_Entry* p_entry);
void shuffle_playlist(Playlist* h_playlist);

#endif  // _PLAYLIST_H_
