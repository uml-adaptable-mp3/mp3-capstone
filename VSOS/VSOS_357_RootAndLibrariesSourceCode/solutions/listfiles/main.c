/* For free support for VSIDE, please visit www.vsdsp-forum.com */

// Starting point template for creating VSOS3 libraries and device drivers.
// This will create a <projectname>.DL3 file, which you can copy to 
// your VS1005 Developer Board's system disk's SYS subdirectory.

// If init(), main() or fini() are not needed, remove them from the solution.
// There's no need to have any unneeded functions in the library.

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <kernel.h>		// Kernel symbols


ioresult main(char *parameters) {
	char s[200];
	char m[8];
	u_int16 i=0;
	FILE *f;
	sprintf(s,"%s*",parameters);
	while(1) {
		sprintf(m,"rb#%d",i++);
		f=fopen(s,m);
		if (!f) break;
		printf("%3d:  %8ld  %s\n",i,f->fileSize,f->Identify(f,NULL,0));
		fclose(f);
	}
}


