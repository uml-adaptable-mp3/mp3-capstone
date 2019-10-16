#ifndef __DEBUG_LIB_H__
#define __DEBUG_LIB_H__

#include <vstypes.h>

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 0
#endif

#if DEBUG_LEVEL > 0
void puthex(u_int16 a);
void puthex6(u_int32 a);

void putWrite(u_int32 sector);
void putRead(u_int32 sector);
void putFree(u_int32 sector);
void putFlush(void);
#endif

#endif /* __DEBUG_LIB_H__ */
