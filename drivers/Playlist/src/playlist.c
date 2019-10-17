#include <vo_stdio.h>
#include <volink.h>
#include <vstypes.h>
#include <stdlib.h>
#include <sysmemory.h>
#include <string.h>

#include "playlist.h"

Playlist* create_new_playlist() {
    Playlist* p_list = (Playlist*) malloc(sizeof(Playlist));
    p_list->head = NULL;
    p_list->current = NULL;
    p_list->last = NULL;
    p_list->length = 0;
    return p_list;
}

void destroy_playlist(Playlist** ph_playlist) {
    Playlist_Entry* p_temp = NULL;
    Playlist_Entry* p_next = NULL;

    if (ph_playlist == NULL) {
        return;
    }

    p_next = (*ph_playlist)->head;

    // free each node
    while (p_next != NULL) {
        p_temp = p_next;
        p_next = p_next->next;

        // free memory
        free(p_temp->filename);
        free(p_temp);
    }
    // free the whole list
    free(*ph_playlist);
    *ph_playlist = NULL;
}

void add_song(Playlist* h_playlist, const char* filename) {
    // add a song to the end of the list
    Playlist_Entry* p_new_node;

    // check that this is a valid playlist
    if (h_playlist == NULL) {
        printf("ERROR: Cannot add songs to an uninitialized playlist.\n");
        return;
    }

    // make a new node in the list
    p_new_node = (Playlist_Entry*) malloc(sizeof(Playlist_Entry));
    if (p_new_node == NULL) {
        // malloc failed, error!
        printf("ERROR: Failed to allocate memory for new song in playlist.\n");
        return;
    }

    // allocate space for the filename
    p_new_node->filename = (char*) malloc((strlen(filename) + 1) * sizeof(char));
    if (p_new_node->filename == NULL) {
        // malloc failed, error!
        printf("ERROR: Failed to allocate memory for new song in playlist.\n");
        return;
    }

    // copy the string to the new space
    strcpy(p_new_node->filename, filename);

    // insert it at the end of the list
    p_new_node->next = NULL;
    if (h_playlist->head != NULL) {
        // list is not empty, so add it to the end and update pointers
        p_new_node->prev = h_playlist->last;
        h_playlist->last->next = p_new_node;
        h_playlist->last = p_new_node;
    }
    else {
        // list is empty, so initialize all of the pointers
        p_new_node->prev = NULL;
        h_playlist->head = p_new_node;
        h_playlist->current = p_new_node;
        h_playlist->last = p_new_node;
    }
    h_playlist->length++;
}

void delete_song(Playlist* h_playlist, Playlist_Entry* p_entry) {
    // check that this is a valid playlist
    if (h_playlist == NULL) {
        printf("ERROR: Cannot delete songs from an uninitialized playlist.\n");
        return;
    }

    if (p_entry == NULL) {  // cannot free NULL
        return;
    }

    // make the previous node's next skip p_entry (if there is a previous entry)
    if (p_entry->prev != NULL) {
        p_entry->prev->next = p_entry->next;
    }

    // make the next node's prev skip p_entry (if there is a next entry)
    if (p_entry->next != NULL) {
        p_entry->next->prev = p_entry->prev;
    }

    // free memory
    free(p_entry->filename);
    free(p_entry);
    p_entry = NULL;

    // update length
    h_playlist->length--;
}
