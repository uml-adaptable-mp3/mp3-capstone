#include <vo_stdio.h>
#include <volink.h>
#include <vstypes.h>
#include <stdlib.h>
#include <sysmemory.h>
#include <string.h>
#include <time.h>
#include <audio.h>

#include "playlist.h"

#define BUFFER_SIZE 127
#define NULL_Y (void __mem_y*) NULL

static u_int16 sg_SEEDED = 0;

static void seed_random(void) {
    if (!sg_SEEDED) {
        srandom((u_int32) ReadTimeCount());
        sg_SEEDED = 1;
    }
}

Playlist __mem_y* create_new_playlist() {
    Playlist __mem_y* p_list = (Playlist __mem_y*) mallocy(sizeof(Playlist));
    p_list->head = NULL_Y;
    p_list->current = NULL_Y;
    p_list->last = NULL_Y;
    p_list->length = 0;
    memcpyXY((char __mem_y*) p_list->title, "Untitled Playlist", 18);
    p_list->title[18] = '\0';
    // strcpy(p_list->title, "Untitled Playlist");
    return p_list;
}

Playlist __mem_y* create_playlist_from_file(register const char* filename) {
    Playlist __mem_y* h_playlist = (Playlist __mem_y*) NULL_Y;
    FILE *p_file = NULL_Y;
    FILE *p_temp = NULL_Y;
    char name_buffer[BUFFER_SIZE];
    int last_index = 0;
    int dir_offset = 0; // string offset so that the entire string doesn't need to be moved if swapping ../ with D:

    // Open the playlist file
    p_file = fopen(filename, "r");
    if (p_file == NULL_Y) {
        printf("Couldn't open file '%s'\n", filename);
        return NULL_Y;
    }

    h_playlist = create_new_playlist();
    printf("New Playlist:\n");
    printf("Title: %s | length: %d\n", h_playlist->title, h_playlist->length);
    printf("Head: (0x%p) %s\n", h_playlist->head, h_playlist->head->filename);
    printf("current: (0x%p) %s\n", h_playlist->current, h_playlist->current->filename);
    printf("last: (0x%p) %s\n", h_playlist->last, h_playlist->last->filename);

    // parse the file line by line
    while (fgets(name_buffer, BUFFER_SIZE, p_file) != NULL_Y) {
        if (name_buffer[0] == '#') {
            if (strncmp(name_buffer, "#PLAYLIST: ", 11) == 0) {
                // strncpy(h_playlist->title, &name_buffer[11], 50);
                memcpyXY(h_playlist->title, &name_buffer[11], 50);
                printf("Set playlist title to %s\n", h_playlist->title);
            }
            continue;
        }

        // drop trailing newline from filename, if it is present
        last_index = strlen(name_buffer) - 1;
        // printf("The last character in the buffer is: %d\n", name_buffer[last_index]);
        // printf("The second last character in the buffer is: %d\n", name_buffer[last_index - 1]);
        if (name_buffer[last_index] == '\n' || name_buffer[last_index] == '\r') {
            name_buffer[last_index] = '\0';
        }
        if (name_buffer[last_index - 1] == '\n' || name_buffer[last_index - 1] == '\r') {
            name_buffer[last_index - 1] = '\0';
        }

        if (strncmp(name_buffer, "../Music", 8) == 0) {
            name_buffer[1] = filename[0];
            name_buffer[2] = filename[1];
            dir_offset = 1;
        }
        else {
            dir_offset = 0;
        }


        // check if file exists
        p_temp = fopen(&(name_buffer[0+dir_offset]), "r");
        if (p_temp != NULL_Y) {
            // file exists, add it to playlist
            printf("Adding '%s' to playlist...\n", &(name_buffer[0+dir_offset]));
            add_song(h_playlist, &(name_buffer[0+dir_offset]));
            // free the pointer to the file
            fclose(p_temp);
        }
        else {
            printf("File '%s' not found, skipping...\n", &(name_buffer[0+dir_offset]));
        }
        printf("After adding:\n");
        printf("Title: %s | length: %d\n", h_playlist->title, h_playlist->length);
        printf("Head: (0x%p) %s\n", h_playlist->head, h_playlist->head->filename);
        printf("current: (0x%p) %s\n", h_playlist->current, h_playlist->current->filename);
        printf("last: (0x%p) %s\n", h_playlist->last, h_playlist->last->filename);
    }

    return h_playlist;
}

void destroy_playlist(Playlist __mem_y* __mem_y* ph_playlist) {
    Playlist_Entry __mem_y* p_temp = (Playlist_Entry __mem_y*) NULL_Y;
    Playlist_Entry __mem_y* p_next = (Playlist_Entry __mem_y*) NULL_Y;

    if (ph_playlist == NULL_Y) {
        return;
    }

    p_next = (Playlist_Entry __mem_y*) ((Playlist*) *ph_playlist)->head;

    // free each node
    while (p_next != NULL_Y) {
        p_temp = p_next;
        p_next = (Playlist_Entry __mem_y*) p_next->next;

        // free memory
        freey((char __mem_y*) p_temp->filename);
        freey((Playlist_Entry __mem_y*) p_temp);
    }
    // free the whole list
    freey((Playlist*) *ph_playlist);
    *ph_playlist = NULL_Y;
}

void add_song(Playlist __mem_y* h_playlist, const char* filename) {
    // add a song to the end of the list
    Playlist_Entry __mem_y* p_new_node;

    // check that this is a valid playlist
    if (h_playlist == NULL_Y) {
        printf("ERROR: Cannot add songs to an uninitialized playlist.\n");
        return;
    }

    // make a new node in the list
    p_new_node = (Playlist_Entry __mem_y*) mallocy(sizeof(Playlist_Entry));
    if (p_new_node == NULL_Y) {
        // mallocy failed, error!
        printf("ERROR: Failed to allocate memory for new song in playlist.\n");
        return;
    }

    // allocate space for the filename
    p_new_node->filename = (char __mem_y*) mallocy((strlen(filename) + 1) * sizeof(char));
    if (p_new_node->filename == NULL_Y) {
        // mallocy failed, error!
        printf("ERROR: Failed to allocate memory for new song in playlist.\n");
        return;
    }

    // copy the string to the new space
    // strcpy(p_new_node->filename, filename);
    memcpyXY(p_new_node->filename, filename, strlen(filename) + 1);
    printf("=== Song Added to playlist: %s\n", (char __mem_y*) p_new_node->filename);
    // insert it at the end of the list
    p_new_node->next = NULL_Y;
    if (h_playlist->head != NULL_Y) {
        // list is not empty, so add it to the end and update pointers
        printf("Appending to end\n");
        p_new_node->prev = h_playlist->last;
        h_playlist->last->next = (Playlist_Entry __mem_y*) p_new_node;
        h_playlist->last = (Playlist_Entry __mem_y*) p_new_node;
    }
    else {
        // list is empty, so initialize all of the pointers
        printf("adding to beginning\n");
        p_new_node->prev = NULL_Y;
        h_playlist->head = p_new_node;
        h_playlist->current = p_new_node;
        h_playlist->last = p_new_node;
    }
    h_playlist->length++;
    printf("Size is now %d\n", h_playlist->length);
}

void delete_song(Playlist __mem_y* h_playlist, Playlist_Entry __mem_y* p_entry) {
    // check that this is a valid playlist
    if (h_playlist == NULL_Y) {
        printf("ERROR: Cannot delete songs from an uninitialized playlist.\n");
        return;
    }

    if (p_entry == NULL_Y) {  // cannot free NULL_Y
        return;
    }

    // make the previous node's next skip p_entry (if there is a previous entry)
    if (p_entry->prev != NULL_Y) {
        p_entry->prev->next = p_entry->next;
    }

    // make the next node's prev skip p_entry (if there is a next entry)
    if (p_entry->next != NULL_Y) {
        p_entry->next->prev = p_entry->prev;
    }

    // free memory
    freey(p_entry->filename);
    freey((Playlist_Entry __mem_y*) p_entry);
    p_entry = NULL_Y;

    // update length
    h_playlist->length--;
}

void shuffle_playlist(Playlist* h_playlist) {
    int i, j;
    Playlist_Entry __mem_y* __mem_y * p_node_array;
    __mem_y Playlist_Entry* temp;

    seed_random();  // make sure random is seeded

    // make array of pointers to nodes
    p_node_array = mallocy((size_t) h_playlist->length * sizeof(Playlist_Entry*));
    if (p_node_array == NULL_Y) {
        printf("ERROR: Could not shuffle playlist: Out of memory\n");
        return;
    }
    temp = h_playlist->head;
    for (i = 0; i < h_playlist->length; ++i) {
        p_node_array[i] = temp;
        temp = temp->next;
    }

    // perform the Fisher-Yates shuffle algorithm
    for (i = 0; i < h_playlist->length - 1; ++i) {
        // random j such that i <= j < length
        j = ((int) random() % (h_playlist->length - i)) + i;
        // swap i and j
        temp = p_node_array[i];
        p_node_array[i] = p_node_array[j];
        p_node_array[j] = temp;
    }

    // Fix all of the pointers in the list
    // first element in array is head, its previous is NULL_Y
    p_node_array[0]->prev = NULL_Y;
    p_node_array[0]->next = p_node_array[1];
    for (i = 1; i < h_playlist->length - 1; ++i) {
        p_node_array[i]->prev = p_node_array[i-1];
        p_node_array[i]->next = p_node_array[i+1];
    }
    p_node_array[i]->prev = p_node_array[i-1];
    p_node_array[i]->next = NULL_Y;

    // assign the new head and last, reset current to new head
    h_playlist->head = p_node_array[0];
    h_playlist->current = p_node_array[0];
    h_playlist->last = p_node_array[i];

    // free the allocated memory for the array, but NOT the nodes in it
    freey(p_node_array);
}
