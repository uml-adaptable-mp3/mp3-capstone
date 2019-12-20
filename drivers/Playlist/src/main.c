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


static void *decoderLibrary = NULL;
static AUDIO_DECODER *audioDecoder = NULL;
static FILE *fp = NULL;
static char *errorString = "VSOS BAD";
static int eCode = 0;
static char playlist_filename[127];
static char temp_filename[50];
static Playlist *h_playlist;
static FILE *current_song;


u_int16 quit_selected = FALSE;
u_int16 restart_song = FALSE;
u_int16 move_prev = FALSE;
u_int16 select_next = FALSE;
s_int32 time_elapsed;
u_int16 shuffle_selected = FALSE;
u_int16 repeat_selected = FALSE;
u_int16 linear_selected = FALSE;
u_int16 queue_mode = 0;

/**
 * @brief Function to play music via the system's decoder library. Runs in a
 *        separate task.
 *
 */
void PlayerThread(void) {
    eCode = DecodeAudio(decoderLibrary, audioDecoder, &errorString);
}

DLLENTRY(PlayPause)     // ENTRY_1
void PlayPause(void) {
    audioDecoder->pause ^= 1;
	RunLibraryFunction("lcd_display", ENTRY_5, audioDecoder->pause);
    if (audioDecoder->pause) {
        printf("Paused\n");
    }
    else {
        printf("Playing\n");
    }
}

DLLENTRY(Skip)      // ENTRY_2
void Skip(void) {
    printf("\rSkipping... \n");
    audioDecoder->cs.cancel = 1;
    select_next = TRUE;
}

DLLENTRY(Prev)      // ENTRY_3
void Prev(void) {
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
}

DLLENTRY(QueueToggle)   // ENTRY_4
void QueueToggle(void) {

    if(queue_mode == 0) {
        printf("Repeat On...\n");
        queue_mode++;
        repeat_selected = TRUE;
        linear_selected = FALSE;
    }
    else if(queue_mode == 1) {
        printf("Shuffling...\n");
        // cancel playing the current song
        //audioDecoder->cs.cancel = 1;
        // show shuffle selected
        queue_mode++;
        shuffle_selected = TRUE;
        repeat_selected = FALSE;
    }
    else {
        printf("Regular queue...\n");
        queue_mode = 0;
        shuffle_selected = FALSE;
        linear_selected = TRUE;
    }
    RunLibraryFunction("lcd_display", ENTRY_7, queue_mode);
}

DLLENTRY(Quit)      // ENTRY_5
void Quit(void) {
    printf("Quitting...\n");
    audioDecoder->cs.cancel = 1;
    quit_selected = TRUE;
}

/**
 * @brief Driver for playing playlists
 *
 * @param parameters Command Line Arguments as a string. Should be the filepath
 *                   of a .m3u playlist file.
 * @return int S_ERROR (-1) if an error occurred, or S_OK if not.
 */
int main(char *parameters) {
    char user_input;

    // playlist_filename = parameters;
    strncpy(playlist_filename, parameters, 127);
    
    printf("Loading Decoder library\n");
    decoderLibrary = LoadLibrary("audiodec");
    if (!decoderLibrary) {
        printf("Couldn't open library\n");
        return S_ERROR;
    }


    // init a new playlist from the given file
    printf("Changing playlist to %s", playlist_filename);
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
            destroy_playlist(&h_playlist);
            fclose(current_song);
            return S_ERROR;
        }

        printf("Now Playing: %s\n", current_song->Identify(current_song, NULL, 0));
        // RunLibraryFunction("METADATA", ENTRY_2, (int)current_song);
        // RunLibraryFunction("lcd_display", ENTRY_2, (int) current_song);

        RunLibraryFunction("lcd_display", ENTRY_3,
            (int) (100.0 * ((double) audioDecoder->cs.Tell(&audioDecoder->cs) / (double) audioDecoder->cs.fileSize)));
        RunLibraryFunction("lcd_display", ENTRY_4, (int) audioDecoder->cs.playTimeSeconds);  // currentPlaybackTime

        audioDecoder = CreateAudioDecoder(decoderLibrary, current_song, stdaudioout, NULL, auDecFGuess);
        if (!audioDecoder) {
            printf("Couldn't create audio decoder\n");
            destroy_playlist(&h_playlist);
            fclose(current_song);
            return S_ERROR;
        }
        StartTask(TASK_DECODER, PlayerThread);
        Delay(100);
        while (pSysTasks[TASK_DECODER].task.tc_State && pSysTasks[TASK_DECODER].task.tc_State != TS_REMOVED) {

            printf("\r[%02ld:%02ld]", audioDecoder->cs.playTimeSeconds / 60L, audioDecoder->cs.playTimeSeconds % 60L);

            RunLibraryFunction("lcd_display", ENTRY_3,
                (int) (100.0 * ((double) audioDecoder->cs.Tell(&audioDecoder->cs) / (double) audioDecoder->cs.fileSize)));
            RunLibraryFunction("lcd_display", ENTRY_4, (int) audioDecoder->cs.playTimeSeconds);  // currentPlaybackTime

            Delay(250);
            if (ioctl(stdin, IOCTL_TEST, NULL)) {
                //character(s) available in the stdin buffer
                user_input = fgetc(stdin);
                switch (user_input) {
                case 'S':
                case 's':
                    Skip();
                    break;
                case 'B':
                case 'b':
                    Prev();
                    break;
                case 'Q':
                case 'q':
                    Quit();
                    break;
                case 'P':
                case 'p':
                    PlayPause();
                    break;
                case 'M':
                case 'm':
                    QueueToggle();
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
        else if (h_playlist->current->next != NULL) {
            if (repeat_selected && !select_next) {
                // do nothing - keep current song
            }
            else {
                // go to next song
                h_playlist->current = h_playlist->current->next;
                if (select_next) {
                    select_next = FALSE;
                }
            }
        }
        else {
            // reset playlist to beginning
            h_playlist->current = h_playlist->head;
        }
        
        // Check queue toggles
        if (shuffle_selected) {
            // shuffle playlist, which resets current to the new beginning
            printf("Shuffling now...\n");
            shuffle_playlist(h_playlist);
            printf("Resetting current to head\n");
            h_playlist->current = h_playlist->head;
            shuffle_selected = FALSE;
        }
        else if (linear_selected) {
            // go back to regular playlist order
            printf("Back to regular queue...\n");
            strcpy(temp_filename, h_playlist->current->filename);
            destroy_playlist(&h_playlist);
            h_playlist = create_playlist_from_file(playlist_filename);
            if (h_playlist == NULL) {
                printf("Error generating playlist from file '%s'", playlist_filename);
                destroy_playlist(&h_playlist);
                fclose(current_song);
                return S_ERROR;
            }
            while(strcmp(h_playlist->current->filename, temp_filename) != 0 && h_playlist->current != NULL) {
                h_playlist->current = h_playlist->current->next;
            }
            linear_selected = FALSE;
        }
        printf("Closing current song\n");
        fclose(current_song);
        printf("Deleting audio decoder\n");
        DeleteAudioDecoder(decoderLibrary, audioDecoder);
    }
    printf("Exiting Playlist\n");
    printf("Dropping library\n");
    destroy_playlist(&h_playlist);
    DropLibrary(decoderLibrary);
    
   return S_OK;
}
