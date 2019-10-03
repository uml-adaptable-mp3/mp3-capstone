/*

  PowerOff - Turns the power off the VS1005 DevBoard

  Version 1.00 - 2015-12-03 Henrik Herranen

  Thanks goes to VSDSP Forum member Passionate Jaguar for part
  of this code.

*/
#include <vo_stdio.h>
#include <volink.h>     /* Linker directives like DLLENTRY */
#include <apploader.h>  /* RunLibraryFunction etc */
#include <timers.h>
#include <libaudiodec.h>
#include <vsostasks.h>
#include <consolestate.h>
#include <ctype.h>
#include <uimessages.h>
#include <string.h>
#include <stdlib.h>
#include <audio.h>
#include <aucommon.h>
#include <limits.h>
#include <sysmemory.h>
#include <vo_fat.h>
#include <power.h>
#include <kernel.h>

void SetPWMLevel(int level) {
  USEY(PWM_FRAMELEN) = 255; // pulse end position        	
  USEY(PWM_PULSELEN) = level;  // pulse start position 0,1 disable
}

ioresult main(char *parameters) {
  /* Switch LCD off */
  SetPWMLevel(0);
  Forbid();
  /* Disable many devices */
  SetPower((PCL_SDCARD | PCL_USB_A | PCL_USB_AB | PCL_USB_5V), 0);
  /* Wait for 100 ms */
  Delay(100);
  /* Make sure regulators are clocked in. */
  PERIP(REGU_CF) &= ~(REGU_CF_REGCK);
  PERIP(REGU_CF) |= (REGU_CF_REGCK);
  PERIP(REGU_CF) &= ~(REGU_CF_REGCK);   
  /* Power off the board. */
  PowerOff();

  /* We should really never get this far. But just in case we do, this part
     will repeatedly display "0123456789", one character at the time
     every 100 ms. If you see it, you have a problem. */
  while (1) {
    static int n = '0';
    putchar(n++);
    if (n > '9') {
      n = '0';
    }
    Delay(100);
  }
}
