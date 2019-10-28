#ifndef DEV_HW_SPI_H
#define DEV_HW_SPI_H

#define CSPIN_USE_IOCHANNEL 0xffffu
#define CSPIN_USE_INTERNAL 0x0fffu


/// The hardware registers for SPI device
typedef struct spiPeripheralRegisterStruct {
	u_int16 config;
	u_int16 clkconfig;
	u_int16 status;
	u_int16 data;
	u_int16 fsync;
	u_int16 defaultdata;	
} spiRegisters;

/// The hardware specific info of SPI hardware port DEVICE. 
/// Contains the pointer to registers base. 
/// If csPin is not 0xffffU, it contains the GPIO pin definition for the peripheral's chip select.
/// if csPin is 0xffffU, then ioChannel contains the pattern to set to the system bus address decoder (74HC138) to select the peripheral.
typedef struct devHwSpiHardwareInfoStruct {
  /* NOTE! This structure will leak into deviceInfo[] because there are only
     6 words of space in hardwareInfo[]. Nevertheless, this is not a
     problem since this device never has a file system, so we can use
     the extra words. */
  volatile __y spiRegisters *regs;
  u_int16 csPin;
  u_int16 ioChannel; 
  u_int16 maxClockKHz;
  u_int32 ioMask;
  u_int32 peripMask;
  u_int16 hasHwResources;
} devHwSpiHwInfo;

/// Creates a hardware SPI channel to a peripheral device. 
ioresult DevHwSpiCreate(register __i0 DEVICE *dev, const void *name, u_int16 extraInfo);
/// Does nothing
ioresult DevHwSpiDelete(register __i0 DEVICE *dev);
/// Input data from the SPI.
/// Sends ones to MOSI and gets the MISO result
/// \param buf If buf is zero, the function returns a single byte read from the SPI. If buf is nonzero, then it is pointer to buffer.
/// Data read from the SPI has big endian byte order EXCEPT if the bytes amount is odd, then the last byte is stored as little-endian.
/// \param nbytes number of bytes to read
/// \param unused no meaning for this device
ioresult DevHwSpiInput(register __i0 DEVICE *dev, u_int16 unused, void *buf, u_int16 nbytes);
/// Output bytes from *buf. Words are big-endian, last odd byte is little-endian.
/// If buf is zero, outputs nbytes bytes of fixed 8-bit value value.
ioresult DevHwSpiOutput(register __i0 DEVICE *dev, u_int16 value, void *buf, u_int16 nbytes);	
/// IO control is used to control chip select
/// \param request IOCTL_START_FRAME:pull chip select low  IOCTL_END_FRAME:pull chip select high.
ioresult DevHwSpiIoctl(register __i0 DEVICE *dev, s_int16 request, char *argp);
/// Returns a pointer to a constant name of the device. Parameters buf and bufsize are not used.
const char *DevHwSpiIdentify(register __i0 void *obj, char *buf, u_int16 bufsize);

u_int16 DevHwSpiRead(register __i0 DEVICE *f, void *buf, u_int16 destinationIndex, u_int16 bytes);
u_int16 DevHwSpiWrite(register __i0 DEVICE *f, void *buf, u_int16 sourceIndex, u_int16 bytes);


#endif
