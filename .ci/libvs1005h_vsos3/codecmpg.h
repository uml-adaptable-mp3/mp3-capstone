/**
   \file codecmpg.h MPEG1.0/2.0 Layer II and III Codec.
*/

#ifndef CODEC_MPG_H
#define CODEC_MPG_H

#include <vstypes.h>
#include <codec.h>
#include <filter.h>

//#define USE_COMMENTS

#ifndef ASM
/**
   Create and allocate space for codec. Currently uses static allocation.

   \return A Codec structure pointer.
*/
struct Codec *CodMpgCreateStatic(void);

/**
   Create and allocate space for codec. Uses..

   \return A Codec structure pointer.
*/
struct Codec *CodMpgCreate(void);

/**
   Free all resources allocated for codec.

   \param cod A Codec structure.
*/
void CodMpgDeleteStatic(struct Codec *cod);
/**
   Free all resources allocated for codec.

   \param cod A Codec structure.
*/
void CodMpgDelete(struct Codec *cod);


enum codMpgTag {
#ifndef TAG_END
#define TAG_END 0
#endif
    codMpgTagEnd = 0,
    codMpgDisableCrc   = 1, /*Note: by default equals bits in decodeFlags*/
    codMpgEnableLayer2 = 2, /* only if compiled with layer2 */
    codMpgLeaveOnError = 4, /*  */
    codMpgLeaveOnInvalidLayer = 8, /* also bitstream syntax */
};
/**
   Set internal flags.
   \param cod - A codec structure
   \param tags - first tag
 */
void CodMpgSetTags(struct Codec *cod, ...);

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
enum CodecError CodMpgDecode(struct Codec *cod, struct CodecServices *cs,
			     const char **errorString);
/*
  Additional functions. These can be used to initialize decoding,
  read frames, decode frames etc.
 */

#endif /* !ASM */

#endif /* CODEC_VORBIS_H */
