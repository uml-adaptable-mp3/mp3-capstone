#ifndef AUXSPD_H
#define AUXSPD_H

#include <vstypes.h>
#include <aucommon.h>

#ifndef ASM

void AuiSpdCyclicFunc(register struct CyclycNode *cyclicNode);

extern struct AuiSpdRegisters {
  u_int32 fmCcf;
  u_int16 anaCf3;
} auiSpdRegisters[1];

extern struct AudioIn audioIn;
extern struct AudioOut audioOut;
extern SIMPLE_FILE audioFile;

IOCTL_RESULT AudioIoctl(register __i0 VO_FILE *self, s_int16 request,
			IOCTL_ARGUMENT argp);
extern IOCTL_RESULT (*ioctl_orig)(register __i0 VO_FILE *self,
			   s_int16 request, IOCTL_ARGUMENT arg);
ioresult AudioClose(void);

void SrxInterruptVector(void);
void StxInterruptVector(void);

#endif /* !ASM */

#endif /* !AUXSPD_H */
