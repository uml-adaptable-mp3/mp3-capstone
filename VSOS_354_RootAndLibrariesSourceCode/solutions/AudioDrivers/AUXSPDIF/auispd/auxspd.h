#ifndef AUXSPD_H
#define AUXSPD_H

#include <vstypes.h>
#include <aucommon.h>

#ifndef ASM

void AuiSpdCyclicFunc(register struct CyclycNode *cyclicNode);

extern struct AuiSpdRegisters {
  u_int32 fmCcf;
  u_int16 anaCf3;
} auiSpdRegisters[2];

extern struct AudioIn audioIn;
extern struct AudioOut audioOut;
extern SIMPLE_FILE audioFile;

ioresult AudioClose(void);

#endif /* !ASM */

#endif /* !AUXSPD_H */
