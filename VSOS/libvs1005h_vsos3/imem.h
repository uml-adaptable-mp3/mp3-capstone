/**
   \file imem.h Instruction memory manipulation.

   Note that string.h also contains some I-mem manipulation functions
   like memcpyii().
*/

#ifndef MANIPULATE_IMEM_H
#define MANIPULATE_IMEM_H

#ifndef ASM

#include <vstypes.h>

/**
   Read from Instruction memory.

   \param addr I-Mem address.
   \return 32-bit instruction word.
*/
u_int32 ReadIMem(register __i0 u_int16 addr);

/**
   Write to Instruction memory.

   \param addr I-Mem address.
   \param data 32-bit instruction word.
*/
void WriteIMem(register __i0 u_int16 addr, register __a u_int32 instr);
void WriteIMem16(register __i0 u_int16 addr,
		 register __a0 u_int16 instrMSW,
		 register __a1 u_int16 instrLSW);
void WriteIMemSwapped(register __i0 u_int16 addr,
		      register __reg_a u_int32 text);
void WriteIMem16Swapped(register __i0 u_int16 addr,
			register __a0 u_int16 instrLSW,
			register __a1 u_int16 instrMSW);

#endif /* !ASM */

#endif /* !MANIPULATE_IMEM_H */
