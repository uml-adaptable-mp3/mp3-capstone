/**
   \file codecaiff.h Aiff Codec. At this moment the Aiff Codec supports
	8-bit, 16-bit, 24-bit and 32-bit stereo and mono files.
	Random access for seekable supported for all audio formats.
*/

#ifndef CODEC_AIFF_H
#define CODEC_AIFF_H

#include <vstypes.h>
#include <codec.h>
#include <filter.h>


/**
   Create and allocate space for codec.

   \return A Aiff Codec structure.
*/
struct Codec *CodAiffCreate(void);

/**
   Free all resources allocated for codec.

   \param cod A Aiff Codec structure.
*/
void CodAiffDelete(struct Codec *cod);

/**
   Decode file. Upon success or a negative number, Codec has succeeded.
   With a positive number, there has been an error. Upon return, an
   error string is also returned.

   \param cod A Aiff Codec structure.
   \param cs User-supplied codec services with appropriate fields
	filled. The fields that are not to be filled by the user
	should be zero initialized.
   \param errorString A pointer to a char pointer. The codec may
	return its error status here.

   \return Error code.
 */
enum CodecError CodAiffDecode(struct Codec *cod, struct CodecServices *cs,
			     const char **errorString);


#endif
