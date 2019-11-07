#include <vo_stdio.h>
#include <volink.h> // Linker directives like DLLENTRY
#include <vstypes.h>
#include <stdlib.h>
#include <string.h>
#include <apploader.h> // RunLibraryFunction etc
#include <timers.h>
#include <libaudiodec.h>
#include <vsostasks.h>
#include <consolestate.h>
#include <kernel.h>
#include <unistd.h>

#include "playlist.h"

#define TRUE 1
#define FALSE 0

// void run_test();

void *decoderLibrary = NULL;
AUDIO_DECODER *audioDecoder = NULL;
FILE *fp = NULL;
char *errorString = "nothing to see here";
int eCode = 0;

/**
 * @brief Function to play music via the system's decoder library. Runs in a
 *        separate task.
 *
 */
void PlayerThread(void) {
    eCode = DecodeAudio(decoderLibrary, audioDecoder, &errorString);
}

/**
 * @brief Driver for playing playlists
 *
 * @param parameters Command Line Arguments as a string. Should be the filepath
 *                   of a .m3u playlist file.
 * @return int S_ERROR (-1) if an error occurred, or S_OK if not.
 */
int main(char *parameters) {
    char *playlist_filename = parameters;
    Playlist *h_playlist;
    FILE *current_song;
    char user_input;
    u_int16 quit_selected = FALSE;
    u_int16 restart_song = FALSE;
    u_int16 move_prev = FALSE;
    s_int32 time_elapsed;
    u_int16 shuffle_selected = FALSE;

    printf("Loading Decoder library\n");
    decoderLibrary = LoadLibrary("audiodec");
    if (!decoderLibrary) {
        printf("Couldn't open library\n");
        return S_ERROR;
    }

    // init a new playlist from the given file
    h_playlist = create_playlist_from_file(playlist_filename);
    if (h_playlist == NULL) {
        printf("Error generating playlist from file '%s'", playlist_filename);
        return S_ERROR;
    }

    // play all songs in playlist
    while (h_playlist->current != NULL && quit_selected == FALSE) {
        current_song = fopen(h_playlist->current->filename, "rb");
        if (!current_song) {
            printf("Couldn't open file '%s'\n", current_song);
            return S_ERROR;
        }

        printf("Now Playing: %s\n", current_song->Identify(current_song, NULL, 0));
        RunLibraryFunction("METADATA", ENTRY_2, (int)current_song);

        audioDecoder = CreateAudioDecoder(decoderLibrary, current_song, stdaudioout, NULL, auDecFGuess);
        if (!audioDecoder) {
            printf("Couldn't create audio decoder\n");
            return S_ERROR;
        }

        StartTask(TASK_DECODER, PlayerThread);
        Delay(100);
        while (pSysTasks[TASK_DECODER].task.tc_State && pSysTasks[TASK_DECODER].task.tc_State != TS_REMOVED) {

            printf("\r[%02ld:%02ld]", audioDecoder->cs.playTimeSeconds / 60L, audioDecoder->cs.playTimeSeconds % 60L);

            Delay(250);
            if (ioctl(stdin, IOCTL_TEST, NULL)) {
                //character(s) available in the stdin buffer
                user_input = fgetc(stdin);
                switch (user_input) {
                case 'S':
                case 's':
                    printf("\rSkipping... \n");
                    audioDecoder->cs.cancel = 1;
                    break;
                case 'B':
                case 'b':
                    time_elapsed = audioDecoder->cs.playTimeSeconds;
                    audioDecoder->cs.cancel = 1;
                    if (time_elapsed < 3) {
                        printf("Moving to previous...\n");
                        move_prev = TRUE;
                    }
                    else {
                        printf("Restarting song...\n");
                        restart_song = TRUE;
                    }
                    break;
                case 'Q':
                case 'q':
                    printf("Quitting...\n");
                    audioDecoder->cs.cancel = 1;
                    quit_selected = TRUE;
                    break;
                case 'P':
                case 'p':
                    audioDecoder->pause ^= 1;
                    if (audioDecoder->pause) {
                        printf("Paused\n");
                    }
                    else {
                        printf("Playing\n");
                    }
                    break;
                case 'M':
                case 'm':
                    printf("Shuffling...\n");
                    // cancel playing the current song
                    audioDecoder->cs.cancel = 1;
                    // show shuffle selected
                    shuffle_selected = TRUE;
                    break;
                }
            }

            if (appFlags & APP_FLAG_QUIT) {
                printf("Quitting...\n");
                audioDecoder->cs.cancel = 1;
                quit_selected = TRUE;
            }

            Delay(250);
        }

        fclose(current_song);
        DeleteAudioDecoder(decoderLibrary, audioDecoder);
        if (restart_song) {
            // Pointer remains at current song
            restart_song = FALSE;
        }
        else if (move_prev) {
            // go to previous song
            if (h_playlist->current->prev != NULL) {
                h_playlist->current = h_playlist->current->prev;
            }
            else {
                // at top, to move to tail
                h_playlist->current = h_playlist->last;
            }
            move_prev = FALSE;
        }
        else if (shuffle_selected) {
            // shuffle playlist, which resets current to the new beginning
            printf("Shuffling now...\n");
            shuffle_playlist(h_playlist);
            printf("Resetting current to head\n");
            h_playlist->current = h_playlist->head;
            shuffle_selected = FALSE;
        }
        else if (h_playlist->current->next != NULL) {
            // go to next song
            h_playlist->current = h_playlist->current->next;
        }
        else {
            // reset playlist to beginning
            h_playlist->current = h_playlist->head;
        }
    }

    DropLibrary(decoderLibrary);

    destroy_playlist(&h_playlist);
    return S_OK;
}