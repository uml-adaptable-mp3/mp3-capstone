#ifndef DISASSEMBLE_H
#define DISASSEMBLE_H

#ifndef ASM

#include <vstypes.h>

void PrintDisassembled(register u_int16 addr, register u_int32 d, u_int16 fast);

#endif

#endif /* !DISASSEMBLE_H */
