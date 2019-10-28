/**
   \file encoderflac.h Flac 1.0 Encoder.
*/

#ifndef ENCODER_FLAC_H
#define ENCODER_FLAC_H

#include <vstypes.h>
#include <encoder.h>
#include <filter.h>

#ifndef ASM
/* Create functions in encFlacCreate.h and encFlacCreatePreeet.h */

/**
   Free all resources allocated for encoder.

   \param enc A Flac Encoder structure.
*/
void EncFlacDelete(struct Encoder *enc);
void EncFlacDelete1053(struct Encoder *enc);

/**
   Encode file. Upon success or a negative number, Encoder has succeeded.
   With a positive number, there has been an error. Upon return, an
   error string is also returned.

   \param enc A Flac Encoder structure.
   \param es User-supplied encoder services with appropriate fields
	filled. The fields that are not to be filled by the user
	should be zero initialized.
   \param errorString A pointer to a char pointer. The encoder may
	return its error status here.

   \return Error code.
 */
enum EncoderError EncFlacEncode(struct Encoder *enc,
				  const char **errorString);
enum EncoderError EncFlacEncode1053(struct Encoder *enc,
				      const char **errorString);

#endif /* !ASM */

/* Default stream serial number is 0x12345678, but may be changed if
   this variable is set before calling EncFlacCreate() */
extern u_int32 __mem_y encVStreamSerialNumber;

#ifdef __VSDSP__
extern u_int16 preloadResult;
u_int16 PreloadEncF(void);
void UnloadEncF(void);
#endif /* __VSDSP__ */
#endif /* ENCODER_FLAC_H */
