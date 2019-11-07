// Test Code for Playlist

#include <vo_stdio.h>
#include <volink.h> // Linker directives like DLLENTRY
#include <vstypes.h>
#include <stdlib.h>
#include <string.h>

#include "playlist.h"

void test_playlist();
void test_shuffle();

void main() {
    test_playlist();
    test_shuffle();
}


void test_playlist() {
    Playlist* h_my_playlist;
    Playlist_Entry* temp;
    printf("\n\nTesting Playlist functions...\n");

    printf("Making new playlist\n");
    h_my_playlist = create_new_playlist();
    printf("Adding song1.mp3\n");
    add_song(h_my_playlist, "song1.mp3");
    printf("Adding song2.mp3\n");
    add_song(h_my_playlist, "song2.mp3");
    printf("Adding song3.mp3\n");
    add_song(h_my_playlist, "song3.mp3");

    while (h_my_playlist->current != NULL) {
        printf("Song in list: %s\n", h_my_playlist->current->filename);
        h_my_playlist->current = h_my_playlist->current->next;
    }
    h_my_playlist->current = h_my_playlist->head;

    printf("Length of list: %d\n", h_my_playlist->length);

    while (h_my_playlist->current != NULL) {
        printf("Removing %s from list...\n", h_my_playlist->current->filename);
        temp = h_my_playlist->current->next;
        delete_song(h_my_playlist, h_my_playlist->current);
        h_my_playlist->current = temp;
    }
    h_my_playlist->current = h_my_playlist->head;

    printf("Length of list: %d\n", h_my_playlist->length);

    printf("Adding song1.mp3\n");
    add_song(h_my_playlist, "song1.mp3");
    printf("Adding song2.mp3\n");
    add_song(h_my_playlist, "song2.mp3");
    printf("Adding song3.mp3\n");
    add_song(h_my_playlist, "song3.mp3");

    printf("Length of list: %d\n", h_my_playlist->length);

    printf("Deleting playlist...\n");

    destroy_playlist(&h_my_playlist);
}

void test_shuffle() {
    Playlist* h_playlist;
    int i;
    printf("\n\nTesting Playlist Shuffle...\n");

    printf("Making new playlist\n");
    h_playlist = create_new_playlist();
    // add a bunch of song names
    add_song(h_playlist, "song1.mp3");
    add_song(h_playlist, "song2.mp3");
    add_song(h_playlist, "song3.mp3");
    add_song(h_playlist, "song4.mp3");
    add_song(h_playlist, "song5.mp3");
    add_song(h_playlist, "song6.mp3");
    add_song(h_playlist, "song7.mp3");
    add_song(h_playlist, "song8.mp3");
    add_song(h_playlist, "song9.mp3");
    add_song(h_playlist, "song10.mp3");

    printf("Size before: %d\n", h_playlist->length);
    printf("Contents of Playlist Before:\n");
    h_playlist->current = h_playlist->head;
    i = 0;
    while (h_playlist->current != NULL) {
        printf("%d: %s\n", i, h_playlist->current->filename);
        h_playlist->current = h_playlist->current->next;
        ++i;
    }
    printf("\nBeginning Shuffle...\n");
    shuffle_playlist(h_playlist);
    printf("\nSize after: %d\n", h_playlist->length);
    printf("Contents of Playlist After:\n");
    h_playlist->current = h_playlist->head;
    i = 0;
    while (h_playlist->current != NULL) {
        printf("%d: %s\n", i, h_playlist->current->filename);
        h_playlist->current = h_playlist->current->next;
        ++i;
    }
    printf("Test complete.");
    destroy_playlist(&h_playlist);
}
