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

// Startup code for each instance of the library
// If CONFIG.TXT has several instance of the same driver, this is called for each line.
ioresult main(char *parameters) {
	u_int16 i,j,k;
	sscanf(parameters,"%x,%x",&i,&j);
	k = PERIP(i);
	printf(parameters);
	printf(" Y:0x%x: 0x%x-%d -> ",i,k,j);
	k &= ~(1 << j);
	printf("0x%x ",k);
	PERIP(i) = k;
	return S_OK;
}
