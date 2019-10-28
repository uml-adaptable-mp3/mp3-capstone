/* For free support for VSIDE, please visit www.vsdsp-forum.com */

// Starting point template for creating VSOS applications.
// This will create a <projectname>.DL3 file, which you can copy to 
// your VS1005 Developer Board's system disk's SYS subdirectory.

// DL3 files require VSOS kernel version 0.3x to run.

// If you add the libary name to S:CONFIG.TXT, it will be loaded
// and run during boot-up. 

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <timers.h>
#include <libaudiodec.h>
#include <vsostasks.h>
#include <consolestate.h>
#include <kernel.h>

char *filename;
void *decoderLibrary = NULL;
AUDIO_DECODER *audioDecoder = NULL;
FILE *fp = NULL;
char *errorString = "nothing to see here";
int eCode = 0;


void PlayerThread(void) {
	eCode = DecodeAudio(decoderLibrary, audioDecoder, &errorString);
}


int main(char *parameters) {
	filename = parameters;
	

	//printf("Load library\n");
	decoderLibrary = LoadLibrary("audiodec");
	if (!decoderLibrary) {
		printf("Couldn't open library\n");
		return S_ERROR;
	}

	//printf("Open file\n");
	fp = fopen(filename, "rb");
	if (!fp) {
		printf("Couldn't open file '%s'\n",filename);
		return S_ERROR;
	}
	printf("Playing '%s'\n",fp->Identify(fp,NULL,0));
	
	RunLibraryFunction("METADATA",ENTRY_2,(int)fp);

	audioDecoder = CreateAudioDecoder(decoderLibrary, fp, stdaudioout, NULL, auDecFGuess);
	if (!audioDecoder) {
		printf("Couldn't create audio decoder\n");
		return S_ERROR;
	}

	StartTask(TASK_DECODER, PlayerThread); 
	Delay(100);		
	while (pSysTasks[TASK_DECODER].task.tc_State && pSysTasks[TASK_DECODER].task.tc_State != TS_REMOVED) {
		
		printf("\r[%02ld:%02ld]",audioDecoder->cs.playTimeSeconds/60L,audioDecoder->cs.playTimeSeconds%60L);

		Delay(250);
		if (ioctl(stdin,IOCTL_TEST,NULL)) {
			//character(s) available in the stdin buffer
			char c = fgetc(stdin);	
			if (c=='q') {
				printf("\rQuit... \n");
				audioDecoder->cs.cancel = 1;
			}
		}
		
		if (appFlags & APP_FLAG_QUIT) {
			audioDecoder->cs.cancel = 1;
		}

		Delay(250);
	}
	
	fclose(fp);	

	printf("Decode finished.\n");
	//eCode = DecodeAudio(decoderLibrary, audioDecoder, &errorString);	
	//printf("errorCode %d, errortext %s\n", eCode, errorString);
	//printf("Delete the decoder\n");
	DeleteAudioDecoder(decoderLibrary, audioDecoder);

	//printf("Removing the library from memory\n");
	DropLibrary(decoderLibrary);

	//printf("Now we are ready.\n");
	Delay(100);
}

// Library finalization code
DLLENTRY(fini)
void fini(void) {
	//printf("Library Unloading.\n");
	// Add code here to force release of resources such as 
	// memory allocated with malloc, entry points, 
	// hardware locks or interrupt handler vectors.
}
