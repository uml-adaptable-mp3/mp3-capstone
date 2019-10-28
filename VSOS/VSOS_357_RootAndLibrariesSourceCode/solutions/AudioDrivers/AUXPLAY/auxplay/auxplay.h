#ifndef AUXPLAY_H
#define AUXPLAY_H

#include <vstypes.h>
#include <aucommon.h>

#ifndef ASM

extern u_int16 (*stdaudioout_write_orig)(register __i0 VO_FILE *self, void *buf,
				  u_int16 sourceIndex, u_int16 bytes);
extern IOCTL_RESULT (*stdaudioin_ioctl_orig)(register __i0 VO_FILE *self,
				      s_int16 request, IOCTL_ARGUMENT arg);
char *Identify(register __i0 void *self, char *buf, u_int16 bufSize);

u_int16 AudioWrite(register __i0 VO_FILE *self, void *buf,
		   u_int16 sourceIndex, u_int16 bytes);
IOCTL_RESULT AudioIoctl(register __i0 VO_FILE *self, s_int16 request,
			IOCTL_ARGUMENT argp);


ioresult AudioClose(void);

#endif /* !ASM */

#endif /* !AUXPLAY_H */
