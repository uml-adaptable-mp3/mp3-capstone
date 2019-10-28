/*
  Automatic Gain Control AGC.DL3 v1.00
  VLSI Solution 2014-10-06 HH

  This code can create a Subsonic filter and Automatic Gain Control (AGC).
  It is intended to adapt recording gain to get a more uniform recording
  signal level from a microphone. AGC should not be used for high-quality
  program sources.

  To use this library, copy the file AGC.DL3 to the SYS folder of your
  VS1005 boot device. Then open a file, and run LibInitSubsonic() and
  LibInitAgc(), followed by LibSubsonic() and LibAgc() calls. For AGC to
  work properly, you always need to remove any DC offset from the signal
  by calling the Subsonic() high-pass filter first. To see how to use
  the AGC library, see for example the SimpleMp3Encoder source code.

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
#include "agc.h"

DLLENTRY(LibInitSubsonic)
void LibInitSubsonic(register __i0 struct SubsonicM *subsonicM,
		     register __a0 s_int16 channels,
		     register __i1 struct SubsonicG __mem_y *subsonicG,
		     register __reg_d u_int32 sampleRate) {
  InitSubsonic(subsonicM, channels, subsonicG, sampleRate);
}

DLLENTRY(LibSubsonic)
void LibSubsonic(register __i0 struct SubsonicM *mem,
		 register __i1 struct SubsonicG __mem_y *g,
		 register __i2 s_int16 *d,
		 register __a0 u_int16 n) {
  SubsonicN(mem, g, d, n);
}

DLLENTRY(LibInitAgc)
void LibInitAgc(register struct Agc *agc, register s_int16 channels,
		register struct AgcConsts __mem_y *agcConsts,
		register u_int16 maxGain,
		register u_int32 sampleRate) {
  InitAgc(agc, channels, agcConsts, maxGain, sampleRate);
}

DLLENTRY(LibAgc)
void LibAgc(register __i0 struct Agc *agc,
	    register __i1 __mem_y const struct AgcConsts *consts,
	    register __i2 s_int16 *d,
	    register __a0 u_int16 n) {
  AgcN(agc, consts, d, n);
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
}
