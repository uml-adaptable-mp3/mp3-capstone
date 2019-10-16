#ifndef RTC_H
#define RTC_H

#include <vsos.h>

#define RTC_MAGIC_ADDRESS  0x0000
#define RTC_MAGIC_DATA     0x1e0b

/* RTC cannot be found by the hardware; probably not powered */
#define RTC_NOT_FOUND	0xFFFFFFFEUL
/* RTC can be found but time has not been set */
#define RTC_NOT_SET	0xFFFFFFFFUL


u_int32 GetRtc(void);
u_int16 GetRtc128(void);
/* Returns t if ok, RTC_NOT_FOUND if RTC hardware cannot be found */
u_int32 SetRtc(register u_int32 t);
/* Write to RTC RAM memory. addr = 0..31. Note that address 0 is reserved for
   RTC magic value. Returns value it stored to the register. */
u_int16 WriteRtcMem(register u_int16 addr, register u_int16 data);
/* Read from RTC RAM memory. addr = 0..31. Note that address 0 is reserved for
   RTC magic value. */
u_int16 ReadRtcMem(register u_int16 addr);


#endif /* RTC_H */
