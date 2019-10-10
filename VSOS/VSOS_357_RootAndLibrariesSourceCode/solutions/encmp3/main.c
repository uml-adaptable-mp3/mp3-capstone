/*
  MP3 Encoder library ENCMP3.DL3 v1.00
  VLSI Solution 2014-10-06 HH

  This library creates MP3 files from a PCM input.

  To use this device driver, copy the file ENCMP3.DL3 to the SYS
  folder of your VS1005 boot device.

  For further details on how to use this library, see the
  Simple MP3 Encoder application.

  This code may be used freely in any product containing one or more ICs
  by VLSI Solution.

  No guarantee is given for the usability of this code.

 */

/* For free support for VSIDE, please visit www.vsdsp-forum.com */

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <sysmemory.h>
#include <kernel.h>
#include "encodermp3.h"


struct Encoder *enc = NULL;
void (*Delete)(struct Encoder *enc) = NULL;
void fini(void);


DLLENTRY(Create)
struct Encoder *Create(register struct EncoderServices *es,
		       register u_int16 channels, register u_int32 sampleRate,
		       register u_int16 quality) {
  if (enc = EncMp3Create(es, channels, sampleRate, quality)) {
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
}
