#ifndef PRINT_BITS_H
#define PRINT_BITS_H

#ifndef ASM

#include <vstypes.h>

extern int fastMode;

void PrintBitsY(register const char *symName,
		register u_int16 low, register u_int16 high,
		register u_int16 bitMode,
		register s_int16 verbose);
void SetValue(register char *s, register u_int16 memType,
	      register s_int16 verbose);
void PrintBitsStr(register char *s, register u_int16 memType,
		  register u_int16 bitMode,
		  register s_int16 verbose);

#endif

#endif /* !PRINT_BITS_H */
