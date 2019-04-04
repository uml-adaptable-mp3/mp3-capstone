#ifndef CONSOLESTATE_H
#define CONSOLESTATE_H

#include <vsos.h>
#include <volink.h>

#define APP_FLAG_QUIT (1<<0)
#define APP_FLAG_RAWTTY (1<<1)
#define APP_FLAG_ECHO (1<<2)

extern u_int16 appFlags; LINK_ABS (appFlags, 2255) 
extern char *currentDirectory; LINK_ABS (currentDirectory, 2254)

#endif


