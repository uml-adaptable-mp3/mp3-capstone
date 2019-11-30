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
#include <lcd.h>

#define TRUE 1
#define FALSE 0


void init(char* parameters) {

    UI_init();

}



ioresult main(char *parameters) {
    if (parameters != NULL && parameters[0] == 'i') {
        // init mode, don't do anything
    }
    else if (parameters != NULL && parameters[0] == 'h') {
        // load the header
        loadHeader();
    }
    else if (parameters != NULL && parameters[0] == 'm') {
        // load the main menu
        loadHeader();
        loadMainMenu();
    }
    else if (parameters != NULL && parameters[0] == 'p') {
        // load the main menu
        loadHeader();
        loadNowPlaying();
    }
    else {
        LcdInit(0);
    }

    return S_OK;
}
