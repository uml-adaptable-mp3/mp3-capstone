/**
   \file encodermp3.h MPEG 1.0/2.0/2.5 MP3 Encoder.
*/
#ifndef ENCODER_MP3_H
#define ENCODER_MP3_H

#include <vstypes.h>
#include <encoder.h>

#ifndef ASM


/**
   Create Encoder. Upon success a pointer to the Encoder is returned.

   \param enc An Mp3 Encoder structure.
   \param es User-supplied encoder services with appropriate fields
	filled. The fields that are not to be filled by the user
	should be zero initialized.
   \param sampleRate The sample rate for audio.
   \param quality Quality parameter, which consists of several fields.
	mode = quality[15:14], mult = quality[13:12], param = quality[11:10].
      If mode = 0, quality = param (limited betweem 0..10, 10 is best).
      if mode = 1 (VBR), 2 (ABR) or 3 (CBR), suggested bitrate =
      10^(mult+1). E.g. 0xE080 = CBR 128*1000 bit/s = CBR 128 kbit/s.
      Encoders are at freedom of implementing only some of the VBR, ABR
      and CBR options and use their best equivalents for missing modes.
      The MP3 encoder implements Quality, VBR and CBR. If ABR is selected,
      you get VBR instead.
   \return Pointer to the Encoder if successful, otherwise NULL.
 */
struct Encoder *EncMp3Create(struct EncoderServices *es,
				u_int16 channels, u_int32 sampleRate,
				u_int16 quality);

/**
   Encode file. Upon success or a negative number, Encoder has succeeded.
   With a positive number, there has been an error. Upon return, an
   error string is also returned.

   \param enc A Mp3 Encoder structure.
   \param es User-supplied encoder services with appropriate fields
	filled. The fields that are not to be filled by the user
	should be zero initialized.
   \param errorString A pointer to a char pointer. The encoder may
	return its error status here.
   \return Error code.
 */
enum EncoderError EncMp3Encode(struct Encoder *enc,
				  const char **errorString);
enum EncoderError EncMp3Encode1053(struct Encoder *enc,
				      const char **errorString);


/**
   Free all resources allocated for encoder.

   \param enc A Mp3 Encoder structure.
*/
void EncMp3Delete(struct Encoder *enc);

#endif /* !ASM */

#endif /* ENCODER_MP3_H */
