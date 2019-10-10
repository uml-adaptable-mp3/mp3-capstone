/* For free support for VSIDE, please visit www.vsdsp-forum.com */

// Starting point template for creating VSOS3 libraries and device drivers.
// This will create a <projectname>.DL3 file, which you can copy to 
// your VS1005 Developer Board's system disk's SYS subdirectory.

// If init(), main() or fini() are not needed, remove them from the solution.
// There's no need to have any unneeded functions in the library.

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <kernel.h>
#include "usbmsc.h"



ioresult main(char *parameters) {
	int newDevice = parameters ? (*parameters & ~0x20) : 0;
	if (newDevice>='A' && newDevice<='Z') {
		newDevice -= 'A';
	} else {
		newDevice = 'U'-'A'; /* Default */
	}
	usbDisk = vo_pdevices[newDevice];
	UsbMainLoop();
	PERIP(USB_CF) = 0;
}
