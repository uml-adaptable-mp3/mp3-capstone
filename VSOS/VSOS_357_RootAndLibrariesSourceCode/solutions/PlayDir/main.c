/*

  PlayDir

*/
#include <vo_stdio.h>
#include <volink.h>     /* Linker directives like DLLENTRY */
#include <apploader.h>  /* RunLibraryFunction etc */
#include <timers.h>
#include <libaudiodec.h>
#include <vsostasks.h>
#include <consolestate.h>
#include <ctype.h>
#include <uimessages.h>
#include <string.h>
#include <stdlib.h>
#include <audio.h>
#include <aucommon.h>
#include <sysmemory.h>
#include <kernel.h>


void *decoderLibrary = NULL;
AUDIO_DECODER *audioDecoder = NULL;
char *errorString = "";
int eCode = 0;
int fileNum = 0, files = 0, shuffle = 0;
int running = 1;
int pause = 0;
int playSpeed = 1;
double pitchShift = 1.0, speedShift = 1.0;
int adjustPitch = 0, adjustSpeed = 0;

#define IDX_SIZE 1024
u_int16 __mem_y *idx=NULL, *idxShuffle=NULL;

#define MAX_CMD_LEN 80
char cmd[MAX_CMD_LEN] = "";

/* These defines are missing from VSOS releases prior to 3.42 */
#ifndef IOCTL_AUDIO_GET_PITCH
#define IOCTL_AUDIO_GET_PITCH		270
#define IOCTL_AUDIO_SET_PITCH		271
#define IOCTL_AUDIO_GET_SPEED		272
#define IOCTL_AUDIO_SET_SPEED		273
#endif /* !IOCTL_AUDIO_GET_SPEED */

/* These defines are missing from VSOS releases prior to 3.42 */
#ifndef UIMSG_S16_SET_SPEED
#define UIMSG_S16_SET_SPEED 0x020A
#define UIMSG_S16_SET_PITCH 0x020B
#endif /* !UIMSG_S16_SET_SPEED */

void PlayerThread(void) {
  eCode = DecodeAudio(decoderLibrary, audioDecoder, &errorString);
}

void MakeShuffleList(void) {
  int i;
  for (i=0; i<files; i++) {
    idxShuffle[i] = i;
  }
  if (shuffle && files) {
    u_int16 mask = 1;
    srandom(ReadTimeCount());
    while (mask < files) {
      mask = (mask<<1) + 1;
    }
    for (i=0; i<files; i++) {
      u_int16 t, j;
      while ((j=(u_int16)random()&mask) >= files-i)
	;
      t = idxShuffle[i];
      idxShuffle[i] = idxShuffle[i+j];
      idxShuffle[i+j] = t;
    }
  } /* if (shuffle && files) */
}

ioresult PlayFile(FILE *fp) {
  s_int32 lastSeconds = -1, newSeconds;
  s_int16 lastPerCent = -1;
  static s_int16 cmdLen = -1;

  audioDecoder =
    CreateAudioDecoder(decoderLibrary, fp, stdaudioout, NULL, auDecFGuess);

  if (!audioDecoder) {
    printf("!createAuDec\n");
    return S_ERROR;
  }

  audioDecoder->pause = pause;
  audioDecoder->cs.fastForward = playSpeed;

  /*eCode = DecodeAudio(decoderLibrary, audioDecoder, &errorString);*/
  StartTask(TASK_DECODER, PlayerThread);
  Delay(100);		

  while (pSysTasks[TASK_DECODER].task.tc_State &&
	 pSysTasks[TASK_DECODER].task.tc_State != TS_REMOVED) {
    int hadAnythingToDo = 0;

    newSeconds = audioDecoder->cs.playTimeSeconds;

    if (newSeconds != lastSeconds && newSeconds >= 0 &&
	(cmdLen < 0 || !(appFlags & APP_FLAG_ECHO))) {
      s_int16 newPerCent = audioDecoder->cs.Tell(&audioDecoder->cs)/(audioDecoder->cs.fileSize/100);
      hadAnythingToDo = 1;
      lastSeconds = newSeconds;
      printf("~%04x=%ld\n", UIMSG_U32_PLAY_TIME_SECONDS, lastSeconds);
      if (newPerCent != lastPerCent) {
	lastPerCent = newPerCent;
	printf("~%04x=%d\n", UIMSG_U32_PLAY_FILE_PERCENT, newPerCent);
      }
    }

    if (appFlags & APP_FLAG_QUIT) {
      audioDecoder->cs.cancel = 1;
      audioDecoder->pause = 0;
      running = 0;
    }

    if (ioctl(stdin, IOCTL_TEST, NULL) > 0) {
      char c = fgetc(stdin);
      int targetOffset = 0;

      hadAnythingToDo = 1;

      if (cmdLen < 0) {
	switch (c) {
	case 'q':
	  audioDecoder->cs.cancel = 1;
	  audioDecoder->pause = 0;
	  running = 0;
	  break;
	case 'f':
	  speedShift *= 1.005;
	  adjustSpeed = 1;
	  break;
	case 's':
	  speedShift *= 0.995;
	  adjustSpeed = 1;
	  break;
	case 'b':
	  speedShift = 1.0;
	  adjustSpeed = 1;
	  break;
	case '+':
	  pitchShift *= 1.005;
	  adjustPitch = 1;
	  break;
	case '-':
	  pitchShift *= 0.995;
	  adjustPitch = 1;
	  break;
	case '=':
	  pitchShift = 1.0;
	  adjustPitch = 1;
	  break;
	case '>':
	case '<':
	  {
	    s_int16 t = ioctl(stdaudioout, IOCTL_AUDIO_GET_VOLUME, NULL)-256;
	    if (t >= -256) {
	      if (c == '>') {
		if (t > 0) {
		  t--;
		}
	      } else {
		if (t < 100) {
		  t++;
		}
	      }
	      ioctl(stdaudioout, IOCTL_AUDIO_SET_VOLUME, (void *)(t+256));
	      printf("~%04x=%d\n", UIMSG_S16_SET_VOLUME, t);
	    }
	  }

	  break;
	case 'p':
	  fileNum -= 2;
	  /* Intentional fall-through */
	case 'n':
	  audioDecoder->cs.cancel = 1;
	  audioDecoder->pause = 0;
	  break;
	case ' ':
	  pause = !pause;
	  audioDecoder->pause = pause && !audioDecoder->cs.cancel;
	  printf("~%04x=%d\n", UIMSG_BUT_PAUSE, pause);
	  break;
	case '.':
	  targetOffset = 10;
	  break;
	case ':':
	  targetOffset = 60;
	  break;
	case ',':
	  targetOffset = -10;
	  break;
	case ';':
	  targetOffset = -60;
	  break;
	case '~':
	  cmdLen = 0;
	  cmd[0] = '\0';
	  if (appFlags & APP_FLAG_ECHO) {
	    putchar('~');
	  }
	  break;
	} /* switch(c) */
	if (c >= '0' && c <= '9') {
	  playSpeed = c-'0';
	  printf("~%04x=%d\n", UIMSG_BOOL_FAST_FORWARD, playSpeed);
	  audioDecoder->cs.fastForward = playSpeed;
	}
      } else { /* cmdLen >= 0 */
	if (c==8 || c==0x7f) {
	  if (appFlags & APP_FLAG_ECHO) {
	    printf("%c[1D",27);
	    printf("%c[0K",27);
	  }
	  if (--cmdLen >= 0) {
	    cmd[cmdLen] = '\0';
	  }
	} else {
	  if (appFlags & APP_FLAG_ECHO) {
	    putchar(c);
	  }
	  if (c != '\n') {
	    cmd[cmdLen] = c;
	    if (cmdLen < MAX_CMD_LEN-1) {
	      cmdLen++;
	    }
	    cmd[cmdLen] = '\0';
	  } else { /* c == '\n' */
	    int i;
	    if (cmdLen < 6 || (cmd[4] != '=' && cmd[4] != '\'')) {
	      printf("!cmdLine\n");
	    } else {
	      u_int16 message = strtol(cmd+0, NULL, 16);
	      s_int32 val = strtol(cmd+5, NULL, 0);
	      if (message == UIMSG_S16_SET_VOLUME) {
		ioctl(stdaudioout, IOCTL_AUDIO_SET_VOLUME, (void *)(val+256));
		printf("~%04x=%d\n", UIMSG_S16_SET_VOLUME, val);
	      } else if (message == UIMSG_U32_PLAY_TIME_SECONDS) {
		newSeconds = 0;
		targetOffset = val;
	      } else if (message == UIMSG_SELECT_TRACK) {
		if (shuffle) {
		  shuffle = 0;
		  printf("~%04x=%d\n", UIMSG_BOOL_SHUFFLE, 0);
		}
		MakeShuffleList();
		fileNum = val-2;
		audioDecoder->cs.cancel = 1;
		audioDecoder->pause = 0;
	      } else if (message == UIMSG_BUT_PAUSE) {
		pause = val;
		audioDecoder->pause = pause && !audioDecoder->cs.cancel;
		printf("~%04x=%d\n", UIMSG_BUT_PAUSE, pause);
	      } else if (message == UIMSG_BOOL_SHUFFLE) {
		shuffle = val;
		if (val) {
		  fileNum = -1;
		} else {
		  fileNum = idxShuffle[fileNum];
		}
		printf("~%04x=%d\n", UIMSG_BOOL_SHUFFLE, shuffle);
		MakeShuffleList();
	      } else {
		printf("!cmdUnknown\n");
	      }
	    } /* c == '\n' */
	    cmdLen = -1;
	  }
	}
      }

      if (targetOffset && newSeconds >= 0) {
	if (newSeconds+targetOffset < 0) {
	  audioDecoder->cs.goTo = 0;
	} else {
	  audioDecoder->cs.goTo = newSeconds + targetOffset;
	}
      }

      if (adjustSpeed) {
	adjustSpeed = 0;
	if (speedShift > 0.998 && speedShift < 1.002) {
	  speedShift = 1.000;
	}
	if (ioctl(stdaudioout, IOCTL_AUDIO_SET_SPEED,
		  (void *)((u_int16)(speedShift*16384.0+0.5)))) {
	  printf("E: No speed shifter driver found\n");
	}
	printf("~%04x=%d\n", UIMSG_S16_SET_SPEED,
	       ioctl(stdaudioout, IOCTL_AUDIO_GET_SPEED, NULL));
      }

      if (adjustPitch) {
	adjustPitch = 0;
	if (pitchShift > 0.998 && pitchShift < 1.002) {
	  pitchShift = 1.000;
	}
	if (ioctl(stdaudioout, IOCTL_AUDIO_SET_PITCH,
		  (void *)((u_int16)(pitchShift*16384.0+0.5)))) {
	  printf("E: No pitch shifter driver found\n");
	}
	printf("~%04x=%d\n", UIMSG_S16_SET_PITCH,
	       ioctl(stdaudioout, IOCTL_AUDIO_GET_PITCH, NULL));
      }

    } /* if (ioctl(stdin, IOCTL_TEST, NULL) > 0) */

    if (!hadAnythingToDo) {
      Delay(20);
    }
  } /* while decoder task running */

	
#if 0
  printf("Decode returned %d, %s.\n", eCode, errorString);
#endif

  DeleteAudioDecoder(decoderLibrary, audioDecoder);
  return S_OK;
}


ioresult main(char *parameters) {
  int nParam, i;
  FILE *fp = NULL;
  static char fnumMode[10];
  char *p = parameters;
  int verbose = 0;
  int retCode = S_ERROR;

  nParam = RunProgram("ParamSpl", parameters);
  for (i=0; i<nParam; i++) {
    if (!strcmp(p, "-h")) {
      printf("Usage: PlayDir [-v|+v|-p|+p|-s|+s|-h]\n"
	     "-v\tVerbose\n"
	     "+v\tNot verbose\n"
	     "-p\tStart in pause mode\n"
	     "+p\tStart in play mode\n"
	     "-s\tShuffle mode on\n"
	     "+s\tShuffle mode off\n"
	     "-h\tShow this help\n");
      goto finally;
    } else if (!strcmp(p, "-v")) {
      verbose = 1;
    } else if (!strcmp(p, "+v")) {
      verbose = 0;
    } else if (!strcmp(p, "-p")) {
      pause = 1;
    } else if (!strcmp(p, "+p")) {
      pause = 0;
    } else if (!strcmp(p, "-s")) {
      shuffle = 1;
    } else if (!strcmp(p, "+s")) {
      shuffle = 0;
    } else {
      printf("Unknown parameter \"%s\", ignoring\n", p);
    }
    p += strlen(p)+1;
  }

  /* Allocate space for index buffer */
  idx = callocy(sizeof(*idx), IDX_SIZE);
  if (!idx) {
    printf("!outOfMem\n");
    goto finally;
  }
  *idx = IDX_SIZE;
  files = RunLibraryFunction("DIR", ENTRY_1, (int)idx);
  if (files) {
    /* Minimize index buffer smaller, the make a shuffle list for it. */
    idx = reallocy(idx, sizeof(*idx)*files);
    idxShuffle = mallocy(sizeof(*idx)*files);
    if (!idxShuffle) {
      printf("!outOfMem\n");
      goto finally;
    }
    MakeShuffleList();
  }

  decoderLibrary = LoadLibrary("audiodec");
  if (!decoderLibrary) {
    printf("!decLib\n");
    goto finally;
  }

  printf("~%04x'%d\n",
	 UIMSG_S16_SONG_NUMBER, files);

  if (verbose) {
    for (fileNum = 0; fileNum < files; fileNum++) {
      sprintf(fnumMode,"rb#%d", idx[fileNum]-1);
      fp = fopen("*",fnumMode);
      if (fp) {
	printf("~%04x'%d\n",
	       UIMSG_S16_SONG_NUMBER, fileNum+1);
	printf("~%04x'%s\n",
	       UIMSG_TEXT_LONG_FILE_NAME, fp->Identify(fp, NULL, 0));
	RunLibraryFunction("METADATA", ENTRY_2, (int)fp);
	fclose(fp);
      }
    }
  }
  fileNum = 0;

  while (running) {
		
    if (appFlags & APP_FLAG_QUIT) {
      break;
    }
		
#if 0
    printf("Open file %d...\n", fileNum+1);
#endif

    sprintf(fnumMode,"rb#%d", idx[idxShuffle[fileNum]]-1);
    fp = fopen("*",fnumMode); /* e.g. fopen("N:MUSIC\*","rb#123"); */
    if (fp) {
      int playErr;
      printf("~%04x=%d\n",
	     UIMSG_SELECT_TRACK, fileNum+1);
      printf("~%04x=%d\n",
	     UIMSG_S16_SONG_NUMBER, idxShuffle[fileNum]+1);
      printf("~%04x'%s\n",
	     UIMSG_TEXT_LONG_FILE_NAME, fp->Identify(fp, NULL, 0));
      RunLibraryFunction("METADATA", ENTRY_2, (int)fp);

      playErr = PlayFile(fp);
      fclose(fp);
      if (playErr) {
	goto finally;
      }
      if (++fileNum >= files) {
	printf(">\n");
	running = 0;
      } else if (fileNum < 0) {
	printf("<\n");
	running = 0;
      }
    } else {
      running = 0;
    }
  }

  printf("~%04x'%d\n", UIMSG_SELECT_TRACK, 0);

  retCode = S_OK;

 finally:
  if (decoderLibrary) {
    DropLibrary(decoderLibrary);
  }
  if (idxShuffle) {
    freey(idxShuffle);
  }
  if (idx) {
    freey(idx);
  }

  return retCode; /* End of files, ok exit */
}
