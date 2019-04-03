#ifndef VSMPG_H
#define VSMPG_H

#include <vstypes.h>

#ifndef ASM
#include <stdio.h>
#include <stdlib.h>
#endif


#define USE_CRC /* Currently implemented for layer 3 only */
#define EXTRA_QUANT_BITS 9 /* for 32-bit quantization, must be at least 3 */

#define SYNTH_BUF_SIZE 0x110


#ifndef __VSDSP__

#define memsetY memset

#define __a
#define __b
#define __c
#define __d

#define __x
#define __y

#define __a0
#define __a1
#define __b0
#define __b1
#define __c0
#define __c1
#define __d0
#define __d1
#define __i0
#define __i1
#define __i2
#define __i3

#define auto

#else

#if 0
#define __y
#endif

#endif


#ifdef STANDALONE

#ifndef SHORT_BIGENDIAN_OUTPUT
#define SHORT_BIGENDIAN_OUTPUT
#endif

#ifndef ASM

#ifdef NO_STANDALONE_PRINTING
#define puts(s)
#define putchar(c)
#define perror(s)
void exit(register __a0 int);
#endif

#endif/*!ASM*/

#endif/*STANDALONE*/

#ifdef __VSMPG__
#define TEST_ENDIAN 1
#else
#define TEST_ENDIAN 0
#endif

#ifndef CONST
#define CONST const
#endif



#ifndef ASM
//typedef short s_int16;
//typedef unsigned short u_int16;
//typedef long s_int32;
//typedef unsigned long u_int32;
#ifdef __VSDSP__
//typedef long s_int40;	/* Danger: only works on reg variables */
//typedef __fract s_int16 f_int16;
//typedef __fract s_int32 f_int32;
typedef unsigned char byte;
#else
typedef long long s_int40;	/* Danger: only works on reg variables */
typedef unsigned char byte;
#endif

#endif

/* AUDIOBUFSIZE = 64, 128, 256, or 512 */
#define		AUDIOBUFSIZE		64

#define         FALSE                   0
#define         TRUE                    1

#define         SBLIMIT                 32
#define         SCALE_BLOCK             12
#define         SSLIMIT                 18

#define         MPG_MD_STEREO           0
#define         MPG_MD_JOINT_STEREO     1
#define         MPG_MD_DUAL_CHANNEL     2
#define         MPG_MD_MONO             3

/*
  EXTRA_QUANT_BITS effect in layer 3 compliance (-20dB sin sweep):
   7:  -99.2dB (7.7684e-06)
   8:  -99.9dB (7.1876e-06)
   9: -100.1dB (6.9984e-06)
  10: -100.2dB (6.9218e-06)
  11: -100.2dB (6.9010e-06)
  12: -100.2dB (6.8991e-06)
 */

#ifndef ASM

#ifdef ALLOC_IN_X
#define ALLOC_MEM
#else
#define ALLOC_MEM __y
#endif

struct frame {
    ALLOC_MEM const struct al_table * const *alloc;
    int stereo;
    int jsbound;
    int single;
    int II_sblimit;
    int lsf;
    int mpeg25;
    int header_change;
    int lay;
/*    void (*do_layer)(void);*/
    int error_protection;
    int bitrate_index;
    int sFreqIdx;
    int padding;
#if 0
    int private;
#endif
    int mode;
    int mode_ext;
#if 0
    int copyright;
    int original;
    int emphasis;
#endif
    int framesize; /* computed framesize */
#ifdef USE_CRC
    unsigned short file_crc;
    unsigned short actual_crc;
#endif
    int reservoir_len;
};
#ifdef USE_CRC
u_int16 CalcCrc(register __i0 __y u_int16 *data, s_int16 len, u_int16 initial);
#endif/*USE_CRC*/


#include "codMDec.h"

u_int16 get1bity(register __i3 struct BITPTRY *bit);
auto void rewindbitsy(register __i3 struct BITPTRY *bit, register __a1 int number_of_bits);
auto u_int16 getbitsy(register __i3 struct BITPTRY *bit, register __a1 int);

struct gr_info_s {
      s_int16 scfsi;
      u_int16 part2_3_length;
      u_int16 big_values;
      u_int16 scalefac_compress;
      u_int16 block_type;
      u_int16 mixed_block_flag;
      u_int16 table_select[3];
    //u_int16 subblock_gain[3];
      u_int16 maxband[3];
      u_int16 maxbandl;
      u_int16 region1start;
      u_int16 region2start;
      u_int16 preflag;
      u_int16 scalefac_scale;
      u_int16 count1table_select;
      s_int16 full_gain[3];
      s_int16 pow2gain;
};

struct III_sideinfo {
  u_int16 main_data_begin;
  struct {
    struct gr_info_s gr[2];
  } ch[2];
};

//extern struct frame __y fr;

struct CodecMpg;
s_int16 MpgLayer3(struct CodecMpg *cm);
s_int16 MpgLayer2(struct CodecMpg *cm);
auto void InitLayer3(struct CodecMpg *cm);
auto void SetLayer3Acc(u_int16 acc); /* 0 = worst, 3 = best */

void InitDecode(void);
auto void SynthesisMono32(s_int32 *bandPtr,s_int16 samples[2*SBLIMIT]);
auto void Synthesis32(s_int32 *bandPtr, s_int16 ch, s_int16 samples[2*SBLIMIT]);
auto void SynthesisMono32_2(s_int16 *ph, s_int32 *synthBuffs,
			    s_int32 *bandPtr,s_int16 samples[2*SBLIMIT]);
auto void Synthesis32_2(s_int16 *ph, s_int32 *synthBuffs,
			s_int32 *bandPtr, s_int16 ch, s_int16 samples[2*SBLIMIT]);



auto s_int16 wav_open(const char *wavfilename);
struct CodecServices;
auto s_int16 wav_write(struct CodecServices *cs);
#ifdef STANDALONE
auto s_int16 WmaStereoCopy(s_int16 *p, s_int16 samples);
#else
//#define ALWAYS_TWO_CHANNELS
auto s_int16 wav_close(void);
#endif

extern unsigned char *conv16to8;
extern const u_int16 codMpgFreqs[9];
extern const char * const freqNames[9];
extern CONST u_int16 __y muls[64];  /* Values fit    0 .. 2.0     -> 0.1.15 */
extern CONST s_int16 mulmul[27];    /* Values fit -1.0 .. 1.0     -> 1.0.15 */
//extern CONST s_int16 __y decwin[512+32];/* Values fit -32768..32767->1.15.0 */
//extern CONST s_int16 __y decwin16[256+16]; /* Aligned to 16-word boundaries */
extern CONST s_int16 __y codMpgDecwin16scaled[256+16];

#if 0
void FltInit(void);
void FltCollect(int context, double value);
void FltPrint(void);
#endif

/* LAYER2 */
#ifdef GRP_TAB_IN_X
/* Only allow packed version now.. */
extern CONST u_int16 grp_3tab[32];     /* used: 27 */
extern CONST u_int16 grp_5tab[128];     /* used: 125 */
extern CONST u_int16 grp_9tab[729];     /* used: 729 */
#else
extern CONST u_int16 __y grp_3tab[32]; /* used: 27 */
extern CONST u_int16 __y grp_5tab[128]; /* used: 125 */
extern CONST u_int16 __y grp_9tab[729]; /* used: 729 */
#endif

/* LAYER3 */
#define GAINPOW_SZ 64
extern CONST u_int16 __y gainpow2[GAINPOW_SZ];
extern CONST __y s_int16 codMpgAaCsca[2*8];
extern CONST s_int16 codMpgWin[4][36];
extern CONST s_int16 codMpgWin1[4][36];
extern CONST s_int16 codMpgTan1_1[2*16],codMpgTan2_1[2*16],codMpgTan1_2[2*16],codMpgTan2_2[2*16];
extern CONST s_int16 pow1_1[2][2*16],pow2_1[2][2*16],pow1_2[2][2*16],pow2_2[2][2*16];

#ifdef LIMITS_IN_Y
extern CONST __y u_int16 longLimit[9][23];
extern CONST __y u_int16 shortLimit[9][14];
#else
extern CONST u_int16 longLimit[9][23];
extern CONST u_int16 shortLimit[9][14];
#endif
extern CONST s_int16 mapbuf0[9][152];
extern CONST s_int16 mapbuf1[9][156];
extern CONST s_int16 mapbuf2[9][44];
extern CONST u_int16 codMpgNSlen2[512];
#ifdef ISLEN_IN_Y
extern CONST __y u_int16 codMpgISlen2[256];
#else
extern CONST u_int16 codMpgISlen2[256];
#endif

/* These may be used by layer 1 and 2 for other purposes.
   Size for each table is 1152. */
//extern s_int32 hybridIn32[2][SBLIMIT][SSLIMIT];

struct audio_info_struct {
  u_int16 rate;
  u_int16 channels;
};

auto void AudioSet(u_int16 ch, u_int16 r);
auto void AudioInit(void);

extern struct audio_info_struct __y ai;
auto void InitCommon(struct CodecMpg *cm);


/* For pow43 */
extern u_int16 const     pow43_1_mant[];
extern u_int16 const __y pow43_1_exp[];
extern u_int16 const     pow43_2_mant[];
extern u_int16 const __y pow43_2_exp[];
extern u_int16 const     pow43_4_mant[];
extern u_int16 const __y pow43_4_exp[];

/* For cos64, imdct36 & imdct12 */
extern s_int16 __y const cos64NewTab[];
extern s_int16 __y const cos36Tab[];
extern s_int16 __y const cos12Tab[];

/* For bit routines */
extern u_int16 __y * __y wordpointer;
extern u_int16 __y bitindex;
extern s_int16 __y bitsremaining;

/* Decode.c's buffer that must be converted with data */
extern s_int32 synthesisBuffs32[2][2][SYNTH_BUF_SIZE]; /* 2*1088w, aligned-32! */

#define FRAMESPACE (1792+512)

struct L123Y {
    u_int16 bsspace[FRAMESPACE/2]; /* 1152 words -- holds one frame */
    struct frame fr; /* 19 words */
    union {
	struct {
	    u_int16 bit_alloc[2*SBLIMIT];
	    u_int16 scfsi_buf[2*SBLIMIT];
	    /* 128 */
	    u_int16 scale[192];
	    //1491
	} l2;
	struct {
	    struct III_sideinfo sideinfo; /*105*/
	    /* 183 */
	    u_int16 scalefacs[2][39]; /* 78*/
	    //1354
	} l3;
    } i;
};
//extern __y struct L123Y layer123y; //1491

/* Just until it is fully codecized */
#include "codMDec.h"
//extern struct CodecMpg codecMpg;
//extern struct CodecMpg *cm;
//extern struct CodecServices *codecServices;


#endif /*!ASM*/

#endif
