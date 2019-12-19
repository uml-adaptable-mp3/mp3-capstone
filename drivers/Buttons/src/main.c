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

void ButtonTask(void) {
    //struct ButtonState_t ButtonStates = {0, 0, 0, 0, 0, 0, 0};
    void *playlistLib = NULL;
    u_int16 in_menu = 0;

    while(!quitButtonTask) {
        // Check UI state - some button fucntions vary when in menu
        in_menu = RunLibraryFunction("lcd_display", ENTRY_11, 0);
        // Play / Pause
        // Select
        // pin 44
        if(GpioReadPin(BUTTON1) & ACTIVEHIGH) {
            //ButtonStates.play_pause = 1;
            if(in_menu > 0) {
                RunLibraryFunction("lcd_display", ENTRY_10, 0);
            }
            else {
                RunLibraryFunction("Playlist", ENTRY_1, 0);
            }
            while(GpioReadPin(BUTTON1));
        }
        // Skip
        // Menu down
        // pin 26
        if(GpioReadPin(BUTTON2) & ACTIVEHIGH) {
            if(in_menu > 0) { 
                RunLibraryFunction("lcd_display", ENTRY_9, 0);
            }
            else {
                RunLibraryFunction("Playlist", ENTRY_2, 0);
            }
            while(GpioReadPin(BUTTON2));
        }
        // Prev
        // Menu up
        // pin 56
        if(GpioReadPin(BUTTON3) & ACTIVEHIGH) {
            if(in_menu > 0) { 
                RunLibraryFunction("lcd_display", ENTRY_8, 0);
            }
            else {
                RunLibraryFunction("Playlist", ENTRY_3, 0);
            }
            while(GpioReadPin(BUTTON3));
        }
        // Shuffle
        // pin 57
        if(GpioReadPin(BUTTON4) & ACTIVEHIGH) {
            RunLibraryFunction("Playlist", ENTRY_4, 0);
            while(GpioReadPin(BUTTON4));
        }
        // Menu
        // pin 31
        if(GpioReadPin(BUTTON8) & ACTIVEHIGH) {
            RunLibraryFunction("lcd_display", ENTRY_1, 0);
            while(GpioReadPin(BUTTON8));
        }
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

        // TODO: TESTING CHANGING PLAYLIST
       // if(GpioReadPin(BUTTON8) & ACTIVEHIGH) {
        //    printf("You touched the button!\n");
        //     RunLibraryFunction("Playlist", ENTRY_6, (int)playlist);
        //}
        Delay(250);
    }
}

ioresult init(void) {
    if (!(taskAndStack = CreateTaskAndStack((void *)ButtonTask, "BUTTONS", 256, 2))) {
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
