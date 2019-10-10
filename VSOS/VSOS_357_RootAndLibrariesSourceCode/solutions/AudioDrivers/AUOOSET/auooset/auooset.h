#ifndef AUOOSET_H
#define AUOOSET_H

#include <vstypes.h>
#include <aucommon.h>

#ifndef ASM

extern struct AudioIn audioIn;
extern struct AudioOut audioOut;
extern SIMPLE_FILE audioFile;

ioresult AudioClose(void);

#endif /* !ASM */

#endif /* !AUOOSET_H */
