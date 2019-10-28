#ifndef VS23S0X0_MISC
#define VS23S0X0_MISC

#include <vstypes.h>

s_int32 InitVS23S0x0(void);
s_int16 ReadVS23S0x0(register u_int16 *d, register u_int32 addr, register u_int16 words);
s_int16 WriteVS23S0x0(register u_int16 *d, register u_int32 addr, register u_int16 words);

#endif /* !VS23S0X0_MISC */
