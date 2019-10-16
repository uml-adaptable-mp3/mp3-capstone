#ifndef __AUDIO_H__
#define __AUDIO_H__

#include <vs1005h.h>

#if 0
#define WITH_EARSPEAKER
#endif
#define DIRECT_VORBIS_BLOCKSIZE
#define UI_TICKS 32 /* should be a power of two */

#define USE_TIMER
#ifdef USE_TIMER
#define TIMER_TICKS 1000 /* 1000Hz */
#ifndef ASM
extern __y u_int32 timeCount;
u_int32 ReadTimeCount(void);
#endif
#endif

#define DEFAULT_AUDIO_BUFFER_SAMPLES 2048
#define DAC_DEFAULT_SAMPLERATE 8000
#define DAC_DRIVER_ON_DELAY (DAC_DEFAULT_SAMPLERATE/10) /* 100ms In samples */

#define APPL_RESET	 0
#define APPL_AUDIO	 1
#define APPL_BITSTREAM	10


#ifdef ASM
	.import _audioPtr
#define AUDIOPTR_WR 0
#define AUDIOPTR_RD 1
#define AUDIOPTR_FORWARD_MODULO 2
#define AUDIOPTR_LEFTVOL 3
#define AUDIOPTR_RIGHTVOL 4
#define AUDIOPTR_UNDERFLOW 5
#else

extern s_int16 (*applAddr)(s_int16 register __i0 **d,
			   s_int16 register __a1 mode,
			   s_int16 register __a0 n);

extern __y s_int16 audioBuffer[2*DEFAULT_AUDIO_BUFFER_SAMPLES];

struct AUDIOPTR {
    __y s_int16 *wr;       /* 0: write pointer */
    __y s_int16 *rd;       /* 1: read pointer */
    u_int16 forwardModulo; /* 2: 0x8000 + size - 1 */
    s_int16 leftVol;       /* 3: left volume,  default     -32768 =  1.0 */
    s_int16 rightVol;      /* 4: right volume, differential 32767 = -1.0 */
    s_int16 underflow;     /* 5: set if underflow in dac interrupt */
};
extern __y struct AUDIOPTR audioPtr;

extern u_int16 earSpeakerReg;
extern u_int16 volumeReg;
extern u_int16 bassReg;
extern __y u_int16 oldExtClock4KHz, oldClockX, oldOldClock4KHz;
extern u_int32 __y curFctl;
extern __y s_int32 hwSampleRate;
extern __y u_int16 uiTime;
extern __y u_int16 uiTrigger;
extern s_int16 __y timeToRemovePDown2;
extern u_int32 __y haltTime;
extern __y u_int16 uartByteSpeed;

extern __y struct EARSPEAKER {
    u_int16 Freq;
    u_int16 Disable;
    u_int16 Setting;
    s_int16 Old;
    u_int16 longFrames;
} earSpeaker;
extern s_int16 tmpBuf[2*32];


void InitAudio(void); /* initializes the audioPtr structure */
auto void StereoCopy(register __i2 s_int16 *s, register __a0 u_int16 n);
s_int16 AudioBufFill(void); /* how many STEREO samples to play */
s_int16 AudioBufFree(void); /* how many STEREO samples fit */

auto void SetRate(register __reg_c s_int32 sampleRate);
auto void OldSetRate(register __c1 u_int16 sampleRate); // Obsolete!
void AudioAdjustRate(register __c1 s_int16 adjustment);
auto void SetVolume(void);
auto void RealSetVolume(void);

/* Earspeaker is available only when framesize <= 1024. */
void SetAudioFrameSize(u_int16 frameSize);

auto void AudioOutputSamples(s_int16 *p, s_int16 samples);
auto void RealAudioOutputSamples(s_int16 *p, s_int16 samples);

u_int16 UartDivider(u_int16 ck4kHz, u_int16 byteSpeed);

#endif/*!ASM*/


#endif/*__AUDIO_H__*/

