#include <vsos.h>
#include <uimessages.h>
#include <audio.h>
#include <audiofs.h>
#include <cyclic.h>
#include <timers.h>
#include "sysuimsg.h"
#include <aucommon.h>

static s_int16 balance = 0;
static s_int16 masterVolume = 0;
static u_int16 volumeChanged = 1;



void SetStdAudioOutVolume() { //register u_int16 left, register u_int16 right) {
	s_int16 leftVolume;
	s_int16 rightVolume;
	s_int16 volume = masterVolume;
	ioresult r;
	if (volume < 0) volume = 0;
	leftVolume = volume+balance;
	if (leftVolume<volume) leftVolume=volume;
	rightVolume = volume-balance;
	if (rightVolume<volume) rightVolume=volume;
	//volumeReg = (leftVolume<<8) | (rightVolume&0xff);
	//SetVolume();
	//ioctl(stdaudioout, IOCTL_AUDIO_SET_SPDIF_VOLUME, (void*)masterVolume);	
	volumeChanged = 1;	
}

__mem_y s_int16 filterValues[16] = {-32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768};

void SystemUiMessageCyclic(register struct CyclicNode *cyclicNode) {
	u_int16 i;
	s_int16 __mem_y *fVal = filterValues;
	for (i=0; i<16; i++) {
		if (*fVal != -32768) {
			struct FilterEqualizer ftEqu;
			ftEqu.filterNumber = i;
			if (ioctl(stdaudioout, IOCTL_AUDIO_GET_EQU_FILTER, &ftEqu) == S_OK) {
				ftEqu.gainDB = *fVal;
				//printf("Set %d to %d\n",i,*fVal);
				ioctl(stdaudioout, IOCTL_AUDIO_SET_EQU_FILTER, &ftEqu);
			}
			*fVal = -32768;
		}
		fVal++;
	}

	if (volumeChanged) {
		volumeChanged = 0;
		ioctl(stdaudioout, IOCTL_AUDIO_SET_VOLUME, (void*)(256+masterVolume));
	}
	
}

struct CyclicNode systemUiMessageCyclicNode = {{0},SystemUiMessageCyclic};


/*
void StartCyclic() {
	AddCyclic(&systemUiMessageCyclicNode, TICKS_PER_SEC/10, 0); 	
}
*/

int SystemUiMessageReceiver(s_int16 index, u_int16 message, u_int32 value) {
	if (message == UIMSG_S16_SET_VOLUME) {
		masterVolume = (s_int16)value;
		SetStdAudioOutVolume();
		return S_OK;
	}
	if (message == UIMSG_BUT_VOLUME_UP) {
		if (masterVolume) masterVolume--;
		SetStdAudioOutVolume();
		return masterVolume;
	}
	if (message == UIMSG_BUT_VOLUME_DOWN) {
		if (masterVolume<90) masterVolume++;
		SetStdAudioOutVolume();
		return masterVolume;
	}
	if (message == UIMSG_S16_SET_BALANCE) {
		balance = (s_int16)value;
		SetStdAudioOutVolume();
		return S_OK;
	}
	
	if ((message & 0xfff0) == UIMSG_S16_FILTER_1_GAIN) {
		filterValues[message & 0xf] = value;			
	}

	return S_UNKNOWN_MESSAGE;		
}

