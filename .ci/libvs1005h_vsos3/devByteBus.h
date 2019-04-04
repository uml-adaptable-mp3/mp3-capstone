/// \file devByteBus.h Byte-wide Bus device

#ifndef DEV_BYTE_BUS_H
#define DEV_BYTE_BUS_H

/// The hardware registers for ByteBus device
typedef struct byteBusPeripheralRegisterStruct {
	u_int16 conf;
	u_int16 cf;
	u_int16 pntr;
	u_int16 len;
} byteBusRegisters;

/// The hardware specific info of ByteBus hardware port DEVICE. 
/// Contains the pointer to registers base. 
/// If csPin is not 0xffffU, it contains the GPIO pin definition for the peripheral's chip select.
/// if csPin is 0xffffU, then ioChannel contains the pattern to set to the system bus address decoder (74HC138) to select the peripheral.
typedef struct devByteBusHardwareInfoStruct {
  volatile __y byteBusRegisters *regs;
  u_int16 csPin;
  u_int16 ioChannel; 
  u_int16 clockDivider;
} devByteBusHwInfo;

/// Creates a hardware ByteBus channel to a peripheral device. 
ioresult DevByteBusCreate (register __i0 DEVICE *dev, void *name, u_int16 extraInfo);
/// Does nothing
ioresult DevByteBusDelete(register __i0 DEVICE *dev);
/// Input data from the ByteBus.
/// \param buf If buf is zero, the function returns a single byte read from the ByteBus. If buf is nonzero, then it is pointer to buffer.
/// Data read from the ByteBus has big endian byte order EXCEPT if the bytes amount is odd, then the last byte is stored as little-endian.(?)
/// \param nbytes number of bytes to read
/// \param unused no meaning for this device
//ioresult DevByteBusInput(register __i0 DEVICE *dev, u_int16 unused, void *buf, u_int16 nbytes);
/// Output bytes from *buf. Words are big-endian, last odd byte is little-endian.
/// If buf is zero, outputs nbytes bytes of fixed 8-bit value value.
//ioresult DevByteBusOutput(register __i0 DEVICE *dev, u_int16 value, void *buf, u_int16 nbytes);	
/// IO control is used to control chip select
/// \param request IOCTL_START_FRAME:pull chip select low  IOCTL_END_FRAME:pull chip select high.
ioresult DevByteBusIoctl(register __i0 DEVICE *dev, s_int16 request, char *argp);
/// Returns a pointer to a constant name of the device. Parameters buf and bufsize are not used.
char* DevByteBusIdentify(register __i0 void *obj, char *buf, u_int16 bufsize);

u_int16  DevByteBusRead(register __i0 DEVICE *dev, void *buf, u_int16 destinationIndex, u_int16 bytes); ///< Read bytes to *buf, at byte index

u_int16  DevByteBusWrite(register __i0 DEVICE *dev, void *buf, u_int16 sourceIndex, u_int16 bytes); ///< Write bytes from *buf at byte index



#endif
