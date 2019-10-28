#ifndef AUSPDIF_H
#define AUSPDIF_H

#include <vo_stdio.h>
#include <vstypes.h>
#include <aucommon.h>

#ifndef ASM

struct AudioOutSPDif {
  struct AudioOut ao;
  u_int16 volMult[2];
  u_int16 volVal;
} audioOutSPDif;

extern struct AudioOutSPDif audioOutSPDif;
extern SIMPLE_FILE audioFile;
extern s_int32 lockSampleRate;
IOCTL_RESULT AudioIoctl(register __i0 VO_FILE *self, s_int16 request,
			IOCTL_ARGUMENT argp);
extern IOCTL_RESULT (*ioctl_orig)(register __i0 VO_FILE *self,
			   s_int16 request, IOCTL_ARGUMENT arg);

ioresult AudioClose(void);

#endif /* !ASM */

#endif /* !AUSPDIF_H */
