#include <vo_stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <apploader.h>
#include <vstypes.h>
#include <vs1005h.h>
#include <clockspeed.h>
#include <audio.h>
#include <timers.h>
#include <dsplib.h>
#include <crc32.h>
#include <kernel.h>
#include <cyclic.h>
#include <extSymbols.h>
#include <taskandstack.h>

#include "buttons.h"

/** ButtonState struct **
 * Indicates state of button presses
 *      0 - inactive
 *      1 - active
 *************************/
/* struct ButtonState_t {
    int play_pause;
    int prev;
    int next;
    int queue_toggle;
    int menu;
    int vol_up;
    int vol_down;
}; */

u_int16 __mem_y quitButtonTask = 0;
struct TaskAndStack *taskAndStack = NULL;

int ButtonTask(char *parameters) {
    //struct ButtonState_t ButtonStates = {0, 0, 0, 0, 0, 0, 0};
    void *playlistLib = NULL;
    char *playlist = "D:Playlists/playlist.m3u";

    while(!quitButtonTask) {
        // Play / Pause
        // pin 44
        if(GpioReadPin(BUTTON1) & ACTIVEHIGH) {
            RunLibraryFunction("Playlist", ENTRY_1, 0);
            while (GpioReadPin(BUTTON1) & ACTIVEHIGH);
        }
        // Skip
        // pin 26
        if(GpioReadPin(BUTTON2) & ACTIVEHIGH) {
            RunLibraryFunction("Playlist", ENTRY_2, 0);
            while (GpioReadPin(BUTTON2) & ACTIVEHIGH);
        }
        // Prev
        // pin 56
        if(GpioReadPin(BUTTON3) & ACTIVEHIGH) {
            RunLibraryFunction("Playlist", ENTRY_3, 0);
            while (GpioReadPin(BUTTON3) & ACTIVEHIGH);
        }
        // Shuffle
        // pin 57
        if(GpioReadPin(BUTTON4) & ACTIVEHIGH) {
            RunLibraryFunction("Playlist", ENTRY_4, 0);
            while (GpioReadPin(BUTTON4) & ACTIVEHIGH);
        }
        // // Menu
        // // pin 31
        // if(GpioReadPin(BUTTON5) & ACTIVEHIGH) {
        //     // TODO: implement menu stuff
        // }
        // Volume up
        // pin 32
        if(GpioReadPin(BUTTON6) & ACTIVEHIGH) {
            //ButtonStates.vol_up = 1;
            RunLibraryFunction("Volume", ENTRY_1, 0);
            RunLibraryFunction("lcd_display", ENTRY_6, 0);
        }
        // Volume Down
        // pin 33
        if(GpioReadPin(BUTTON7) & ACTIVEHIGH) {
            //ButtonStates.vol_down = 1;
            RunLibraryFunction("Volume", ENTRY_2, 0);
            RunLibraryFunction("lcd_display", ENTRY_6, 0);
        }
        Delay(500);
    }
}

ioresult init(void) {
    if (!(taskAndStack = CreateTaskAndStack(ButtonTask, "BUTTONS", 256, 2))) {
        return S_ERROR;
    }

    return S_OK;
}

void fini(void) {
    /* Remove task running ButtonTask() */
    if (taskAndStack) {
        quitButtonTask = 1;

        while (taskAndStack->task.tc_State && 
               taskAndStack->task.tc_State != TS_REMOVED) {
            Delay(TICKS_PER_SEC/100);
        }
        FreeTaskAndStack(taskAndStack);
    }
}
