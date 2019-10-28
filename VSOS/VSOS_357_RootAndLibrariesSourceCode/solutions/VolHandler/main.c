/* For free support for VSIDE, please visit www.vsdsp-forum.com */

// Starting point template for creating VSOS3 libraries and device drivers.
// This will create a <projectname>.DL3 file, which you can copy to 
// your VS1005 Developer Board's system disk's SYS subdirectory.

// If init(), main() or fini() are not needed, remove them from the solution.
// There's no need to have any unneeded functions in the library.

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <cyclic.h>
#include <timers.h>
#include <kernel.h>
#include <uimessages.h>


void VolumeHandler(register struct CyclicNode *perNode) {
	if (GpioReadPin(0x02)) {
		SystemUiMessage(-1, UIMSG_BUT_VOLUME_DOWN, 1);
	}
	if (GpioReadPin(0x03)) {
		SystemUiMessage(-1, UIMSG_BUT_VOLUME_UP, 1);
	}
}

struct CyclicNode volumeCyclic ={{0}, VolumeHandler};


// This function is called when the library is loaded.
// If CONFIG.TXT has several instance of the same driver, this is called only once.
void init(void) {
	AddCyclic(&volumeCyclic, TICKS_PER_SEC/10, 0);
}


// Library finalization code. This is called when the library is dropped from memory,
// (reference count drops to zero due to a call to DropLibrary)
void fini(void) {
	DropCyclic(&volumeCyclic);	
}
