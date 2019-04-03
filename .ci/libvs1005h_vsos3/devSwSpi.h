/// \file devSwSpi.h Calling interface for Software SPI Port driver.
/// \author Panu-Kristian Poiksalo, VLSI Solution OY
#ifndef DEV_SW_SPI_H
#define DEV_SW_SPI_H

/// Pin definitions for SPI port
/// This structure defines which I/O pins shall be used for a (software) SPI port, which is created. 
/// The pins are defined as 8-bit entities, which high nibble corresponds to a GPIO controller number
/// and the low bit corresponds to a pin number. e.g. 0x23 means GPIO2.3: GPIO controller 2 pin 3.
typedef struct spipinsStruct {
	u_int16 miso;
	u_int16 mosi;
	u_int16 sclk;
	u_int16 xcs;
} SpiPins;

/// The hardware specific info of SPI port DEVICE. Contains the pin definitions.
typedef struct devSwSpiHardwareInfoStruct {
	SpiPins pins;
} devSwSpiHwInfo;

/// Creates a software SPI device. 
/// \todo: pin and speed config
ioresult DevSwSpiCreate (register __i0 DEVICE *dev, const void *name, u_int16 extraInfo);

/// Returns a pointer to a constant name of the device. Parameters buf and bufsize are not used.
const char* DevSwSpiIdentify(register __i0 void *obj, char *buf, u_int16 bufsize);

/// IO control is used to control chip select
/// \param request IOCTL_START_FRAME:pull chip select low  IOCTL_END_FRAME:pull chip select high.
ioresult DevSwSpiIoctl(register __i0 DEVICE *dev, s_int16 request, char *argp);

/// Input data from the SPI.
/// Sends ones to MOSI and gets the MISO result
/// \param buf If buf is zero, the function returns a single byte read from the SPI. If buf is nonzero, then it is pointer to buffer.
/// \param bytes number of bytes to read
u_int16 DevSwSpiRead(register __i0 DEVICE *dev, void *buf, u_int16 destinationIndex, u_int16 bytes);

/// Output bytes from *buf.
/// If buf is zero, outputs nbytes bytes of fixed 8-bit value taken from parameter "sourceIndex".
u_int16 DevSwSpiWrite(register __i0 DEVICE *dev, void *buf, u_int16 sourceIndex, u_int16 bytes);

#endif
