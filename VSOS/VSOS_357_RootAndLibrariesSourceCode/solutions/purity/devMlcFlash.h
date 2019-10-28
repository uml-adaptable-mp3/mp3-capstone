#ifndef DEVMLCFLASH_H
#define DEVMLCFLASH_H

#include <vsos.h>

typedef struct DevMlcFlashHardwareInfo {
  u_int32 totalBlocks;
  u_int16 sectorsPerBlock;
  u_int16 sectorsPerAire;
} DevMlcFlashHwInfo;

extern DEVICE devMlcFlash;
ioresult DevMlcFlashCreate(register __i0 DEVICE *dev, void *name, u_int16 extraInfo);

typedef struct {
	u_int16 flags;
	u_int32 lsn;
	u_int16 ecc[5];
} StdMeta2;

IOCTL_RESULT DevMlcFlashIoctl(register __i0 DEVICE *dev, s_int16 request, IOCTL_ARGUMENT arg);
const char *DevMlcFlashIdentify(register __i0 void *dev, char *buf, u_int16 bufsize);
#endif
