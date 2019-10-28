#ifndef FTPITCH_H
#define FTPITCH_H

#include <vo_stdio.h>
#include <vstypes.h>
#include <aucommon.h>



#ifndef ASM

//extern FILE *stdaudioin_orig, *stdaudioout_orig, *forwardAudio;

extern SIMPLE_FILE audioFile;

auto ioresult SetSampleRate(void);
int MyDesignRoom(register struct RoomReverb __mem_y *room);

#endif /* !ASM */

#endif /* !FTPITCH_H */
