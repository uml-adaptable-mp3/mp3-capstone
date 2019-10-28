#ifndef VS23S010_IO_H
#define VS23S010_IO_H

#include <vstypes.h>
#include <vo_stdio.h>

ioresult ReadVS23S010(register DEVICE *byteBus, register u_int16 *d, register u_int32 addr, register u_int16 bytes);
ioresult WriteVS23S010(register DEVICE *byteBus, register u_int16 *d, register u_int32 addr, register u_int16 bytes);

#endif /* !VS23S010_IO_H */
