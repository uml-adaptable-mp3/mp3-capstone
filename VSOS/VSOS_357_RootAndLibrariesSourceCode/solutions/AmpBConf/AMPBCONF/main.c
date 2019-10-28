/// \file main.c Driver needed to get VS1005 Amp Board work with SD card.
/// \author Henrik Herranen, VLSI Solution Oy
#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <kernel.h>
#include <timers.h>
#include <power.h>
#include <imem.h>

//void (*RomStart)(void) = (void (*)(void)) 0x8000;
__mem_y u_int16 origSetPower = 0;


void AmpBoardSetPower(register u_int16 mask, register u_int16 onoff) {
  if (mask == PCL_SDCARD) {
#if 0
    printf("PCL_SDCARD %d\n", onoff);
#endif
    GpioSetPin(0x07, onoff);
  }
}


/*

  VS1005 AmpBoard pins:

  GPIO0_7 = 3V3 regulator 1:on (spdif+sdcard)
  GPIO1_5 = Amp mute off 1:off
  GPIO0_8 = Amp gain 20dB=1
  GPIO1_6 = Amp standby off:1
  GPIO2_0:1 = RCA input range select 10:1.5Vrms, 00:3Vrms
  GPIO2_2:3 = XLR input range select 10:4Vrms, 00:12Vrms

*/

DLLENTRY(init)
ioresult init(char *paramStr) {
  origSetPower = (u_int16)(ReadIMem((u_int16)SetPower) >> 6);
#if 0
  printf("SetPower 0x%04x = %08lx\n", SetPower, ReadIMem((u_int16)SetPower));
#endif
  SetHandler(SetPower, AmpBoardSetPower);
#if 0
  printf("SetPower 0x%04x = %08lx\n", SetPower, ReadIMem((u_int16)SetPower));
  printf("Orig %04x\n", origSetPower);
#endif
  ResetSdCard();
  return S_OK;
}


DLLENTRY(fini)
void fini(void) {
  SetHandler(SetPower, (void *)origSetPower);
}
