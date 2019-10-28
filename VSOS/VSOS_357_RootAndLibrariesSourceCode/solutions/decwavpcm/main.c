/* For free support for VSIDE, please visit www.vsdsp-forum.com */

// Starting point template for creating VSOS applications.
// This will create a <projectname>.APP file, which you can copy to 
// your VS1005 Developer Board and run it from there.

// If you rename your application to INIT.APP, then the kernel will
// load it automatically after booting.

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <codecwav.h>
#include <libdecwav.h>
#include <kernel.h>

struct Codec *cod = NULL;
void (*Delete)(struct Codec *cod) = NULL;

void fini(void);


DLLENTRY(Create)
struct Codec *Create(void) {
	if (cod = CodWavCreate()) {
		// Remember original Delete() function address
		Delete = cod->Delete;
		// Grab Delete() so that we can do our own clean-up
		cod->Delete = (void *)fini;
	}

	return cod; 
}

void fini(void) {
	if (cod) {
		if (Delete) {
			Delete(cod);
			Delete = NULL;
		}
		cod = NULL;
	}
}
