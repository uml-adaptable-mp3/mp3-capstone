#ifndef OTP_H
#define OTP_H

#include <vstypes.h>

struct OTP {
  u_int16 rdid[3];
  u_int16 scur;
  u_int16 record[8][4];
};

enum trimType {
  ttVCore,
  ttUSB
};

/* Read VS1005h One-Time Programmable FLASH areas */
void ReadOTP(register struct OTP *otp);
void PrintOTP(void);
s_int16 GetTrim(register enum trimType tType);
s_int16 GetVS1005SubType(void);

#endif /* !OTP_H */
