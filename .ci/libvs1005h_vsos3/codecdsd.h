/**
   \file codecdsd.h DSD (DSF) Codec.
*/

#ifndef CODEC_DSD_H
#define CODEC_DSD_H

#include <vstypes.h>
#include <codec.h>
#include <filter.h>

//#define USE_COMMENTS

#ifndef ASM

#ifdef __VSDSP__
extern u_int16 preloadResult;
u_int16 preloadDsd(void);
void UnloadDsd(void);
#endif/*__VSDSP__*/

/**
   Create and allocate space for codec. Currently uses static allocation.

   \return A Codec structure pointer.
*/
struct Codec *CodDsdCreateStatic(void);

/**
   Create and allocate space for codec. Uses..

   \return A Codec structure pointer.
*/
struct Codec *CodDsdCreate(void);

/**
   Free all resources allocated for codec.

   \param cod A Codec structure.
*/
void CodDsdDeleteStatic(struct Codec *cod);
/**
   Free all resources allocated for codec.

   \param cod A Codec structure.
*/
void CodDsdDelete(struct Codec *cod);


enum codDsdTag {
#ifndef TAG_END
#define TAG_END 0
#endif
    codDsdTagEnd = 0,
    //    codFlacDisableCrc   = 1, /*Note: by default equals bits in decodeFlags*/
    //codFlacEnableCrc    = 2, /*  */
    //codFlacLeaveOnError = 4, /*  */
    //codFlacLeaveOnInvalid = 8, /* also bitstream syntax */
};
/**
   Set internal flags.
   \param cod - A codec structure
   \param tags - first tag
 */
void CodDsdSetTags(struct Codec *cod, ...);

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
enum CodecError CodDsdDecode(struct Codec *cod, struct CodecServices *cs,
			     const char **errorString);
/*
  Additional functions. These can be used to initialize decoding,
  read frames, decode frames etc.
 */

#endif /* !ASM */

#endif /* CODEC_DSD_H */
