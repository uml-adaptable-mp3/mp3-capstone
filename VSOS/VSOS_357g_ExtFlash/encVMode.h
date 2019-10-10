#ifndef ENC_VORBIS_MODE_H
#define ENC_VORBIS_MODE_H

#include <vstypes.h>

#define QUALITIES

enum EncVAudioModeCode {
  vEncMode08k,
  vEncMode16k,
  vEncMode32k,
  vEncMode44k,
  vEncMode48k
};

struct EncVAudioMode {
  u_int16 sampleRate;
  u_int32 nominalBitRate[2][3];
  s_int16 bits[2];
  const s_int16 *bands[2];
  u_int16 freqLimit[3];		/* In Hz */
  u_int16 stereoLimit[3];	/* In Hz */
  double accuracy[3];		/* 1.0 = nominal */
  const double *means[2];
};

extern const struct EncVAudioMode evAudioMode[];

#endif /* ENC_VORBIS_MODE_H */
