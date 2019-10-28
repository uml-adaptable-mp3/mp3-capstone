/// Functions for staring the VSOS on VS1005G Developer Board.

#ifndef VSOS_VS1005G_H
#define VSOS_VS1005G_H

// These are allocated in VS1005F ROM

u_int16 MmcCommand(s_int16 cmd, u_int32 arg, DEVICE *ph) ;
void ListDevices();

extern u_int16 osMemory[]; // Allocate memory for file buffers
extern DEVICE console;
extern DEVICE devSwSpi;
extern DEVICE devSdSpi;
extern DEVICE console;
void StartOS3();
auto void intOS(void);
extern u_int16 forbidCount;
__y void *AllocMemYPatch(size_t size, size_t align);
void ApplyGFixes();

#endif
