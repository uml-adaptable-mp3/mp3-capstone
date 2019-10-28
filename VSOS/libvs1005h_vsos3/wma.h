/**
 * @file        wma.h
 * @guid        cc1aefa1047c4d68bf94b588c1ba7e37
 * @copyright   2003 Ionific Ltd.
 * @author      Jussi Lahdenniemi
 * @project     vlsi/wma
 * @description WMA Decoder global header
 */

#ifndef Hcc1aefa1047c4d68bf94b588c1ba7e37
#define Hcc1aefa1047c4d68bf94b588c1ba7e37

//#include "codecwma.h"

/* common definitions */
#define STREAM_BUFFER_SZ 1024
#define HALF_TABLE
#define LPCOEF_SCALE 25


#ifdef ASM

/* asm-only definitions */
#define STREAM_LOC_X  0x0000
#define YPREV0_LOC_X  0x0400
#define OTHERS_LOC_X  0x0800
#define YPREV1_LOC_X  0x1c00
#define SAMPLE_LOC_XY 0x1000
#define STACK_LOC_XY  0x1800
#define STACKSIZE 320//192
#define AUDIOB_LOC_Y  0x0000

#else/*ASM*/

/* C-only definitions */

#include <stdio.h>
#include <assert.h>
#include <math.h>

#include <vstypes.h>

/* Integers */

#if 0 //ndef VSMPG_H
typedef short           s_int16;
typedef unsigned short  u_int16;
typedef long            s_int32;
typedef unsigned long   u_int32;

#ifdef __VSDSP__
typedef __fract s_int16 f_int16;
typedef __fract s_int32 f_int32;
#endif
#endif

#ifndef M_PI
#   define M_PI 3.141592653589793238462643
#endif

/* Special definitions */

#ifndef __VSDSP__
#define auto
#define __d1
#define __d0
#define __c1
#define __c0
#define __b1
#define __b0
#define __a1
#define __a0
#define __i0
#define __i1
#define __i2
#define __i3
#define __d
#define __c
#define __b
#define __a
#define __d1
#define __far
#define __x
#define __y

/* Floats */

typedef float  samp_t;

#else /* VSDSP */

/* Floats */

typedef double  samp_t;

#endif /* !VSDSP */

/* Assertions */

#define VASSERT assert


/* WMA Constants */

#define D_ENCOPT_NO_PKTLOSS       0x4000
#define D_ENCOPT_BARK             0x0001
#define D_ENCOPT_LPC              0x0020
#define D_ENCOPT_PACKETS          0x0002
#define D_ENCOPT_SUBFRAME         0x0004
#define D_ENCOPT_SUBFRAMEDIVMASK  0x0018
#define D_LPCORDER                10
#define D_MASK_RESAMPLE_OFFSET    6
#define D_MAX_QUANT_INCR          127
#define D_MIN_FRAME_SIZE          128
#define D_STEREO_WEIGHT           1.6

/* Global information structure */

#define WMA_INVALID    1
#define WMA_PACKETLOSS 2
typedef struct
{
    /* Input bitstream */

    /* Do NOT change the order or types of these */
    u_int16 invalid; /* 1 = invalid, 2 = packetLoss */
    u_int16 prevPacket;
    u_int16 newHeader;
    u_int32 dummy_bitend;
    u_int32 dummy_bitbreak;
    u_int16 hufsymlen; //offset 7
    /*********************************************/

    s_int16 Np;
    s_int16 MpnOff;
    s_int16 Mpn;
    s_int16 count; /* samples output */
    u_int16 channels; /* 12 */

    u_int16 prevSubframe;
    u_int16 currSubframe;
    u_int16 nextSubframe;

    u_int32 packetLength;
    u_int32 curPacketLength;
    u_int32 curPaddingLength;
    u_int16 stream;
    s_int16 pktframes;
    u_int16 noise;
    u_int16 noisefreq;
    u_int16 const *huffMono;
    u_int16 const *huffDiff;
    u_int16 lpc;
    s_int16 superframes;

    /* Audio stream information */

    u_int16 frequency;
    u_int32 bytespersec;
    u_int16 blockalign;
    u_int16 numOfPayloads;   /* >0 for multiple payloads in a packet */
    u_int16 compressedSize;  /* >0 for compressed payloads */
#define FP_BROADCAST_FLAG 1
    u_int16 fileProperties;  /* LSb = broadcast flag */

    u_int16 encodeOptions;
    u_int16 framesize;
    u_int16 log2frame;
    u_int16 maxSubframes;
    u_int16 log2frameBits;
    u_int16 log2ratio;
    u_int16 initDiscard;
    u_int16 dropExtra;
    u_int16 prevRealSubframe;

    /* (Sub)frame info */

    u_int16 subframe;
    u_int16 barkBands;
/*  u_int16 barkInit;*/
    __y u_int16 const *barkIndex;
    u_int16 lowCutoff;
    u_int16 highCutoff;
    u_int16 subframeStart;
    u_int16 stmode;
    u_int16 qstep;
    u_int16 log2MaxEsc;
    u_int16 firstNoise;
    u_int16 maskupdate;
    u_int16 subfrnumber;

    u_int32 lastBytePos;

    u_int16 oldFlags[2];
    u_int16 lostCnt;
    u_int32 sendTime;

} wmaStream_t;

typedef struct {
    u_int16 coded;
    u_int32 bandNotCoded;  /* bit mask of channels not coded */
    u_int16 bandsNotCoded;
    u_int16 codedCoeff;
    u_int16 reqStart;
    s_int16 mask[26];
    s_int16 noisePower[26];
    samp_t  maxlpc;
    s_int32 *lpccoef;
    u_int16 lpcreset;
    u_int16 lpcsize;
    u_int16 lpcminsize;

    /* Buffers */

    u_int16 sflength;
    u_int16 barkIndexUpdate[26];
    u_int16 barkIndicesUpdate;
} wmaChannel_t;

/* Temporaries for dequantize */
union BWEIGHT {
    struct {
	s_int32 bweight[26+1];
	u_int16 bitbl[26+1];
    } bark;
    struct {
	samp_t bweight[26+1];
    } lpc;
    samp_t tmp[27];
};
extern __y union BWEIGHT wmaWg;

extern s_int32 g_lpccoef[2][D_LPCORDER]; /* scaled by 2^LPCOEF_SCALE */
extern wmaStream_t g_wma;
extern __y wmaChannel_t g_chn[2];


struct BITPTRYBRK {
    s_int16 bitindex;
    u_int16 *wordpointer;
    u_int32 bitcount;
    u_int32 bitbreak;
    u_int32 bitend;
    s_int16 invalid; //0=ok, 1=invalid, 2=packet loss
};
extern __mem_y struct BITPTRYBRK bitptry;


#ifdef __VSDSP__
extern u_int16 g_dctlo[2048];
extern __y u_int16 g_dcthi[2048];
#define RDSAMP(s) ((s_int32)*(u_int16 *)(s)|((s_int32)*(__y u_int16 *)(s)<<16))
#define WRSAMP(s,w) do{register __d s_int32 wr=(w);*(u_int16 *)(s)=wr; *(__y u_int16 *)(s)=(wr>>16);}while(0)
#else
#define RDSAMP(s) (*(s))
#define WRSAMP(s,w) (*(s)=(w))
extern s_int32 g_sample[2048];
#endif
extern s_int16 g_others[2048];
extern s_int16 g_yprev0[];
extern s_int16 g_yprev1[];
#define G_SCALE_YPREV0 0
#define G_SCALE_YPREV1 1
#define G_SCALE_OTHERS 2
#define G_SCALE_OTHERS_SUP 3

//extern s_int16 g_scales[4];

/* Exit reason */

#ifdef __VSDSP__
extern __mem_x const unsigned short * __mem_y g_message;
#define PACKED_STR "\p"
#else
extern const char *g_message;
#define PACKED_STR ""
#endif


void memclearXY(register __i0 u_int16 *p, register __a0 s_int16 c);

/* wmabits.c */

#ifndef STANDALONE
void wmaStreamReset(FILE *file);
#else
void wmaStreamReset(void);
#endif
auto void WaitForWmaData(register __a0 s_int16 n);
auto s_int16 StreamDiff(void);
auto s_int16 WMyGetC(void);
void StreamDiscard(void);
void StreamSeek(u_int32 bitpos);
int wmaInitBitstream( struct CodecWma *cw );
void frameDone( void );
u_int16 wma_getbits( struct CodecWma *cw, register __a1 int number_of_bits );
u_int16 wma_get1bit( struct CodecWma *cw, register __b s_int32 );
void syncbits( struct CodecWma *cw );
void initpacket(struct CodecWma *cw, s_int16 discard);
u_int16 checkPacketLossAtEnd( struct CodecWma *cw );
//void CheckSkipping(void);
auto void wmaStreamDiscardDataUpto(struct CodecWma *cw, register __a u_int32 target);

/* wma.c */

int wmaInitDecoder( struct CodecWma *cw );
int wmaDecodeFrame( struct CodecWma *cw, int skipFlags );

/* output.c */

auto s_int16 WmaStereoCopy(s_int16 *buf, s_int16 cnt);
void outputSamples( struct CodecWma *cw, s_int16 count );

/* dct.c */

void initdct();
#ifdef __VSDSP__
void dct( s_int16 *in, u_int16 length );
#else
void dct( s_int32 in[], u_int16 length );
#endif

#ifndef __VSDSP__
extern __y s_int16 i_twiddle_iv[];
#else
extern __y const s_int16 i_twiddle_iv[];
#endif

/* lpc.c */

void initlpc( void );
void lpcWeightInit( __y wmaChannel_t *chn );
auto s_int32 lpcWeight( void );
extern __y u_int16 lpc_n;

#ifndef __VSDSP__
extern s_int32 lsfCosTable[D_LPCORDER][16];
#else
double CutTo24(double a);
extern const s_int32 lsfCosTable[D_LPCORDER][16];
#endif

/* imlt.c */

void imlt( struct CodecWma *cw, s_int16 ch );

/* dequantize.c */

#ifdef __VSDSP__
extern s_int32 qrand_prior;
extern u_int32 qrand_seed;
#endif

void dequantize(struct CodecWma *cw, __y wmaChannel_t *chn );
void dequantizeLpc(struct CodecWma *cw, __y wmaChannel_t *chn );

/* huffman.c */

auto u_int16 getvlc(struct CodecWma *cw, register u_int16 const* tab );
int decodeRunlevel( struct CodecWma *cw, __y wmaChannel_t *chn, u_int16 ch );

/* hufftbl.c */

#define HUFFMAN_ESCAPE  0x3fff
#define HUFFMAN_EOB     0x3ffe

extern u_int16 const HuffmanRLC44OMono[];
extern u_int16 const HuffmanRLC44ODiff[];
extern u_int16 const HuffmanRLC44QMono[];
extern u_int16 const HuffmanRLC44QDiff[];
extern u_int16 const HuffmanRLC16Mono[];
extern u_int16 const HuffmanRLC16Diff[];
extern u_int16 const HuffmanMask[];
extern u_int16 const HuffmanNoisePower[];


/*
  Rewind and fast forward support
 */
void WMANextPacketAt(u_int32 filePos);
void WMACheckSeek(void);
int WMACheckResync(struct CodecWma *cw);
extern __y u_int32 bitcount;
int WMAResyncInternal(struct CodecWma *cw, s_int16 *resync);
#ifdef STANDALONE
void Disable(void);
void Enable(void);
#endif

void CheckBroadCast(struct CodecWma *cw );
auto void DecodeTime(register __a0 u_int16 samples,
		     register __a1 u_int16 limit,
		     register __b s_int32 bytes);
extern void OutOfStreamData(void);
extern void UpdateDREQ(void);

#endif /*ASM*/

#endif /* Hcc1aefa1047c4d68bf94b588c1ba7e37 */
