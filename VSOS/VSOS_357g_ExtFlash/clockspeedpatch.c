#include <vstypes.h>
#include <clockspeed.h>
#include <vs1005g.h>
#include "clockspeedpatch.h"
#include <vo_stdio.h>


auto void DelayMicroSecPatch(u_int32 microSec) {
	BusyDelay4x(microSec * (u_int32)((u_int16)((s_int32)clockSpeed.cpuClkHz >> 22)));
}

/* GetDivider() returns cpuKHz / speedKHz / 2 - 1, rounded upwards. */
u_int16 GetDivider(register u_int16 maxSpeedKHz, register u_int16 maxVal) {
  u_int16 div = 0;
  u_int32 maxSpeedHzTimes2 = 2000L * maxSpeedKHz;
  static u_int32 lastCpu = 0;
  if (maxSpeedHzTimes2 <= clockSpeed.cpuClkHz) {
    div = (u_int16)((clockSpeed.cpuClkHz + maxSpeedHzTimes2 - 1) /
		    maxSpeedHzTimes2) - 1;
    if (lastCpu != clockSpeed.cpuClkHz) {
      lastCpu = clockSpeed.cpuClkHz;
    }
    if (div > maxVal) {
      return maxVal;
    }
  }
  return div;
}
