#ifndef GETMEMORY_H
#define GETMEMORY_H

#include <vstypes.h>
#include <sysmemory.h>

extern u_int16 highestISoFar;

#define GetMemoryInit() (highestISoFar = 0)
u_int16 GetMemory(u_int16 page, u_int16 address, u_int16 sizeWords, u_int16 align);

#endif
