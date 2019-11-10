#include <vo_stdio.h>
#include <volink.h> // Linker directives like DLLENTRY
#include <vstypes.h>
#include <stdlib.h>
#include <string.h>
#include <apploader.h> // RunLibraryFunction etc
#include <timers.h>
#include <libaudiodec.h>
#include <vsostasks.h>
#include <taskandstack.h>
#include <consolestate.h>
#include <kernel.h>
#include <unistd.h>
#include <uimessages.h>         // Volume control and reporting
#include <aucommon.h>

#include "volume.h"

#define TRUE 1
#define FALSE 0

struct TaskAndStack *taskAndStack = NULL;
u_int16 quitVolumeTask = 0;


int VolumeTask(char *parameters) {
    s_int16 volume = 90; // 30 - (90/6) = volume 15 to start
    char user_input;
    
    // Sets the volume to 15 - a reasonable volume to start at
    ioctl(stdaudioout, IOCTL_AUDIO_SET_VOLUME, (void *)(volume+256));

    while(!quitVolumeTask) {
        if (ioctl(stdin, IOCTL_TEST, NULL)) {
            //character(s) available in the stdin buffer
            user_input = fgetc(stdin);
            switch (user_input) {
            case '+':
                printf("Volume Up\n");
                volume = volumeUp(volume);
                break;
            case '-':
                printf("Volume Down\n");
                volume = volumeDown(volume);
                break;
            }

            Delay(250);
        }
    }
}

DLLENTRY(init)
ioresult init(char* parameters) {
    taskAndStack = CreateTaskAndStack(VolumeTask, "VOLUME", 256, 6);
    if(!taskAndStack){
        return S_ERROR;
    }
    
    // StartTask(TASK_IO, VolumeTask);
    return S_OK;
}

DLLENTRY(fini)
void fini(void) {
    /* Remove task running VolumeTask() */
    if (taskAndStack) {
        quitVolumeTask = 1;

        while (taskAndStack->task.tc_State &&
               taskAndStack->task.tc_State != TS_REMOVED) {
            Delay(TICKS_PER_SEC/100);
        }
        FreeTaskAndStack(taskAndStack);
    }
}