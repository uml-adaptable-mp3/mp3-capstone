/// \file main.c VSOS3 LCD Console Device Driver
/// \author Panu-Kristian Poiksalo, VLSI Solution Oy

/// This device driver uses LcdFilledRectangle to draw console output.

/* For free support for VSIDE, please visit www.vsdsp-forum.com */

// Starting point template for creating VSOS3 libraries and device drivers.
// This will create a <projectname>.DL3 file, which you can copy to 
// your VS1005 Developer Board's system disk's SYS subdirectory.

// DL3 files require VSOS3 kernel version 0.3x to run.

// If you add the libary name to S:CONFIG.TXT, and put the library 
// to S:SYS, it will be loaded and run during boot-up. 

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <kernel.h>
#include "console.h"

const FILEOPS ConsoleFileOperations = { NULL, NULL, ConsoleIoctl, ConsoleWrite, ConsoleWrite };
const SIMPLE_FILE lcdConsoleFile = {__MASK_PRESENT | __MASK_OPEN | __MASK_WRITABLE | __MASK_FILE, NULL, &ConsoleFileOperations};

FILE *vo_stdout_save = NULL;

// Library or driver initialization code
// This function is called when the library is loaded.
// It is planned that for VSOS 0.310 and above this will be the function 
// which starts device drivers. Init may also be removed from memory after the call.

ioresult main(char *parameters) {
	vo_stdout_save = vo_stdout;
	vo_stdout = &lcdConsoleFile;	
	console.Ioctl = ConsoleIoctl;
	ConsoleFrame("Console"); 
	return S_OK;
}

// Library finalization code

void fini(void) {
	// Add code here to force release of resources such as 
	// memory allocated with malloc, entry points, 
	// hardware locks or interrupt handler vectors.
	vo_stdout = vo_stdout_save;
}
