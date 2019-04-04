#ifndef VSMPG_GENERAL_H
#define VSMPG_GENERAL_H

/*#define KILL_SAT*/
/*#undef KILL_SAT*/

#include "codMpg.h"

auto s_int16 myfread(unsigned char *ptr, s_int16 n, FILE *stream);
auto s_int16 my16fread(u_int16 __y *ptr, s_int16 n, FILE *stream);
auto s_int16 myfwrite(const void *ptr, s_int16 n, FILE *stream);
auto void mymem16cpy(u_int16 __y *d, s_int16 dOff, const u_int16 __y *s, s_int16 sOff, s_int16 n);
auto void InitGeneral(void);

auto s_int16 mygetc(void);


#ifdef STANDALONE
void PrintAudioLoops(void);
void PrintStreamLoops(void);
void Print32(char c, u_int32 n);
void Print16(char c, u_int16 n);

extern s_int16 audio_rd_loops;
extern s_int16 audio_wr_loops;
extern s_int16 stream_rd_loops;
extern s_int16 stream_wr_loops;

extern s_int16 (*applAddr)(s_int16 register __i0 **d,
			   s_int16 register __a1 mode,
			   s_int16 register __a0 n);

auto s_int16 MyGetC(void);
auto void MyGetCPairs(register __i0 __y void *p, register __a0 s_int16 n);
#ifdef STANDALONE
auto void WaitForData(register __c0 u_int16 n);
#endif
#ifdef USE_VORBIS
auto s_int16 ReadStream(register __i3 u_int16 *buf,
			register __c1 s_int16 byteOff,
			register __c0 s_int16 byteSize);
#endif/*USE_VORBIS*/
extern s_int16 __y timeToRemovePDown2;

#ifdef USE_EARSPEAKER
extern __y s_int16 earSpeakerDisable;
extern __y u_int16 earSpeakerSetting;
extern __y u_int16 earSpeakerFreq;
extern __y s_int16 earSpeakerOld;
extern s_int16 earSpeakerVal[6];
#endif

#endif

#endif
