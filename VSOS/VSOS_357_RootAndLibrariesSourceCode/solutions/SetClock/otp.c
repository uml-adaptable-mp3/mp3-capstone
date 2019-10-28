#include <vo_stdio.h>
#include <string.h>
#include <stdlib.h>
#include <crc32.h>
#include <vs1005h.h>
#include "otp.h"
#include "fuse.h"




struct OTP otp;
extern s_int16 chipIsG;

s_int16 GetTrim(register enum trimType tType) {
 /* Set the default to highest trim so that an IC without laser fuses or
    Internal FLASH are sure to get enough voltage to run. */
  s_int16 vCoreTrim = -32768;
  u_int32 serialNumber = 0;
  u_int16 usbTrim = PERIP(UTM_TRIMRES);
  int i;

  if (!otp.rdid[0] && !otp.rdid[1] && !otp.rdid[2]) {
    ReadOTP(&otp);
  }

  for (i=0; i<7; i++) {
    u_int16 *d = otp.record[i];
    u_int16 opCode = d[0]>>13;
    if (opCode >= 1 && opCode <= 3) {
      u_int16 dd[4];
      u_int32 crc29;
      dd[0] = opCode << 13;
      dd[1] = 0;
      dd[2] = d[2];
      dd[3] = d[3];
      crc29 = CalcCrc32(0, dd, 8) & 0x1FFFFFFF;
      if (crc29 == (((u_int32)(d[0]&0x1FFF)<<16) | d[1])) {
	if (opCode == 1) {
	  vCoreTrim = (d[3]>>9) & 7;
	  if (vCoreTrim >= 4) vCoreTrim -= 8;
	  usbTrim = SwapBits0And2(d[2]);
	} else if (opCode == 2) {
	  serialNumber = ((u_int32)d[2]<<16) | d[3];
	}
      }
    }
  }

  /* 2018-04-11 specification:
     VS1005H series ICs with new serial numbers have cVoreTrim interpreted
     differently from earlier batches. */
  if (vCoreTrim == -32768) {
    vCoreTrim = 3;
  } else if (!chipIsG && serialNumber >= 2050973) {
    vCoreTrim += 2;
  }

  if (tType == ttVCore) {
    return vCoreTrim;
  } else if (tType == ttUSB) {
    return usbTrim;
  }
  return -0x8000;
}

void PrintOTP(void) {
  u_int16 fuseCrc = CalcFuseCRC();
  static const u_int16 data[8*8] = {0};
  int i;
  static const char s14[15]="              ";

  printf("Laser fuse(63:0) = 0x%04x:%04x:%04x:%04x, crc %s\n",
	 ReadFuseMulti(48, 16), ReadFuseMulti(32, 16),
	 ReadFuseMulti(16, 16), ReadFuseMulti( 0, 16),
	 fuseCrc ? "ERROR" : "ok");

  if (!otp.rdid[0] && !otp.rdid[1] && !otp.rdid[2]) {
    ReadOTP(&otp);
  }

  printf("Serial Flash:\n  RDID: manufacturer %2x (C2), type %2x (20),"
	 " density %2x (14)\n", otp.rdid[0], otp.rdid[1], otp.rdid[2]);

  if (otp.rdid[0] != 0xc2 || otp.rdid[1] != 0x20 || otp.rdid[2] != 0x14) {
    printf("    Bad RDID\n");
    goto finally;
  }

  printf("  SCUR: 0x%02x. Factory lock %s, user lock %s\n",
	 otp.scur,
	 (otp.scur & 1) ? "on" : "off",
	 (otp.scur & 2) ? "on" : "off");

  for (i=0; i<7; i++) {
    u_int16 *d = otp.record[i];
    u_int16 opCode = d[0]>>13;
    static const char *opName[8] =
      {"Invalidated", "TrimData", "SerialNumber", "N/A",
       "ThirdPartyID", "N/A", "N/A_Extended", "Unused"};
    printf("  Field %d, 0x%04x 0x%04x 0x%04x 0x%04x opCode %d = %s\n",
	   i, d[0], d[1], d[2], d[3], opCode, opName[opCode]);
    if (opCode >= 1 && opCode <= 3) {
      u_int16 dd[4];
      u_int32 crc29;
      u_int16 crcOk;
      dd[0] = opCode << 13;
      dd[1] = 0;
      dd[2] = d[2];
      dd[3] = d[3];
      crc29 = CalcCrc32(0, dd, 8) & 0x1FFFFFFF;
      crcOk = crc29 == (((u_int32)(d[0]&0x1FFF)<<16) | d[1]);
      printf("    CRC29:         0x%08lx, %s\n",
	     crc29, crcOk ? "ok" : "ERROR");
      if (crcOk) {
	u_int32 payload = ((u_int32)d[2]<<16) | d[3];
	if (opCode == 1) {
	  u_int16 usbTrim = SwapBits0And2((u_int16)(payload>>16));
	  u_int16 speed = (u_int16)(payload>>15) & 1;
	  u_int16 memDelay = (u_int16)(payload>>13) & 3;
	  u_int16 intFlash1V8 = (u_int16)(payload>>12) & 1;
	  s_int16 vCoreTrim = (u_int16)(payload>>9) & 7;
	  u_int16 customerCode = (u_int16)(payload>>5) & 15;
	  u_int16 unused = (u_int16)(payload>>0) & 31;
	  if (vCoreTrim >= 4) { /* Value is 3-bit 2's complement */
	    vCoreTrim -= 8;
	  }
	  printf("    USB Trim:      0x%04x\n"
		 "    High speed:    %s\n"
		 "    Mem Delay:     %d\n"
		 "    Int Flash:     %s V\n"
		 "    VCORE Trim:    %+d\n"
		 "    Customer code: %d\n"
		 "    Unused: %d\n",
		 usbTrim,
		 speed ? "yes" : "no",
		 memDelay,
		 intFlash1V8 ? "1.8" : "2.8",
		 vCoreTrim,
		 customerCode,
		 unused);
	} else if (opCode == 2) {
	    printf("    Serial number: 0x%08lx\n", payload);
	}
      }
    }
  }

 finally:
  {}
}
