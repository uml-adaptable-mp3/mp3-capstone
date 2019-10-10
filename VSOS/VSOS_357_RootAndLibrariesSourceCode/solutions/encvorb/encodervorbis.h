/**
   \file encodervorbis.h Vorbis 1.0 Encoder.
*/

#ifndef ENCODER_VORBIS_H
#define ENCODER_VORBIS_H

#include <vstypes.h>
#include <encoder.h>
#include <filter.h>

#ifndef ASM
/* Create functions in encVorbisCreate.h and encVorbisCreatePreeet.h */

/**
   Free all resources allocated for encoder.

   \param enc A Vorbis Encoder structure.
*/
void EncVorbisDelete(struct Encoder *enc);
void EncVorbisDelete1053(struct Encoder *enc);

/**
   Encode file. Upon success or a negative number, Encoder has succeeded.
   With a positive number, there has been an error. Upon return, an
   error string is also returned.

   \param enc A Vorbis Encoder structure.
   \param es User-supplied encoder services with appropriate fields
	filled. The fields that are not to be filled by the user
	should be zero initialized.
   \param errorString A pointer to a char pointer. The encoder may
	return its error status here.

   \return Error code.
 */
enum EncoderError EncVorbisEncode(struct Encoder *enc,
				  const char **errorString);
enum EncoderError EncVorbisEncode1053(struct Encoder *enc,
				      const char **errorString);

#endif /* !ASM */

/* Default stream serial number is 0x12345678, but may be changed if
   this variable is set before calling EncVorbisCreate() */
extern u_int32 __mem_y encVStreamSerialNumber;

#ifdef __VSDSP__
extern u_int16 preloadResult;
u_int16 PreloadEncV(void);
void UnloadEncV(void);
#endif /* __VSDSP__ */
#endif /* ENCODER_VORBIS_H */
