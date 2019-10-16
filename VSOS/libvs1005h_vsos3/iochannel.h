// This header file is used with the 74HC138 chip select generator on the developer board.

#ifndef IOCHANNEL_H
#define IOCHANNEL_H
#include "vs1005h.h"
/*	
GPIO1-15 GPIO0-11 GPIO1-0
	GP15	CE		xCS0	
0	0		0		0	- power control latch
1	0		0		1	- ext1
2	0		1		0	- ext2
3	0*		1		1	- booting / IO from spi flash
4	1		0		0	- ext4
5	1		0*		1	- booting / IO from nand flash
6	1		1		0	- TFT display	
7	1		1		1	- rst, pullup 10K, idle, do not use
*/

#define INITIALIZE_IO_CHANNEL_IDLE() {GpioSetPin(0x1f,1); GpioSetPin(0x0b, 1); GpioSetPin(0x10, 1);}
#define SELECT_IO_CHANNEL_IDLE(){PERIP(GPIO1_SET_MASK)=(1 << 15) | (1 << 0); PERIP(GPIO0_SET_MASK)=(1 << 11);}
//PERIP(GPIO1_SET_MASK)

//#define SELECT_IO_CHANNEL(a) {GpioSetPin(0x1f, a&4); GpioSetPin(0x0b, a&2); GpioSetPin(0x10, a&1);}
#define SELECT_IO_CHANNEL(a) {SELECT_IO_CHANNEL_IDLE(); if (!(a&1)) { PERIP(GPIO1_CLEAR_MASK) = (1 << 15); } if (!(a&2)) { PERIP(GPIO0_CLEAR_MASK) = (1 << 11); } if (!(a&4)) { PERIP(GPIO1_CLEAR_MASK) = (1 << 0); }}

#endif

