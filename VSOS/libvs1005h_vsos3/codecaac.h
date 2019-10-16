/**
   \file codecflac.h FLAC Codec.
*/

#ifndef CODEC_MPG_H
#define CODEC_MPG_H

#include <vstypes.h>
#include <codec.h>
#include <filter.h>

//#define USE_COMMENTS

#ifdef __VSDSP__
extern u_int16 preloadResult;
u_int16 preloadAac(void);
void UnloadAac(void);
#endif/*__VSDSP__*/

#ifndef ASM
/**
   Create and allocate space for codec. Currently uses static allocation.

   \return A Codec structure pointer.
*/
struct Codec *CodAacCreateStatic(void);

/**
   Create and allocate space for codec. Uses..

   \return A Codec structure pointer.
*/
struct Codec *CodAacCreate(void);

/**
   Free all resources allocated for codec.

   \param cod A Codec structure.
*/
void CodAacDeleteStatic(struct Codec *cod);
/**
   Free all resources allocated for codec.

   \param cod A Codec structure.
*/
void CodAacDelete(struct Codec *cod);


enum codAacTag {
#ifndef TAG_END
#define TAG_END 0
#endif
    codAacTagEnd = 0,
    codAacDisableCrc   = 1, /*Note: by default equals bits in decodeFlags*/
    codAacEnableCrc    = 2, /*  */
    codAacLeaveOnError = 4, /*  */
    codAacLeaveOnInvalid = 8, /* also bitstream syntax */
};
/**
   Set internal flags.
   \param cod - A codec structure
   \param tags - first tag
 */
void CodAacSetTags(struct Codec *cod, ...);

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
enum CodecError CodAacDecode(struct Codec *cod, struct CodecServices *cs,
			     const char **errorString);
/*
  Additional functions. These can be used to initialize decoding,
  read frames, decode frames etc.
 */

#endif /* !ASM */

#endif /* CODEC_VORBIS_H */
