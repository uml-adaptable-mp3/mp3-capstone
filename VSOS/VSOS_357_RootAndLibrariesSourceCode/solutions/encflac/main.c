/*
  FLAC Encoder library ENCFLAC.DL3 v1.00
  VLSI Solution 2017-03-29 HH

  This library creates FLAC files from a PCM input.

  To use this device driver, copy the file ENCFLAC.DL3 to the SYS/
  folder of your VS1005 boot device.

  For further details on how to use this library, see the
  Rec.dl3 VSOS Shell command line utility.

  This code may be used freely in any product containing one or more ICs
  by VLSI Solution.

  No guarantee is given for the usability of this code.

 */

#if 0
#define USE_DYNAMIC
#endif

/* For free support for VSIDE, please visit www.vsdsp-forum.com */

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <sysmemory.h>
#include <audio.h>
#include <time.h>
#include <kernel.h>
#include "encodervorbis.h"
#include "encFCreate.h"


struct Encoder *enc = NULL;
void (*Delete)(struct Encoder *enc) = NULL;
void fini(void);


DLLENTRY(Create)
struct Encoder *Create(register struct EncoderServices *es,
		       register u_int16 channels, register u_int32 sampleRate,
		       register u_int16 quality) {
	int i;

	if (enc = EncFlacCreate(es, channels, sampleRate, quality)) {
		// Remember original Delete() function address
		Delete = enc->Delete;
		// Grab Delete() so that we can do our own clean-up
		enc->Delete = (void *)fini;
	}
	return enc;
}

void fini(void) {
	if (enc) {
		if (Delete) {
			Delete(enc);
			Delete = NULL;
		}
		enc = NULL;
	}
#ifdef USE_DYNAMIC
	UnloadEncF();
#endif
}



#ifdef USE_DYNAMIC
#define MAX_WINF_BITS 11
#define MAX_WINF_SIZE (1<<MAX_WINF_BITS)

DLLENTRY(preloadResult)
u_int16 preloadResult;

int preload(void) {
	return AllocMemXY(2*MAX_WINF_SIZE, MAX_WINF_SIZE); 
}
#endif
