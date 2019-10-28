/*

  vs23io.c - VS23S010 Read/Write routines

*/
#include <vo_stdio.h>
#include <volink.h>     /* Linker directives like DLLENTRY */
#include <apploader.h>  /* RunLibraryFunction etc */
#include <string.h>
#include <stdlib.h>
#include <swap.h>
#include "devByteBus.h"
#include <audio.h>
#include <consolestate.h>
#include <saturate.h>
#include "vs23s010_io.h"


ioresult ReadVS23S010(register DEVICE *byteBus, register u_int16 *d, register u_int32 addr, register u_int16 bytes) {
  u_int16 addr2[2];
  if ((addr|(u_int16)bytes) & 3) {
    return S_ERROR;
  }
  addr2[0] = 0x300 | ((u_int16)(addr>>16) & 0xff);
  addr2[1] = (u_int16)addr;
  byteBus->Ioctl(byteBus, IOCTL_START_FRAME, 0);
  byteBus->Write(byteBus, addr2, 0, 5); /* Includes one dummy byte */
  byteBus->Read(byteBus, d, 0, bytes);
  byteBus->Ioctl(byteBus, IOCTL_END_FRAME, 0);
  return S_OK;
}


ioresult WriteVS23S010(register DEVICE *byteBus, register u_int16 *d, register u_int32 addr, register u_int16 bytes) {
  u_int16 addr2[2];
  if ((addr|(u_int16)bytes) & 3) {
    return S_ERROR;
  }
  addr2[0] = 0x200 | ((u_int16)(addr>>16) & 0xff);
  addr2[1] = (u_int16)addr;
  byteBus->Ioctl(byteBus, IOCTL_START_FRAME, 0);
  byteBus->Write(byteBus, addr2, 0, 4);
  byteBus->Write(byteBus, d, 0, bytes+1); /* Includes one dummy byte */
  byteBus->Ioctl(byteBus, IOCTL_END_FRAME, 0);
  return S_OK;
}
