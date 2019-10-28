#ifndef __VS1005_FUSE_H__
#define __VS1005_FUSE_H__

#include <vstypes.h>

#define FUSE_REG 0xFC3A
#define FUSE_RESULT_BIT 6

#define FUSE_SERIAL_B		32	/* 32 bits, Serial number */
#define FUSE_USB_UTMTRIM_B	16	/* 16 bits, Write to UTM_TRIMRES */
#define FUSE_SPEED_B		15	/*  1 bits, 1:120 MHz, 0:<120 MHz */
#define FUSE_MEM_DELAY_B	13	/*  2 bits, 0=max delay, 3=min delay */
#define FUSE_INT_FLASH_B	12	/*  1 bits, 0=2.8V, 1=1.8V */
#define FUSE_VCORE_B		 9	/*  3 bits, Trim for 1.9V, signed */
#define FUSE_CUSTOMER_B		 5	/*  4 bits, Customer code */
#define FUSE_CHECKSUM_B		 0	/*  5 bits, Fuse checksum */

#define FUSE_SERIAL_BITS	32
#define FUSE_USB_UTMTRIM_BITS	16
#define FUSE_SPEED_BITS		 1
#define FUSE_MEM_DELAY_BITS	 2
#define FUSE_INT_FLASH_BITS	 1
#define FUSE_VCORE_BITS		 3
#define FUSE_CUSTOMER_BITS	 4
#define FUSE_CHECKSUM_BITS	 5


#ifndef ASM

auto u_int16 ReadFuseMulti(register __a0 u_int16 loBit, /* no interrupt... */
			   register __a1 u_int16 bits); /*   ...protection */
auto u_int16 CalcFuseCRC(void);
u_int32 SwapBits0And2(register __reg_d u_int32 d);

#endif /* !ASM */

#endif /* !__VS1005_FUSE_H__ */
