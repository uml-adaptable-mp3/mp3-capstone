#include <vo_stdio.h>
#include <vs1005h.h>
#include <timers.h>
#include "rtc.h"

void StartAndWaitRtcReady(register u_int16 rtcCfBits) {
  PERIP(RTC_CF) = rtcCfBits;
  while (PERIP(RTC_CF));
}

u_int16 WriteRtcMem(register u_int16 addr, register u_int16 data) {
  PERIP(RTC_LOW) = 0;			/* Clear LSbs */
  PERIP(RTC_HIGH) = RTC_I_MEM_WR;	/* Set memory write instruction */
  StartAndWaitRtcReady(RTC_CF_IBUSY);	/* Wait for instruction ready */
  PERIP(RTC_LOW) = addr;		/* Set memory address */
  PERIP(RTC_HIGH) = data;		/* Set data */
  StartAndWaitRtcReady(RTC_CF_DBUSY);	/* Wait for data write to register */
  PERIP(RTC_CF) = RTC_CF_GSCK|RTC_CF_EXEC;
  while(PERIP(RTC_CF) & RTC_CF_GSCK);	/* Wait for data write to memory */
  PERIP(RTC_CF) = 0;			/* Clear execute */
}

u_int16 ReadRtcMem(register u_int16 addr) {
  PERIP(RTC_HIGH) = RTC_I_MEM_RD;	/* Set memory read instruction */
  StartAndWaitRtcReady(RTC_CF_IBUSY);	/* Wait for instruction ready */
  PERIP(RTC_LOW) = addr;		/* Set memory address */
  StartAndWaitRtcReady(RTC_CF_DBUSY);	/* Wait for address transmit ready */
  StartAndWaitRtcReady(RTC_CF_RDBUSY);	/* Wait for result */
  return PERIP(RTC_HIGH);
}

unsigned long GetRtc(void) {
  int i;
  if (ReadRtcMem(RTC_MAGIC_ADDRESS) != RTC_MAGIC_DATA) {
    WriteRtcMem(RTC_MAGIC_ADDRESS, ~RTC_MAGIC_DATA);
    if (ReadRtcMem(RTC_MAGIC_ADDRESS) != ~RTC_MAGIC_DATA) {
      return RTC_NOT_FOUND;
    }
    return RTC_NOT_SET;
  }
  PERIP(RTC_HIGH) = RTC_I_READRTC;
  StartAndWaitRtcReady(RTC_CF_IBUSY);
  StartAndWaitRtcReady(RTC_CF_RDBUSY);
  return (((u_int32)PERIP(RTC_HIGH)) << 16) | (u_int32)PERIP(RTC_LOW);
}

u_int32 SetRtc(register u_int32 t) {
  WriteRtcMem(RTC_MAGIC_ADDRESS, RTC_MAGIC_DATA);
  WriteRtcMem(RTC_MAGIC_ADDRESS, RTC_MAGIC_DATA);
  if (ReadRtcMem(RTC_MAGIC_ADDRESS) != RTC_MAGIC_DATA) {
    return RTC_NOT_FOUND;
  }

  PERIP(RTC_HIGH) = RTC_I_LOADRTC;
  StartAndWaitRtcReady(RTC_CF_IBUSY);
  PERIP(RTC_HIGH) = (u_int16)(t >> 16);
  PERIP(RTC_LOW) = (u_int16)t;
  StartAndWaitRtcReady(RTC_CF_DBUSY);
 
  PERIP(RTC_CF) = RTC_CF_EXEC;
  /* To load the RTC, RTC_DF_EXEC must be held active for more than
     one second, then released. */
  Delay((u_int16)(TICKS_PER_SEC*1.1));
  PERIP(RTC_CF) = 0;

  return t;
}

u_int16 GetRtc128(void) {
  char d;
  PERIP(RTC_HIGH) = RTC_I_DIV128;
  StartAndWaitRtcReady(RTC_CF_IBUSY);
  StartAndWaitRtcReady(RTC_CF_RDBUSY);
  return (PERIP(RTC_HIGH) >> 9) ^ 64;
}
