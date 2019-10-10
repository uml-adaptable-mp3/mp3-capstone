
#ifndef DevUMass_H
#define DevUMass_H

ioresult DevUMassIoctl(register __i0 DEVICE *dev, s_int16 request, char *argp);
ioresult DevUMassBlockRead(register __i0 DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data);
ioresult DevUMassBlockWrite(register __i0 DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data);
ioresult DevUMassCreate (register __i0 DEVICE *dev, void *name, u_int16 extraInfo);
//ioresult DevUMassCreate (register __i0 DEVICE *dev, void *name, u_int16 extraInfo);
u_int16  DevUMassProbe();
ioresult CreateUsbFlashDisk(DEVICE *usbFlash, u_int16 lun); /* Probe MUST be called before this */

#endif
