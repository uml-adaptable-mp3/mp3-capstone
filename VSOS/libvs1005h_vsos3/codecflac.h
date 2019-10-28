/**
   \file codecflac.h FLAC Codec.
*/

#ifndef CODEC_MPG_H
#define CODEC_MPG_H

#include <vstypes.h>
#include <codec.h>
#include <filter.h>

//#define USE_COMMENTS

#ifndef ASM

#ifdef __VSDSP__
extern u_int16 preloadResult;
u_int16 preloadFlac(void);
void UnloadFlac(void);
#endif/*__VSDSP__*/

/**
   Create and allocate space for codec. Currently uses static allocation.

   \return A Codec structure pointer.
*/
struct Codec *CodFlacCreateStatic(void);

/**
   Create and allocate space for codec. Uses..

   \return A Codec structure pointer.
*/
struct Codec *CodFlacCreate(void);

/**
   Free all resources allocated for codec.

   \param cod A Codec structure.
*/
void CodFlacDeleteStatic(struct Codec *cod);
/**
   Free all resources allocated for codec.

   \param cod A Codec structure.
*/
void CodFlacDelete(struct Codec *cod);


enum codFlacTag {
#ifndef TAG_END
#define TAG_END 0
#endif
    codFlacTagEnd = 0,
    codFlacDisableCrc   = 1, /*Note: by default equals bits in decodeFlags*/
    codFlacEnableCrc    = 2, /*  */
    codFlacLeaveOnError = 4, /*  */
    codFlacLeaveOnInvalid = 8, /* also bitstream syntax */
};
/**
   Set internal flags.
   \param cod - A codec structure
   \param tags - first tag
 */
void CodFlacSetTags(struct Codec *cod, ...);

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
enum CodecError CodFlacDecode(struct Codec *cod, struct CodecServices *cs,
			     const char **errorString);
/*
  Additional functions. These can be used to initialize decoding,
  read frames, decode frames etc.
 */

#endif /* !ASM */

#endif /* CODEC_VORBIS_H */
