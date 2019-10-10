/** \file lcd7735.h Definitions for ST7735 TFT LCD Controller */

#ifndef LCD7735_H
#define LCD7725_H

#define LCD_NOP	0x00
#define LCD_SWRESET	0x01
#define LCD_RDDID	0x04
#define LCD_RDDST	0x09
#define LCD_RDDPM	0x0A
#define LCD_RDDMADCTR 0x0B
#define LCD_RDDCOLMOD 0x0C
#define LCD_RDDIM	0x0D
#define LCD_RDDSM	0x0E
#define LCD_SLPIN	0x10
#define LCD_SLPOUT	0x11
#define LCD_PTLON	0x12
#define LCD_NORON	0x13
#define LCD_INVOFF	0x20
#define LCD_INVON	0x21
#define LCD_GASET	0x26
#define LCD_DISPOFF	0x28
#define LCD_DISPON	0x29
#define LCD_CASET	0x2A
#define LCD_RASET	0x2B
#define LCD_RAMWR	0x2C
#define LCD_RAMRD	0x2E
#define LCD_PTLAR	0x30
#define LCD_TEOFF	0x34
#define LCD_TEON	0x35
#define LCD_MADCTL	0x36
#define LCD_IDMOFF	0x38
#define LCD_IDMON	0x39
#define LCD_COLMOD	0x3A
#define LCD_RDID1	0xDA
#define LCD_RDID2	0xDB
#define LCD_RDID3	0xDC

#define LCD_PIXEL_FORMAT_12BPP 3
#define LCD_PIXEL_FORMAT_16BPP 5
#define LCD_PIXEL_FORMAT_18BPP 6

auto u_int16 SpiSendLcd16(register __a0 u_int16 dataTopAligned, register __a1 dummy);
#endif

