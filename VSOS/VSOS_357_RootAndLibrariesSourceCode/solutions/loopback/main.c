/* For free support for VSIDE, please visit www.vsdsp-forum.com */

// Starting point template for creating VSOS audio applications.
// This will create a <projectname>.AP3 file, which you can copy to 
// your VS1005 Developer Board and run it from there.

// If you rename your application to INIT.AP3, then the kernel will
// load it automatically after booting.

#include <vo_stdio.h>
#include <stdlib.h>
#include <apploader.h> // Contains LoadLibrary() and DropLibrary()
#include <saturate.h> // Contains number saturation functions
#include <consolestate.h>
#include <kernel.h>

#define BUFSIZE 128

int main(void) {
	// Remember to never allocate buffers from stack space. So, if you
	// allocate the space inside your function, never forget "static"!
	static s_int16 myBuf[BUFSIZE];
	// By default, VSOS contains a simple output audio driver.
	// In CONFIG.TXT you can load different drivers for audio input and output.
	// This program copies samples from stdaudioin to stdaudioout.
	
	printf("Looping from stdaudioin to stdaudioout...\n");
	while (!(appFlags & APP_FLAG_QUIT)) {
		// Read stereo samples from stdaudioin into myBuf.
		// By default, stdaudioin comes from line in.
		// By default both input and output are 16-bit stereo at 48 kHz.
		fread(myBuf, sizeof(s_int16), BUFSIZE, stdaudioin);

		// Here you may process audio in 16-bit L/R stereo format.
#if 0
		// This example code shows how to add to left channel volume.
		// Replace "#if 0" with "#if 1" to activate.
		{
			int i;
			s_int16 *p = myBuf; // Buffer to first element in myBuf
			for (i=0; i<BUFSIZE; i+=2) {
				// NOTE: The 16-bit sample is first cast to 32 bits to avoid
				// overflow in multiplication (which would lead to catastrophic
				// distortion), then multiplied by 4 (increase volume by 12 dB),
				// then finally saturated (which leads to smooth distortion if
				// overflow occurs), then cut back to 16 bits.
				*p = Sat32To16((s_int32)*p * 4);
				p+=2; // Skip to next left sample
			}
		}
#endif
		// Write stereo samples from myBuf into stdaudioout.
		// By default, stdaudioout goes to line out.
		fwrite(myBuf, sizeof(s_int16), BUFSIZE, stdaudioout);
	} /* while (1) */
	printf("Exit loopback.\n");
	return EXIT_SUCCESS;
}

