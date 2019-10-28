/// \file main.c Turn VS1005 Amp Board output on.
/// \author Henrik Herranen, VLSI Solution Oy
#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <kernel.h>
#include <timers.h>
#include <devboard.h>



/*

  VS1005 AmpBoard pins:

  GPIO0_7 = 3V3 regulator 1:on (spdif+sdcard)
  GPIO1_5 = Amp mute off 1:off
  GPIO0_8 = Amp gain 20dB=1
  GPIO1_6 = Amp standby off:1
  GPIO2_0:1 = RCA input range select 10:1.5Vrms, 00:3Vrms
  GPIO2_2:3 = XLR input range select 10:4Vrms, 00:12Vrms

*/

ioresult main(char *paramStr) {
  GpioSetPin(0x16,0); //Class-D amp standby on
  GpioSetPin(0x07,1); //3V3 regulator ON(spdif+sdcard)
  GpioSetPin(0x16,1); //Class-D amp standby off
  GpioSetPin(0x08,1); //Class-D amp gain 20dB
  Delay(200);
  GpioSetPin(0x15,1); //Class-D amp mute off
  Delay(100); 

  return S_OK;
}
