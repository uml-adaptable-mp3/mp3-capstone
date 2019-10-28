#ifndef __CODEC_MPG_DECODE_H__
#define __CODEC_MPG_DECODE_H__

#include <codec.h>
#include <vstypes.h>

#define USE_MPG_BITPTR /*Not finished!*/

#ifdef USE_MPG_BITPTR
struct BITPTRX {
    s_int16 index;
    u_int16 *pointer;
};
struct BITPTRY {
    s_int16 index;
    __y u_int16 *pointer;
};
#endif

#define DECODE_FLAGS_CRC_DISABLED        (1<<0)
#define DECODE_FLAGS_LAYER2_ENABLED      (1<<1)
#define DECODE_FLAGS_LEAVEONERROR        (1<<2)
#define DECODE_FLAGS_LEAVEONINVALIDLAYER (1<<3) /*also on invalid bitstream*/
struct CodecMpg {
    struct Codec c;
    struct BITPTRY bit;
    __y struct L123Y *layer123y;
    u_int16 header[2];
    u_int16 oldHeader[2];
    s_int16 skipFrameCount;
    u_int32 decodeFlags;
    s_int16 (*NextFrame)(struct CodecMpg *cm);

    struct C1 {
	s_int16 fsize;
	s_int16 fsizeold;
	s_int16 ssize;
	s_int16 oldssize;
	__y u_int16 *bsbuf;
	u_int16 bsOffset, bsOldOffset;
    } c1;
    s_int32 *hybridIn32;    //hybridIn32[2][SBLIMIT][SSLIMIT];
    __y s_int32 *hybHist32; //hybHist32[2][32*SSLIMIT]; /*2304*/
    s_int32 *reorder_tmp; //s_int32 reorder_tmp[3*66]
    s_int16 synthPhase;
    s_int32 *synthesisBuffs32; //aligned 32 words..
    s_int32 *synthBuffs; //original allocation of synthesisBuffs32
};
extern struct CodecMpg codecMpg;

void CodMpgInitReadFrame(struct CodecMpg *cm, struct CodecServices *cs);
s_int16 CodMpgReadFrame(struct CodecMpg *cm, struct CodecServices *cs);
s_int16 CodMpgPlayFrame(struct CodecMpg *cm, struct CodecServices *cs);
auto void CodMpgSetPointer(struct CodecMpg *cm, s_int16 backstep);



#endif/*!__CODEC_MPG_DECODE_H__*/
