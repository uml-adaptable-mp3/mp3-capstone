

/* For free support for VSIDE, please visit www.vsdsp-forum.com */

// Starting point template for creating VSOS applications.
// This will create a <projectname>.DL3 file, which you can copy to 
// your VS1005 Developer Board's system disk's SYS subdirectory.

// DL3 files require VSOS kernel version 0.3x to run.

// If you add the libary name to S:CONFIG.TXT, it will be loaded
// and run during boot-up. 

/// \playfiles.c Command-line driven utility to play multiple files from a single filespec
/// For example, write "PlayFiles E:MUSIC/*.MP3" in your CONFIG.TXT /**/
 

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <timers.h>
#include <libaudiodec.h>
#include <vsostasks.h>
#include <consolestate.h>
#include <ctype.h>
#include <vo_gpio.h>
#include <kernel.h>

void *decoderLibrary = NULL;
AUDIO_DECODER *audioDecoder = NULL;
char *errorString = "";
int eCode = 0;
volatile int fileNum = 0;
int running = 1;

void PlayerThread(void) {
	eCode = DecodeAudio(decoderLibrary, audioDecoder, &errorString);
}


ioresult PlayFile(FILE *fp) {
	audioDecoder = CreateAudioDecoder(decoderLibrary, fp, stdaudioout, NULL, auDecFGuess);
	if (!audioDecoder) {
		printf("Couldn't create audio decoder\n");
		return S_ERROR;
	}

	StartTask(TASK_DECODER, PlayerThread);
	Delay(100);		
	while (pSysTasks[TASK_DECODER].task.tc_State && pSysTasks[TASK_DECODER].task.tc_State != TS_REMOVED) {
		Delay(100);
		if (appFlags & APP_FLAG_QUIT) {
			audioDecoder->cs.cancel = 1;
			running = 0;
		}
		

		// Check for UART control
		if (ioctl(stdin,IOCTL_TEST,NULL) > 0) {
			char c = fgetc(stdin);
			
			if (c == 'Q') { // Q Quit
				audioDecoder->cs.cancel = 1;
				running = 0;
			}

			if (c == '>') { // > Next
				audioDecoder->cs.cancel = 1;
			}
			if (c == '<') { // < Previous
				audioDecoder->cs.cancel = 1;
				fileNum -= 2;
			}
			
			if (isdigit(c)) { // number:select file
				fileNum = c-'0';
				audioDecoder->cs.cancel = 1;				
			}
		}
		
		if (GpioReadPin(0x00)) { // GPIO0.0: Button S1: Next
			audioDecoder->cs.cancel = 1; //Cancel playback, playback will continue at next song
		}


		if (GpioReadPin(0x01)) { // GPIO0.1: Button S2: Previous
			audioDecoder->cs.cancel = 1; //Cancel playback, decrement fileNum by 2 so that after it is autoincremented by one, it will point to the previous song.
			fileNum -= 2;
			if (fileNum < -1) {
				fileNum = -1;
			}
		}

	}

	
	//printf("Decode returned %d, %s.\n", eCode, errorString);

	DeleteAudioDecoder(decoderLibrary, audioDecoder);
	return S_OK;
}


int main(char *filespec) {
	FILE *fp = NULL;
	static char fnumMode[10];


	//kernelDebugLevel = 99; //get lots of kernel debug output to stderr (UART)
	printf("PlayFiles %s\n",filespec);

	decoderLibrary = LoadLibrary("audiodec");
	if (!decoderLibrary) {
		return S_ERROR;
	}

	fileNum = 0;

	while(running) {
		
		if (appFlags & APP_FLAG_QUIT) {
			break;
		}
		
		printf("Open file %d...\n",fileNum+1);

		sprintf(fnumMode,"rb#%d",fileNum);
		fp = fopen(filespec,fnumMode); // e.g. fopen("N:MUSIC\*","rb#123");
		if (fp) {
			printf("Playing '%s' ",fp->Identify(fp,NULL,0)); //this Identify is good to be right after the fopen
			printf("from device %s...\n",fp->dev->Identify(fp->dev,NULL,0));
	
			RunLibraryFunction("METADATA",ENTRY_2,(int)fp);

			PlayFile(fp);
			fclose(fp);
			fileNum++;
		} else {
			printf("File %d not found, finished playing.\n",fileNum+1);
			//fileNum=0; //Restart from first file
			break; //end of files, end playing
		}
	}

	DropLibrary(decoderLibrary);
	return S_OK; //End of files, ok exit
}
