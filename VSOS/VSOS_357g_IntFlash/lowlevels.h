#ifndef LOWLEVELS_H
#define LOWLEVELS_H

void WriteToProgramRam(register __i0 addr, register __reg_a u_int32 text);
void WriteToProgramRam16(register __i0 addr, register __a0 u_int16 text1,  register __a1 u_int16 text2);
void WriteToProgramRamSwapped(register __i0 addr, register __reg_a u_int32 text);
void WriteToProgramRam16Swapped(register __i0 addr, register __a0 u_int16 text2,  register __a1 u_int16 text1);
u_int32 ReadFromProgramRam(register __i0 u_int16 addr);

#endif