// Example for how to copy single files, or several files recursively,
// using the .AP3 graphical front-end applications.
#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <unistd.h>
#include <copyfile.h>
#include <kernel.h>


int main(void) {
	ioresult res;
#if 0
	// Copy S:SYS/decwma.dl3 to E:dest.dl3
	printf("CopyFile1   = %d\n", CopyFile("E:dest.dl3", "S:SYS/decwma.dl3"));
#endif
#if 0
#if 0
	// Copy whole system disk to directory E:copy
    res = CopyFileRec("E:copy", "S:");
#else
	// Copy whole system disk to directory E:copy, send log to stderr (typically UART)
	res = CopyFileRecVerbose("E:copy", "S:", stderr);
#endif
	printf("CopyFileRec = %d\n", res);
#endif
#if 0
	// Create directory E:mydir
	res = mkdir("E:mydir");
	fprintf(stderr, "mkdir = %d\n", res);
#endif  
	return S_OK;
}
