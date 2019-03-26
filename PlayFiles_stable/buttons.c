#include <vo_stdio.h>
#include <vo_gpio.h>
#include <libaudiodec.h>
#include <audio.h>
#include <aucommon.h>
#include <uimessages.h>
#include "buttons.h"

extern AUDIO_DECODER *audioDecoder;
extern u_int16 repeatSongStatus;
extern u_int16 shuffle;
extern s_int16 volume;
extern u_int16 mute;
extern volatile int fileNum;
extern u_int16 running;
extern u_int16 seed;
extern u_int16 anyButtonPressed;
extern u_int16 idleMode;

extern s_int16 arrowSelection;
extern s_int16 offset;

// NOTE: Every function to perform an action through a button has a button and active state as inputs
//       to the function. This allows for the functions to be mapped to different buttons and toggled with
//       an active high or active low signal incredibly easily

/*************/
/* FUNCTIONS */
/*************/

// Cancels currently playing song so that it appropriately moves on to the next song
// Next song can vary based on repeat song or shuffle songs set
u_int16 nextSong(register u_int16 button, u_int16 activeHigh)
{
	if ((GpioReadPin(button) & activeHigh) || (GpioReadPin(button) & !activeHigh)) 
	{
		printf("Next song button pressed! \n");
		audioDecoder->cs.cancel = 1;
		repeatSongStatus = 0;
		anyButtonPressed = 1;
		idleMode = 0;
		while (GpioReadPin(button)) {}
		return 1;
	}
	return 0;
}

// Sets a repeat variable so that when a song ends, the file index is not changed.
// This has precedence over next/previous song, so if a user tries to change songs
// this way, it won't let you. Also if a song ends on its own, it will repeat
// It doesn't have precedence over selecting a specific song or shuffle song being set though
u_int16 repeatSong(register u_int16 button, u_int16 activeHigh)
{
	if ((GpioReadPin(button) & activeHigh) || (GpioReadPin(button) & !activeHigh)) 
	{
		repeatSongStatus = !repeatSongStatus;
		printf("Repeat - %i \n", repeatSongStatus);
		anyButtonPressed = 1;
		idleMode = 0;
		while (GpioReadPin(button)) {}
		return 1;
	}
	return 0;
}

// Generates a random seed value used to shuffle songs. When a user presses next/previous,
// It will pick a random song. A random song is also picked if a song naturally ends.
// NOTE : Typically systems only shuffle when next song is chosen or the song ends,
//        a future improvement would be previous song history so that this can be done
u_int16 shuffleSong(register u_int16 button, u_int16 activeHigh)
{
	if ((GpioReadPin(button) & activeHigh) || (GpioReadPin(button) & !activeHigh)) 
	{
		seed = 0;
		shuffle = !shuffle;
		printf("Shuffle - %i \n", shuffle);
		
		// Generates a random seed value based on how long it takes for the user to
		// release the button, which allows more randomness to be introduced to the LFSR
		while (GpioReadPin(button)) 
		{
			seed++;
		}
		
		// There's a small chance that the field overflows and resets back to 0 and stops
		// perfectly here, which would cause the LFSR to not function, so assign a default
		// seed in this rare instance
		if (seed == 0)
		{
			seed = DEFAULT_SEED;
		}

		anyButtonPressed = 1;
		idleMode = 0;
		return 1;
	}
	return 0;
}

u_int16 playPause(register u_int16 button, u_int16 activeHigh)
{
	// NOT current status to properly toggle it from on to off
	if ((GpioReadPin(button) & activeHigh) || (GpioReadPin(button) & !activeHigh)) 
	{
		audioDecoder->pause = !audioDecoder->pause;
		
		#if 1
		   if (audioDecoder->pause) 
		   {
			   printf("PAUSING \n");
		   }
		   else 
		   {
			   printf("PLAYING \n");
		   }
		#endif
		
		anyButtonPressed = 1;
		idleMode = 0;
		// Wait until button released so it doesn't keep toggling
		while (GpioReadPin(button)) {}
		
		return 1;
	}
	return 0;
}

// Controls volume by adjusting the internal attentuation of the pre-amplifier
// inside the SoC
u_int16 volumeUp(register u_int16 button, u_int16 activeHigh)
{
	// Whenever the button is pressed, adjust the volume
	if ((GpioReadPin(button) & activeHigh) || (GpioReadPin(button) & !activeHigh)) 
	{
		// Only turn volume up if it's not its maximum value
		// NOTE: 0 is maximum value since this is not actually controlling volume
		//       but rather the internal attenuation of the pre-amplifier
		//       So, 0 attenuation is the loudest volume
		// Each digit is 0.5dB of attenuation, so each volume step is 3dB
		if (volume > 0)
		{
			volume = volume - 6;
		}
		
		// Command to control volume, as shown in sample code from VLSI FI
		ioctl(stdaudioout, IOCTL_AUDIO_SET_VOLUME, (void *)(volume+256));
       printf("~%04x=%d\n", UIMSG_S16_SET_VOLUME, 30-volume/6);
     
		// Wait 300ms before exiting so that if the volume button is held it 
		// will continue to increment
		Delay(300);
		anyButtonPressed = 1;
		idleMode = 0;
		return 1;
	}	
	return 0;
}

u_int16 volumeDown(register u_int16 button, u_int16 activeHigh)
{
	if ((GpioReadPin(button) & activeHigh) || (GpioReadPin(button) & !activeHigh)) 
	{
		if (volume < 180)
		{
			volume = volume + 6;
		}
		
		ioctl(stdaudioout, IOCTL_AUDIO_SET_VOLUME, (void *)(volume+256));
       printf("~%04x=%d\n", UIMSG_S16_SET_VOLUME, 30-volume/6);
       
		Delay(300);
		anyButtonPressed = 1;
		idleMode = 0;
		return 1;
	}
	return 0;
}

u_int16 volumeMute(register u_int16 button, u_int16 activeHigh)
{
	if ((GpioReadPin(button) & activeHigh) || (GpioReadPin(button) & !activeHigh)) 
	{
		mute = !mute;
		printf("Mute - %i \n", mute);
		
		if (mute)
		{
			ioctl(stdaudioout, IOCTL_AUDIO_SET_VOLUME, (void *)(MUTE_VOLUME+256));
		}
		else
		{
			ioctl(stdaudioout, IOCTL_AUDIO_SET_VOLUME, (void *)(volume+256));
		}
		
		anyButtonPressed = 1;
		idleMode = 0;
		while (GpioReadPin(button)) {}
		return 1;
	}
	return 0;
}

u_int16 stopSong(register u_int16 button, u_int16 activeHigh)
{
	if ((GpioReadPin(button) & activeHigh) || (GpioReadPin(button) & !activeHigh)) 
	{
		printf("Song stopped! \n");
		audioDecoder->cs.cancel = 1;
		running = 0;
		anyButtonPressed = 1;
		idleMode = 0;
		return 1;
	}
	return 0;
}

u_int16 previousSong(register u_int16 button, u_int16 activeHigh)
{
	if ((GpioReadPin(button) & activeHigh) || (GpioReadPin(button) & !activeHigh)) 
	{ 
		printf("previous Song hit! \n");
		// Cancel playback, decrement fileNum by 2 so that after it is autoincremented by one, it will point to the previous song.
		audioDecoder->cs.cancel = 1;
		// Any time the song is changed through next/previous, shut off song repeat and shuffle so that the indexing is correct.
		repeatSongStatus = 0;
		shuffle = 0;
		
		if (audioDecoder->cs.playTimeSeconds < 3)
		{
			fileNum -= 2;
		}
		else
		{
			fileNum -= 1;
		}

		if (fileNum < -1) 
		{
			fileNum = -1;
		}
		
		anyButtonPressed = 1;
		idleMode = 0;
		while (GpioReadPin(button)) {}
		return 1;
	}
	
	return 0;
}