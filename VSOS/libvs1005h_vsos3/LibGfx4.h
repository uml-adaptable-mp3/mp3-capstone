#ifndef LIBGFX4_H
#define LIBGFX4_H

typedef s_int16 (*FontStyle)(regz u_int16*,regz u_int16,regz wchar,regz u_int16);
void Rasterizer4(regz u_int16*,regz s_int16,regz BtnPos*); // standard rasterizer
void RenderText4(regz wchar* string,regz s_int16 x,regz s_int16 y,regz u_int16 pitch,regz FontStyle font);
void RenderIcon4(regz u_int16* data,regz s_int16 x,regz s_int16 y,regz u_int16 pitch,regz u_int16 size);
void ColorShift4(regz s_int16 amount,regz s_int16 x,regz s_int16 y,regz u_int16 w,regz u_int16 h,regz u_int16 pitch);
void Shadow4(regz s_int16 x,regz s_int16 y,regz u_int16 w,regz u_int16 h,regz u_int16 pitch);
s_int16 TextWidth4(regz wchar* string,regz FontStyle font);
// font styles are not intended to be called directly; pass as a parameter to RenderText4 instead
s_int16 StdBigFont(regz u_int16*,regz u_int16,regz wchar,regz u_int16);
s_int16 StdSmallFont(regz u_int16*,regz u_int16,regz wchar,regz u_int16);
s_int16 StdUniFont(regz u_int16*,regz u_int16,regz wchar,regz u_int16);

extern u_int16 bitCache4[]; // the main graphics buffer
extern u_int16 palette4[]; // color map (15 colors)

typedef void (*WallFunc)(regz u_int16*,regz s_int16,regz s_int16,regz s_int16);
extern WallFunc wallFunc;

#endif