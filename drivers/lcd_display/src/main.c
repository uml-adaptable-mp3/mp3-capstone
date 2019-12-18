#include <vo_stdio.h>
#include <volink.h> // Linker directives like DLLENTRY
#include <vstypes.h>
#include <stdlib.h>
#include <string.h>
#include <apploader.h> // RunLibraryFunction etc
#include <timers.h>
#include <vsostasks.h>
#include <consolestate.h>
#include <kernel.h>
#include <unistd.h>

#include "UI.h"
#include "ID3_decode.h"
#include <lcd.h>

#define TRUE 1
#define FALSE 0


void init(char* parameters) {

    UI_init();

}

// 0 for main menu, 1 for now playing
DLLENTRY(switchView)  // ENTRY 1
void switchView(u_int16 view) {
    if (view) {
        // show now playing
        loadHeader();
        loadNowPlaying();
    }
    else {
        // show main menu
        loadHeader();
        loadMainMenu();
    }
}

DLLENTRY(setSong)  // ENTRY 2
void setSong(FILE* file_descriptor) {
    resetSong();
    // runs callback that sets the strings to be displayed when switching to now playing
    DecodeID3(file_descriptor, (UICallback) UIMetadataDecodeCallBack);
}

DLLENTRY(setPercentageComplete)  // ENTRY 3
void setPercentageComplete(u_int16 percentage) {
    updatePercentComplete(percentage);
}

DLLENTRY(currentPlaybackTime)  // ENTRY 4
void currentPlaybackTime(u_int16 time) {
    updatePlaybackTime(time);
}

DLLENTRY(showPlayPause)  // ENTRY 5
void showPlayPause(u_int16 isPaused) {
    UIShowPlayPause(isPaused);
}

void setVolumeLevel(u_int16 volume) {
    // do volume stuff
}

// 0: normal, 1: shuffle, 2: repeat
void showMode(u_int16 mode) {
    switch(mode) {
    case 0:  // normal playback
        break;
    case 1:  // shuffle
        break;
    case 2:  // repeat
        break;
    }
}

void cursorUp(void) {
    uiCursorUp();
}

void cursorDown(void) {
    uiCursorDown();
}

void select(void) {

}

ioresult main(char *parameters) {
    FILE* current_song;
    if (parameters != NULL && parameters[0] == 'i') {
        // init mode, don't do anything
    }
    else if (parameters != NULL && parameters[0] == 'h') {
        // load the header
        loadHeader();
    }
    else if (parameters != NULL && parameters[0] == 'm') {
        // load the main menu
        switchView(0);
    }
    else if (parameters != NULL && parameters[0] == 'p') {
        // load the now playing
        current_song = fopen("D:Music/Jeff Buckley - Hallelujah.mp3", "rb");
        if (!current_song) {
            printf("Couldn't open file '%s'\n", current_song);
            return S_ERROR;
        }
        setSong(current_song);
        fclose(current_song);
        switchView(1);
    }
    else if (parameters != NULL && parameters[0] == 'u') {
        cursorUp();
    }
    else if (parameters != NULL && parameters[0] == 'd') {
        cursorDown();
    }
    else {
        LcdInit(0);
    }

    return S_OK;
}
