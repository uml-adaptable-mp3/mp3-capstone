#ifndef DEVSDCARD_HW_H
#define DEVSDCARD_HW_H

#ifndef ASM

#include <vstypes.h>

typedef struct devSdHwHardwareInfoStruct {
  u_int16 hcShift;
  DEVICE *ph;
  s_int32 multipleReadNextBlock;
} devSdHwHwInfo;

ioresult DevSdHwCreate(register __i0 DEVICE *dev, const void *name,
		       u_int16 extraInfo);
ioresult DevSdHwInput(register __i0 DEVICE *dev, u_int16 port, void *buf,
		      u_int16 bytes);
ioresult DevSdHwOutput(register __i0 DEVICE *dev, u_int16 port, void *buf,
		       u_int16 bytes);	
ioresult DevSdHwBlockRead(register __i0 DEVICE *dev, u_int32 firstBlock,
			  u_int16 blocks, u_int16 *data);
ioresult DevSdHwBlockWrite(register __i0 DEVICE *dev, u_int32 firstBlock,
			   u_int16 blocks, u_int16 *data);
ioresult DevSdHwDelete(register __i0 DEVICE *dev);
const char *DevSdHwIdentify(register __i0 void *obj, char *buf,
			    u_int16 bufsize);
ioresult DevSdHwIoctl(register __i0 DEVICE *dev, s_int16 request,
		      char *argp);

#endif /* !ASM */

#endif /* !DEVSDCARD_HW_H */
