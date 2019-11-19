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

#define TRUE 1
#define FALSE 0

int main(char *parameters) {
    char *playlist_filename = parameters;

    // printf("Display Driver Not Implemented.\n");
    // return S_OK;
    // load the main menu
    loadMainMenu();


    return S_OK;
}
