#include "voaudio.h"
#include <string.h>
#include <stdlib.h>
#include "audiofs.h"
#include "audio.h"

auto void OldStereoCopy(register __i2 s_int16 *s, register __a0 u_int16 n);
auto void StereoCopy(register __i2 s_int16 *s, register __a0 u_int16 n);
// StereoCopy hook JMPIs to OldStereoCopy. Could be a good location to install bass/treble etc
void AudioIdleHook(void);

extern __y struct AUDIOPTR audioPtr;
extern __y u_int32 timeCount;
u_int16 voAudioChannels = 2;

IOCTL_RESULT AudioIoctl(register __i0 VO_FILE *self, s_int16 request, IOCTL_ARGUMENT arg){
	switch(request) {
		case IOCTL_RESTART: {
			PERIP(INT_ENABLE0_HP) &= ~INTF_DAC;
			InitAudio();	
		}
		//Intentional fall-through, do not put anything here!!!
		case IOCTL_AUDIO_SET_ORATE: {
			#ifdef IOCTL_HAS_32_BITS
				if (!arg) arg=48000;
				SetRate(arg);
			#else 			
				register s_int32 rate = *((s_int32*)arg);
				if (!rate) rate=48000;
				SetRate(rate);
			#endif
			PERIP(INT_ENABLE0_HP) |= INTF_DAC;
			break;
		}	

		case IOCTL_AUDIO_SET_OCHANNELS: {
			voAudioChannels = arg;
			break;
		}

		case IOCTL_AUDIO_SET_VOLUME: {
			volumeReg = (u_int16)arg - 256;
			if (volumeReg >= 0x100) {
				volumeReg = 0;
			} else {
				volumeReg *= 0x0101;
			}
			SetVolume();	
		}

		default: {
			//printf("AuIoCt%d ",request);
			return S_ERROR;
			break;
		}

	}
	return S_OK;
}

u_int16 AudioWrite(register __i0 VO_FILE *self, void *bufV, u_int16 sourceIndex, u_int16 bytes) {
	s_int16 *buf = bufV;
	if (voAudioChannels == 2) {
		// Stereo
		AudioOutputSamples(buf, bytes/4); //StereoCopy hook -> -> OldStereoCopy
	} else {
		// Mono implementation is inefficient, but really small
		u_int16 d[2];
		u_int16 samples = bytes/2+1;
		while (--samples) {
			d[0] = d[1] = *buf++;
			AudioOutputSamples(d, 1);
		}
	}
}


const FILEOPS AudioFileOperations = {
	CommonOkResultFunction, //AudioOpen, 
	CommonOkResultFunction, //AudioClose, 
	AudioIoctl, 
	AudioWrite,//Not real AudioRead 
	AudioWrite
};

const SIMPLE_FILE audioout = {__MASK_FILE | __MASK_OPEN | __MASK_WRITABLE | __MASK_PRESENT, 
	NULL, &AudioFileOperations};
	
