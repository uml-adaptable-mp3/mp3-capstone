#include <vo_stdio.h>
#include <vsos.h>
#include <volink.h> // Linker directives like DLLENTRY
#include <vstypes.h>
#include <stdlib.h>
#include <string.h>
#include <apploader.h> // RunLibraryFunction etc
#include <timers.h>
#include <libaudiodec.h>
#include <vsostasks.h>
#include <taskandstack.h>
#include <consolestate.h>
#include <kernel.h>
#include <unistd.h>
#include <uimessages.h>         // Volume control and reporting
#include <aucommon.h>
#include <cyclic.h>
#include <extSymbols.h>

#include "volume.h"

/* Make this a unique 16-bit library identifier */
#define LIB_ID 0x2c91

s_int16 volume = 90; // 25 - (90/6) = volume 10 to start

void CycVolume(register struct CyclicNode *cyclicNode) {
  volume;
}
struct CyclicNode cycVolumeNode = {{0}, CycVolume};

// Turns the volume up one step
// returns updated current volume value
DLLENTRY(volumeUp)      // ENTRY_1
u_int16 volumeUp(void) {
    // Only turn volume up if it's not its maximum value
	// NOTE: 0 is maximum value since this is not actually controlling volume
	//       but rather the internal attenuation of the pre-amplifier
	//       So, 0 attenuation is the loudest volume
	// Each digit is 0.5dB of attenuation, so each volume step is 3dB
	if (volume > 0) {
		volume = volume - 6;
        ioctl(stdaudioout, IOCTL_AUDIO_SET_VOLUME, (void *)(volume+256));
	}
    else {
        printf("NOTICE: Volume already set to max\n");
    }

    printf("~%04x=%d\n", UIMSG_S16_SET_VOLUME, 25-volume/6);

    // Delay(100);
    return volume;
}

// Turns the volume down one step
// returns updated current volume value
DLLENTRY(volumeDown)    // ENTRY_2
u_int16 volumeDown(void) {
    // Only turn volume down if it's not its minimum value
	// NOTE: 0 is maximum value since this is not actually controlling volume
	//       but rather the internal attenuation of the pre-amplifier
	//       So, 180 attenuation is no volume
	// Each digit is 0.5dB of attenuation, so each volume step is 3dB
    if (volume < 150) {
        volume = volume + 6;
        if (volume >= 150) {
            // mute when volume is too low to hear
            ioctl(stdaudioout, IOCTL_AUDIO_SET_VOLUME, (void *)(500));
        }
        else {
            ioctl(stdaudioout, IOCTL_AUDIO_SET_VOLUME, (void *)(volume+256));
        }
    }
    else {
        printf("NOTICE: Volume already set to min\n");
    }
    
    printf("~%04x=%d\n", UIMSG_S16_SET_VOLUME, 25-volume/6);
    
    // Delay(100);
    return volume;
}

int main(char *parameters) {
    char *vol_ctrl = parameters;
    
    if (*vol_ctrl == '+') {
        printf("Volume Up\n");
        volumeUp();
    }
    else if (*vol_ctrl == '-') {
        printf("Volume Down\n");
        volumeDown();
    }
    else {
        // Sets the volume to 15 - a reasonable volume to start at
        ioctl(stdaudioout, IOCTL_AUDIO_SET_VOLUME, (void *)(volume+256));
    }
}

/* This driver runs a cyclic process and adds a new symbol _cycVolume
   to the system. This can be accessed through C with the name
   cycVolume. */
ioresult init(char *parameters) {
  /* Add a new symbol */
  AddSymbol("_cycVolume", (void *)LIB_ID, (int)(&volume));
  /* Activate the cyclic function. */
  AddCyclic(&cycVolumeNode, TICKS_PER_SEC/10, TICKS_PER_SEC/10);
}

void fini(void) {
  /* Stop the cyclic function. */
  DropCyclic(&cycVolumeNode);
  /* Remove all symbols associated with LIB_ID. */
  SymbolDeleteLib((void *)LIB_ID);
}