/*
  File buffer device FILEBUF.DL3 v0.02
  VLSI Solution 2014-09-25 HH

  This code creates an automatic Input or Output ring buffer for a file.
  It is intended to be used when playing/recording media files from/to a
  memory device that may occasionally freeze for a while. Typical examples
  of such memories are SD cards and USB devices. For instance, when
  writing to an SD card, writes are normally very fast (less than one
  millisecond), but occasional writes can take upto half a second. Without
  buffering recording to such a device would not be continuous.

  To use this device driver, copy the file FILEBUF.DL3 to the SYS
  folder of your VS1005 boot device. Then open a file, and run 
  CreateFileBuf() function for the file. Now file buffering will be
  in use until you close the file. An example of how to do this
  is the Simple MP3 Encoder example, available at VLSI Solution's
  web pages at http://www.vlsi.fi/ as well as the VSDSP Forum at
  http://www.vsdsp-forum.com/ .

  This code may be used freely in any product containing one or more ICs
  by VLSI Solution.

  No guarantee is given for the usability of this code.

 */

/* For free support for VSIDE, please visit www.vsdsp-forum.com */

// Starting point template for creating VSOS3 libraries and device drivers.
// This will create a <projectname>.DL3 file, which you can copy to 
// your VS1005 Developer Board's system disk's SYS subdirectory.

// If init(), main() or fini() are not needed, remove them from the solution.
// There's no need to have any unneeded functions in the library.

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <timers.h>
#include <kernel.h>
#include "autobuffer.h"

DLLENTRY(CreateFileBuf)
/* When loaded, the library becomes an array of pointers to DLLENTRies. 
   You can run the functions by indexing the array and casting its
   members to function pointers of the appropriate type. Please see the
   PLAYFILE library for an example.  */
ioresult CreateFileBuf(register FILE *fp, register u_int16 bufWords) {
  return BindAutoBuffer(fp, bufWords);
}

// This function is called when the library is loaded.
// If CONFIG.TXT has several instance of the same driver, this is called only once.
void init(void) {
  // Add here code to save any system variables that are overwritten by the library
  // If you have global memory allocation code, add it here.
  // This function may be removed from memory after calling it.	
  //	printf("Library Loading...\n");
}

// Startup code for each instance of the library
// If CONFIG.TXT has several instance of the same driver, this is called for each line.
ioresult main(char *parameters) {
  // If this is a device driver, add function handlers with SetHandler
  // or write new values of system variables here. Don't forget to save the
  // old values, you should restore their values in fini().
  //	printf("Library Loading...\n");
  //	printf("Parameters: '%s'\n",parameters);
  return S_OK;
}

// Library finalization code. This is called when the library is dropped from memory,
// (reference count drops to zero due to a call to DropLibrary)
void fini(void) {
  // Add code here to force release of resources such as 
  // memory allocated with malloc, entry points, 
  // hardware locks or interrupt handler vectors.
  // Restore all handlers that you have set in init.
  //	printf("Library Unloading.\n");
  while (autoBuffer.n) {
    printf("Can't remove FILEBUF.DL3, useCount = %d\n", autoBuffer.n);
    Delay(TICKS_PER_SEC);
  }
}
