#ifndef DCT_H
#define DCT_H

#include "codMpg.h"

/* Input values seem to be -1.0 .. 1.0 most of the time -> 1.0.15 format,
   Output values seem to be -1.0 .. 1.0 most of the time -> 1.0.15 */
auto void dct64(s_int16 *o0,s_int16 *o1,const s_int16 *s);
auto void dct64_32(s_int32 *o0,s_int32 *o1,const s_int32 *s);

auto void dct36_32_linear(s_int32 *buf,__y s_int32 *o1,const s_int16 *wintab, s_int32 tmpbuf32[18]);
auto void dct12_32_linear(s_int32 *buf,__y s_int32 *rawout1,const s_int16 *wi, s_int32 tmpbuf32[36]); /* interleaved short blocks */


#endif
