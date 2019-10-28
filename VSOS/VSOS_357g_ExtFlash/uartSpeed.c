#include <vo_stdio.h>
#include <vstypes.h>
#include <clockspeed.h>
#include <vs1005g.h>
#include <audio.h>
#include "uartSpeed.h"


auto void SetUartSpeed(u_int32 bitsPerSecond) {
  int shiftRight = clockSpeed.xtalPrescaler + clockSpeed.div2 +
    clockSpeed.div256*8;
  u_int16 extClock4KHz = clockSpeed.extClkKHz >> 2;
  u_int32 uartByteSpeed = ((bitsPerSecond + 5)/ 10) << shiftRight;

  while (uartByteSpeed > 65535UL) {
    extClock4KHz >>= 1;
    uartByteSpeed >>= 1;
  }
  PERIP(UART_DIV) = UartDivider(extClock4KHz, (u_int16)uartByteSpeed);
}
