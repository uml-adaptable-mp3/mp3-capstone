/*
  Ogg Vorbis Encoder library ENCVORB.DL3 v1.01
  VLSI Solution 2015-10-15 HH

  This library creates Ogg Vorbis files from a PCM input.

  To use this device driver, copy the file ENCVORB.DL3 to the SYS/
  folder of your VS1005 boot device.

  For further details on how to use this library, see the
  Simple MP3 Encoder application or the Rec.dl3 VSOS Shell command
  line utility.

  This code may be used freely in any product containing one or more ICs
  by VLSI Solution.

  No guarantee is given for the usability of this code.

 */

/* For free support for VSIDE, please visit www.vsdsp-forum.com */

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <sysmemory.h>
#include <audio.h>
#include <time.h>
#include <kernel.h>
#include "encodervorbis.h"
#include "encVCreate.h"


struct Encoder *enc = NULL;
void (*Delete)(struct Encoder *enc) = NULL;
void fini(void);


DLLENTRY(Create)
struct Encoder *Create(register struct EncoderServices *es,
		       register u_int16 channels, register u_int32 sampleRate,
		       register u_int16 quality) {
	u_int32 t1 = time(NULL); // Gives meaningful values only if RTC active.
	u_int32 t2 = ReadTimeCount(); // Time since last boot in 1/1000s
	int i;

	// Bit-reverse the other value to make them appear more random
	for (i=0; i<32; i++) {
	  	t2 ^= ((t1 >> i) & 1) << (31-i);
	}
	encVStreamSerialNumber = t2;

	if (enc = EncVorbisCreate(es, channels, sampleRate, quality)) {
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
	UnloadEncV();
}

int main(void) {
	printf("encvorb.dl3 main() called, this is probably an error\n");
}

#define MAX_WINF_BITS 11
#define MAX_WINF_SIZE (1<<MAX_WINF_BITS)

DLLENTRY(preloadResult)
u_int16 preloadResult;

int preload(void) {
	return AllocMemXY(2*MAX_WINF_SIZE, MAX_WINF_SIZE); 
}
