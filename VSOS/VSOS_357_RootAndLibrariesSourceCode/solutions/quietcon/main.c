/* For free support for VSIDE, please visit www.vsdsp-forum.com */

// This program removes all console output and publishes uartOut so you can print to the UART yourself.
/* 
	To use this program, include this line in the beginning of your program:
	
		DLLIMPORT(uartOut) extern VO_FILE *uartOut;
	
	Then you can print to the uart with e.g. 
		
		fprintf(uartOut,"Hello, Uart!\n");

*/

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <kernel.h>		// Kernel symbols

const FILEOPS nullFileOperations = {
	(void*)CommonOkResultFunction,
	(void*)CommonOkResultFunction,
	(void*)CommonOkResultFunction,
	(void*)CommonOkResultFunction,
	(void*)CommonOkResultFunction,
};
char* NullFileIdentify(register __i0 void *self, char *buf, u_int16 bufsize) {return "FNUL";}
const SIMPLE_FILE nullFile = {__MASK_PRESENT | __MASK_OPEN | __MASK_READABLE | __MASK_WRITABLE | __MASK_FILE, 
	NullFileIdentify, &nullFileOperations};

VO_FILE *stdout_save;
VO_FILE *stderr_save;

//Redirect all output to null and publish uartOut
void init(void) {
	stdout_save = vo_stdout;
	stderr_save = vo_stderr;
	AddSymbol("_uartOut", NULL, (u_int16)(&stderr_save)); //publish original stderr as uartOut
	vo_stdout = &nullFile; //point stdout to nullfile
	vo_stderr = &nullFile; //point stderr to nullfile
}

void fini(void) {
	vo_stdout = stdout_save;
	vo_stderr = stderr_save;
}

