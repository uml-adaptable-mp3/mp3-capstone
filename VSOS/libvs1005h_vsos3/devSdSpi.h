#ifndef DEVSDCARDSPI_H
#define DEVSDCARDSPI_H

typedef struct devSdSpiHardwareInfoStruct {
	u_int16 hcShift;
	DEVICE *ph;
	s_int32 multipleReadNextBlock;
} devSdSpiHwInfo;

ioresult DevSdSpiCreate (register __i0 DEVICE *dev, const void *name, u_int16 extraInfo);
ioresult DevSdSpiInput(register __i0 DEVICE *dev, u_int16 port, void *buf, u_int16 bytes);
ioresult DevSdSpiOutput(register __i0 DEVICE *dev, u_int16 port, void *buf, u_int16 bytes);	
ioresult DevSdSpiBlockRead(register __i0 DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data);
ioresult DevSdSpiBlockWrite(register __i0 DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data);
ioresult DevSdSpiDelete(register __i0 DEVICE *dev);
const char* DevSdSpiIdentify(register __i0 void *obj, char *buf, u_int16 bufsize);
ioresult DevSdSpiIoctl(register __i0 DEVICE *dev, s_int16 request, char *argp);

#endif
