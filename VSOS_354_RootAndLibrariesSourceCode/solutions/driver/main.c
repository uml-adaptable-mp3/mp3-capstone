/* For free support for VSIDE, please visit www.vsdsp-forum.com */

/// \file liblist.c Print out a list of loaded libraries and free mem to stdout
/// \author Panu-Kristian Poiksalo, VLSI Solution Oy


#include <vo_stdio.h>
#include <volink.h>
#include <apploader.h>
#include <lcd.h>
#include <stdbuttons.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <kernel.h>

u_int16 LibNameCompare(register u_int16 *lib, register const char *name) {
	char *s;
	lib += *lib + 6;
	s = (char *)(lib + *lib * 2 + 2);

	while (1) {
		if (*name == 0) return 1;
		if (*s == 0) return 1;
		if (toupper(*s) != toupper(*name)) return 0;
		s++; name++;
	}
}

void *FindLib(register const char *name) {
	u_int16 i;
	for (i=0; i<MAX_LIB; i++) {
		if (loadedLib[i] && LibNameCompare(loadedLib[i],name)) return loadedLib[i];
	}
	return NULL;
}

void *main(char *parameters) {
	u_int16 i;
	volatile u_int16 *lib;

	lib = FindLib(&parameters[1]);
	switch(parameters[0]) {
		case '?': {
			if (!lib) { 
				break;
			}
			printf("Lib %s is at %p.\n",parameters+1,lib);
			return lib;
		}
		
		case '-': {
			if (!lib) { 
				break;
			}
			lib[1] = 1;
			DropLibrary((void*)lib);			
			return NULL;
		}
		
		case '+': {
			char *program = parameters+1;
			char *params = strchr(program, ' ');
			while (*params == ' ') {
				*params++ = '\0';
			}
			lib = LoadLibraryP(program,params);
			if (lib[2+ENTRY_MAIN] != NULL) { // If there is main function
				RunLoadedFunction(lib, ENTRY_MAIN, (int)params);
			}
			return lib;
		}



		default: {
			printf("Usage: DRIVER {+|-|?}libname\n");
			return NULL;
		}
	}
	
	printf("Lib %s not found.\n",parameters+1);
	finally:
	return NULL;

	//printf("%s:%p\n",parameters,FindLib(parameters));
}
