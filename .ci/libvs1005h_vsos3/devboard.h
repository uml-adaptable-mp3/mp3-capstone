/// \file devboard.h Definitions for VS1005 developer board v1.0

#ifndef DEVBOARD_H
#define DEVBOARD_H

extern u_int16 powerControlLatchState;
void DelayL (volatile u_int32 i);
void WritePCL(u_int16 value);		
void SetPowerOn(u_int16 mask);
void SetPowerOff(u_int16 mask);

#endif
