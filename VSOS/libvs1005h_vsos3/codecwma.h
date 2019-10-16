/**
   \file codecwma.h WMA4.0-WMA9
*/

#ifndef CODEC_WMA_H
#define CODEC_WMA_H

#include <vstypes.h>
#include <codec.h>
#include <filter.h>

//#define USE_COMMENTS

#ifndef ASM
/**
   Create and allocate space for codec. Currently uses static allocation.

   \return A Codec structure pointer.
*/
struct Codec *CodWmaCreateStatic(void);

/**
   Create and allocate space for codec. Uses..

   \return A Codec structure pointer.
*/
struct Codec *CodWmaCreate(void);

/**
   Free all resources allocated for codec.

   \param cod A Codec structure.
*/
void CodWmaDeleteStatic(struct Codec *cod);
/**
   Free all resources allocated for codec.

   \param cod A Codec structure.
*/
void CodWmaDelete(struct Codec *cod);

struct CodecWma;

#include "wma.h"

struct CodecWma {
    struct Codec c;
    wmaStream_t *wma; //g_wma
    __mem_y wmaChannel_t *chn;//g_chn[2];
    s_int32 *lpccoef; //g_lpccoef[2][D_LPCORDER]; /*scaled by 2^LPCOEF_SCALE*/
    __mem_y union BWEIGHT *weight; //wmaWg
    s_int16 scales[4];
#ifdef __VSDSP__
    s_int16 *dctlo; //needs to be aligned to 2048 words and have both X and Y
#else
    s_int32 *sample; //2048
#endif
    s_int16 *others; //2048
    s_int16 *yprev0; //1024
    s_int16 *yprev1; //1024
};
extern struct CodecWma codecWma; //for now

enum codWmaTag {
#ifndef TAG_END
#define TAG_END 0
#endif
    codWmaTagEnd = 0,
    codWmaDisableCrc   = 1, /*Note: by default equals bits in decodeFlags*/
    codWmaEnableLayer2 = 2, /* only if compiled with layer2 */
    codWmaLeaveOnError = 4, /*  */
    codWmaLeaveOnInvalidLayer = 8, /* also bitstream syntax */
};
/**
   Set internal flags.
   \param cod - A codec structure
   \param tags - first tag
 */
void CodWmaSetTags(struct Codec *cod, ...);

/**
   Decode file. Upon success or a negative number, Codec has succeeded.
   With a positive number, there has been an error. Upon return, an
   error string is also returned.

   \param cod - A Codec structure.
   \param cs - User-supplied codec services with appropriate fields
	filled. The fields that are not to be filled by the user
	should be zero initialized.
   \param errorString - A pointer to a char pointer. The codec may
	return its error status here.

   \return Error code.
 */
enum CodecError CodWmaDecode(struct Codec *cod, struct CodecServices *cs,
			     const char **errorString);
/*
  Additional functions. These can be used to initialize decoding,
  read frames, decode frames etc.
 */

#endif /* !ASM */

#endif /* CODEC_WMA_H */
