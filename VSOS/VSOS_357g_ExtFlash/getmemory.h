#ifndef GETMEMORY_H
#define GETMEMORY_H

#include <vstypes.h>
#include <sysmemory.h>

u_int16 GetMemory(u_int16 page, u_int16 address, u_int16 sizeWords, u_int16 align);

#endif