#include <vstypes.h>
#include <vo_stdio.h>
#include <vo_gpio.h>
#include <volink.h>
#include <stdlib.h>
#include <sysmemory.h>
#include <string.h>
#include <time.h>
#include <uimessages.h>         // Volume control and reporting
#include <audio.h>
#include <aucommon.h>


#include "volume.h"

// Turns the volume up one step
// returns updated current volume value
u_int16 volumeUp(s_int16 currVol) {
    // Only turn volume up if it's not its maximum value
	// NOTE: 0 is maximum value since this is not actually controlling volume
	//       but rather the internal attenuation of the pre-amplifier
	//       So, 0 attenuation is the loudest volume
	// Each digit is 0.5dB of attenuation, so each volume step is 3dB
	if (currVol > 0) {
		currVol = currVol - 6;
        ioctl(stdaudioout, IOCTL_AUDIO_SET_VOLUME, (void *)(currVol+256));
	}
    else {
        printf("NOTICE: Volume already set to max\n");
    }

    printf("~%04x=%d\n", UIMSG_S16_SET_VOLUME, 30-currVol/6);

    Delay(100);
    return currVol;
}

// Turns the volume down one step
// returns updated current volume value
u_int16 volumeDown(s_int16 currVol) {
    // Only turn volume down if it's not its minimum value
	// NOTE: 0 is maximum value since this is not actually controlling volume
	//       but rather the internal attenuation of the pre-amplifier
	//       So, 180 attenuation is no volume
	// Each digit is 0.5dB of attenuation, so each volume step is 3dB
    if (currVol < 180) {
        currVol = currVol + 6;
        ioctl(stdaudioout, IOCTL_AUDIO_SET_VOLUME, (void *)(currVol+256));
    }
    else {
        printf("NOTICE: Volume already set to min\n");
    }
    
    printf("~%04x=%d\n", UIMSG_S16_SET_VOLUME, 30-currVol/6);
    
    Delay(100);
    return currVol;
}