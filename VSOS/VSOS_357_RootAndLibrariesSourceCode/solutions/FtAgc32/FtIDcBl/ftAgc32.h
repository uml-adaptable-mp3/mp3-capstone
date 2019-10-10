#ifndef FTEQU_H
#define FTEQU_H

#include <vo_stdio.h>
#include <vstypes.h>
#include <aucommon.h>



#ifndef ASM

//extern FILE *stdaudioin_orig, *stdaudioout_orig, *forwardAudio;

extern SIMPLE_FILE audioFile;
extern struct Agc32 agc32;

#endif /* !ASM */

#endif /* !FTEQU_H */
