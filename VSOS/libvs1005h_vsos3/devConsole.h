/// \file devConsole.h Byte-wide Bus device

/// \todo this file is just a copy of devbytebus, todo:modify

#ifndef DEV_CONSOLE_H
#define DEV_CONSOLE_H

ioresult ConsoleInput (register __i0 DEVICE *dev, u_int16 port, void *buf, u_int16 bytes); ///< Stream Read, IO port input
ioresult ConsoleOutput (register __i0 DEVICE *dev, u_int16 port, void *buf, u_int16 bytes); ///< Stream Write, IO port output

extern const FILEOPS consoleOperations;
extern const FILESYSTEM consoleFS;
/// The hardware specific info of Console hardware port DEVICE. 
/// Contains the pointer to registers base. 
/// If csPin is not 0xffffU, it contains the GPIO pin definition for the peripheral's chip select.
/// if csPin is 0xffffU, then ioChannel contains the pattern to set to the system bus address decoder (74HC138) to select the peripheral.
typedef struct devConsoleHardwareInfoStruct {
  u_int16 dummy;
} devConsoleHwInfo;

/// Creates a hardware Console channel to a peripheral device. 
ioresult DevConsoleCreate (register __i0 DEVICE *dev, void *name, u_int16 extraInfo);
/// Does nothing
ioresult DevConsoleDelete(register __i0 DEVICE *dev);
/// Input data from the Console.
/// \param buf If buf is zero, the function returns a single byte read from the Console. If buf is nonzero, then it is pointer to buffer.
/// Data read from the Console has big endian byte order EXCEPT if the bytes amount is odd, then the last byte is stored as little-endian.(?)
/// \param nbytes number of bytes to read
/// \param unused no meaning for this device
ioresult DevConsoleInput(register __i0 DEVICE *dev, u_int16 unused, void *buf, u_int16 nbytes);
/// Output bytes from *buf. Words are big-endian, last odd byte is little-endian.
/// If buf is zero, outputs nbytes bytes of fixed 8-bit value value.
ioresult DevConsoleOutput(register __i0 DEVICE *dev, u_int16 value, void *buf, u_int16 nbytes);	
/// IO control is used to control chip select
/// \param request IOCTL_START_FRAME:pull chip select low  IOCTL_END_FRAME:pull chip select high.
ioresult DevConsoleIoctl(register __i0 DEVICE *dev, s_int16 request, char *argp);
/// Returns a pointer to a constant name of the device. Parameters buf and bufsize are not used.
char* DevConsoleIdentify(register __i0 void *obj, char *buf, u_int16 bufsize);


/// Create a visual frame for the console (for booting)
void ConsoleFrame(const char *caption);
/// Clear the console with current lcd0 background color and put cursor to home position
void ConsoleClear();
#endif
