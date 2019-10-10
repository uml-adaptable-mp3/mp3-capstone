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
#include <string.h>
#include <kernel.h>

char *filename;
void *decoderLibrary = NULL;
AUDIO_DECODER *audioDecoder = NULL;
FILE *fp = NULL;
char *errorString = "nothing to see here";
int eCode = 0;

struct CodecLoop codecLoop = {0};

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

	/* Initialize loop: write all values first, and as the last thing set cs->loop.
	   Do not modify loop parameters while looping, except for numberOfLoops (to have
	   infinite looping, set it to 0xFFFFFFFFU, to stop looping, clear it. */
	{
		struct CodecServices *cs = &audioDecoder->cs;
		memset(&codecLoop, 0, sizeof(codecLoop));
#if 1
		codecLoop.startSeconds = 2;
		codecLoop.startSamples = 1234;
		codecLoop.endSeconds = 4;
		codecLoop.endSamples = 5678;
#else
		codecLoop.startSeconds = 1324;
		codecLoop.startSamples = 0;
		codecLoop.endSeconds = 1340;
		codecLoop.endSamples = 0;
#endif
		codecLoop.numberOfLoops = 0x7fffffff;
#if 1
		audioDecoder->cs.loop = &codecLoop;
#endif
	}

	StartTask(TASK_DECODER, PlayerThread); 
	Delay(100);		
	//	audioDecoder->cs.goTo = 1320;
	while (pSysTasks[TASK_DECODER].task.tc_State && pSysTasks[TASK_DECODER].task.tc_State != TS_REMOVED) {
		static u_int32 oldSeconds = -1;
		if (audioDecoder->cs.playTimeSeconds != oldSeconds) {
		  oldSeconds = audioDecoder->cs.playTimeSeconds;
		  printf("\n%[%02ld:%02ld]\n",
			 oldSeconds/60L, oldSeconds%60L);
		}

		Delay(1);
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

		Delay(1);
	}
	
	fclose(fp);	

	printf("\nDecode finished.\n");
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
