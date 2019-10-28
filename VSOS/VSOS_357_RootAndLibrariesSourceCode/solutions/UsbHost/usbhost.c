/* For free support for VSIDE, please visit www.vsdsp-forum.com */

// Starting point template for creating VSOS applications.
// This will create a <projectname>.APP file, which you can copy to 
// your VS1005 Developer Board and run it from there.

// If you rename your application to INIT.APP, then the kernel will
// load it automatically after booting.

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include "devUMass.h"
#include <ctype.h>
#include <string.h>
#include <kernel.h>

DEVICE usbFlash[4];
char usbSystemDevices[4] = {0};

int main(char *parameters) {
	u_int16 nDisks;
	u_int16 i;
		
	nDisks = DevUMassProbe();
	
	if (nDisks == 0) {
		fprintf(vo_stderr,"No device.\n");
		return S_ERROR;
	}
	
	fprintf(vo_stderr,"%d drive(s).\n",nDisks);
	if (strlen(parameters) < nDisks) {
		nDisks = strlen(parameters);
	}
	if (nDisks > 4) nDisks = 4;
	
	for (i=0; i<nDisks; i++) {
		char c = toupper(parameters[i]);
		if (CreateUsbFlashDisk(&usbFlash[i], i) == S_OK) {
			fprintf(vo_stderr, "%c: %s\n",c,usbFlash[i].Identify(&usbFlash[i],NULL,0));
			vo_pdevices[c-'A'] = &usbFlash[i];
			usbSystemDevices[i] = c;
		} else {
			fprintf(vo_stderr, "%c: No disk\n",c);
		}
	}
	return S_OK;
}
	
void fini() {
	u_int16 i;
	for (i=0; i<4; i++) {
		if (usbSystemDevices[i]) {
			printf("Close device %c:\n",usbSystemDevices[i]);
			vo_pdevices[usbSystemDevices[i]-'A'] = 0;			
		}
	}
}