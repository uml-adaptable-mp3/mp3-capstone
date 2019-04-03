/// \file rgb565.h Standard web color definitions for RGB565 colorspace

#ifndef RGB565_H
#define RGB565_H




#if 1
// RGB565 order
#define __RGB565(a) ((u_int16)((((a)>>19)<<11)|((((a)>>10)&0x3f)<<5)|(((a)>>3)&0x1f)))
#define __RGB565RGB(r,g,b) ((u_int16)((((r)>>3)<<11)|((((g)>>2)&0x3f)<<5)|(((b)>>3)&0x1f)))

#define COLOR_AQUA 0x07ffu
#define COLOR_BLACK 0x0000u
#define COLOR_BLUE 0x001fu
#define COLOR_BROWN 0xA144u
#define COLOR_CHARTREUSE 0x87e0u
#define COLOR_FUCHSIA 0xF81Fu
#define COLOR_GRAY 0x8410u
#define COLOR_GREEN 0x0400u
#define COLOR_LIME 0x07e0u
#define COLOR_MAROON 0x8000u
#define COLOR_NAVY 0x0010u
#define COLOR_OLIVE 0x8400u
#define COLOR_ORANGE 0xFD00u
#define COLOR_PURPLE 0x8010u
#define COLOR_RED 0xf800u
#define COLOR_SILVER 0xC818u
#define COLOR_TEAL 0x0410u
#define COLOR_VIOLET 0xf41eu
#define COLOR_WHITE 0xffffu
#define COLOR_YELLOW 0xffe0u
#define COLOR_COMPRESSED_TEXTURE 0xC00C

#else
// BGR565 order
#define __RGB565(a) ((((a)>>19)<<11)|((((a)>>10)&0x3f)<<5)|(((a)>>3)&0x1f))
#define __RGB565RGB(r,g,b) ((((b)>>3)<<11)|((((g)>>2)&0x3f)<<5)|(((r)>>3)&0x1f))

#define COLOR_WHITE 0xffffu
#define COLOR_BLACK 0x0000u
#define COLOR_NAVY 0x8000u
#define COLOR_RED 0x001fu
#define COLOR_AQUA 0xffe0u
#define COLOR_ORANGE 0x051fu
#define COLOR_GRAY __RGB565RGB(128,128,128)

#endif


#endif
