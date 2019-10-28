#include <vo_stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vs1005h.h>
#include "vs23s010_io.h"
#include "ftRev.h"
#include "devByteBus.h"

DEVICE byteBus;

s_int32 InitVS23S0x0(void) {
  s_int32 vs23Size = 0;
  int i;
  static const devByteBusHwInfo myHwInfo = {
    (void __mem_y *)NF_CF,	/* Base register */
#if 0
    0x21,	/* csPin = TDI */
#else
    0x20,	/* csPin = TMS */
#endif
    0,	/* ioChannel (0 since csPin != 0xFFFF) */
    15000,	/* maxClockKHz */
  };
  u_int16 smallBuf[4];
  DevByteBusCreate(&byteBus, &myHwInfo, 0);

  for (i=4; i>0; i--) {
    smallBuf[0] = i;
    smallBuf[1] = 0xf4de;
    smallBuf[2] = 0x17f3;
    smallBuf[3] = 0x1234;
    WriteVS23S010(&byteBus, smallBuf, i*(128L*1024)-8, 8);
  }
  vs23Size = 0;
  for (i=1; i<=4; i++) {
    ReadVS23S010(&byteBus, smallBuf, i*(128L*1024)-8, 8);
    if (smallBuf[0] == i && *((u_int32 *)(smallBuf+1)) == 0x17f3f4de) {
      vs23Size = i*131072;
    }
  }

  return vs23Size;
}

s_int16 ReadVS23S0x0(register u_int16 *d, register u_int32 addr, register u_int16 words) {
  return ReadVS23S010(&byteBus, d, addr*2, words*2);
}

s_int16 WriteVS23S0x0(register u_int16 *d, register u_int32 addr, register u_int16 words) {
  return WriteVS23S010(&byteBus, d, addr*2, words*2);
}

