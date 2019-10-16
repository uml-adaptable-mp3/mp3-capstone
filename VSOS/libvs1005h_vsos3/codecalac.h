/**
   \file codecalac.h ALAC Codec.
*/

#ifndef CODEC_ALAC_H
#define CODEC_ALAC_H

#include <vstypes.h>
#include <codec.h>
#include <filter.h>

//#define USE_COMMENTS

#ifndef ASM

#ifdef __VSDSP__
extern u_int16 preloadResult;
u_int16 preloadAlac(void);
void UnloadAlac(void);
#endif/*__VSDSP__*/

/**
   Create and allocate space for codec. Currently uses static allocation.

   \return A Codec structure pointer.
*/
struct Codec *CodAlacCreate(void);

/**
   Free all resources allocated for codec.

   \param cod A Codec structure.
*/
void CodAlacDelete(struct Codec *cod);


enum codAlacTag {
#ifndef TAG_END
#define TAG_END 0
#endif
    codAlacTagEnd = 0,
    codAlacDisableCrc   = 1, /*Note: by default equals bits in decodeFlags*/
    codAlacEnableCrc    = 2, /*  */
    codAlacLeaveOnError = 4, /*  */
    codAlacLeaveOnInvalid = 8, /* also bitstream syntax */
};
/**
   Set internal flags.
   \param cod - A codec structure
   \param tags - first tag
 */
void CodAlacSetTags(struct Codec *cod, ...);

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
enum CodecError CodAlacDecode(struct Codec *cod, struct CodecServices *cs,
			     const char **errorString);
/*
  Additional functions. These can be used to initialize decoding,
  read frames, decode frames etc.
 */

#endif /* !ASM */

#endif /* CODEC_ALAC_H */
