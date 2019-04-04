/**
   \file codecwav.h Wav Codec. At this moment the Wav Codec supports
	8-bit and 16-bit stereo and mono files. In addition 4-bit IMA ADPCM
	files are supported in both stereo and mono.
	Randon access for seekable supported for all audio formats.

   \note There is a "sox" bug patch
	for stereo files: if the first left and right samples of
	IMA blocks are always
	identical in an otherwise stereo file, the right sample is
	discarded and interpolated.
*/

#ifndef CODEC_WAV_H
#define CODEC_WAV_H

#include <vstypes.h>
#include <codec.h>
#include <filter.h>


/**
   Create and allocate space for codec.

   \return A Wav Codec structure.
*/
struct Codec *CodWavCreate(void);

/**
   Free all resources allocated for codec.

   \param cod A Wav Codec structure.
*/
void CodWavDelete(struct Codec *cod);

/**
   Decode file. Upon success or a negative number, Codec has succeeded.
   With a positive number, there has been an error. Upon return, an
   error string is also returned.

   \param cod A Wav Codec structure.
   \param cs User-supplied codec services with appropriate fields
	filled. The fields that are not to be filled by the user
	should be zero initialized.
   \param errorString A pointer to a char pointer. The codec may
	return its error status here.

   \return Error code.
 */
enum CodecError CodWavDecode(struct Codec *cod, struct CodecServices *cs,
			     const char **errorString);


#endif
