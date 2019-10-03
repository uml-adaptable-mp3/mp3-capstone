/// VSOS APPLICATION FOR VS1005G DEVELOPER BOARD

/// FM-radio Example Application for VS1005G
/// NOTE! THIS IS A WORK-IN-PROGESS, NOT A READY-MADE EXAMPLE!!!

/// Compiling this software creates an .AP3 file, copy it to the
/// VS1005 VSOS boot device main folder.

/// Please visit www.vsdsp-forum.com for free support and discussion!
#include <vo_stdio.h>
#include <vstypes.h>
#include <string.h>
#include <vo_gpio.h>
#include <sysmemory.h>
#include <vsostasks.h>
#include <timers.h>
#include <audio.h>
#include <uimessages.h>
#include <lcd.h>
#include <apploader.h>
#include <consolestate.h>
#include <ctype.h>
#include <aucommon.h>
#include <kernel.h>

#include "fmMessages.h" // to be integrated into uimessages.h
#include "fmModel.h" // The FM-radio Model
#include "fmmain.h"
#include "fm_function.h"



#if 1
#define USE_DEVBOARD_BUTTONS
#endif

#if 1
#define USE_STDIN
#endif



#define MAX_CHANNEL 16


#define EMPTY_CH "   --   "

RADIOCHANNEL newChannel={0,0}, currentChannel={0,0};

char radioText[65] = "-";
char radioTextCandidate[65] = "";

static s_int16 numChannels=0;
static u_int16 deleteMode=0;



u_int32 chFreqs[SEARCH_CH] = {0};


/// This is the receiver for UIMessages from the FM-radio model.
void FmModelReceiver(s_int16 index, u_int16 message, u_int32 value) {
	static UiMessageListItem __mem_y *msgList = NULL;
	char *title = NULL;
	u_int16 idx = 0;
	u_int32 *freqs = NULL;

	Forbid();
	
	switch (message) {
		case UIMSG_VSOS_MSG_LIST: 
		msgList = (void*)value;
		break;
		
		case UIMSG_U32_FM_FREQ:
		// notification of new frequency
		newChannel.frequency = value;
		newChannel.name[0] = 0;
		break;
		
		case UIMSG_U16_FM_SCAN_RES:
		// notification of scan results
		freqs = (u_int32 *)value;
		for (idx = 0; idx < SEARCH_CH; idx++) {
			chFreqs[idx] = (u_int32)freqs[idx];
		}
		break;

		case UIMSG_TEXT_RDS_PROGRAMME_SERVICE:
		// notification of new channel name
		strcpy(newChannel.name,(char*)value);
		break;
		
		case UIMSG_TEXT_RDS_RADIOTEXT:
		// notification of new radiotext
		strcpy(radioTextCandidate,(char*)value);
		break;
		
		case UIMSG_TEXT:
		// Statustext
		//printf("Note: %s\n",(char*)value);
		break;

		default:
		break;

	}
	
	Permit();	
	return;
}



void AdjustVolume(register s_int16 x) {
  s_int16 t = ioctl(stdaudioout, IOCTL_AUDIO_GET_VOLUME, NULL);
  if (t >= 0) {
    t = t-256-x;
    if (t < 0) {
      t = 0;
    } else if (t > 100) {
      t = 100;
    }
    ioctl(stdaudioout, IOCTL_AUDIO_SET_VOLUME, (void *)(t+256));
    printf("Vol = -%d dB\n", t/2);
  }
}


u_int16 dump = 0, rdsDebug = 0;
extern u_int32 fmBoundary[2];

ioresult main(char *params) {
  u_int32 oldTimeCount = ReadTimeCount();
  int appRunning = -1, scanMode = 0;
  u_int32 initFreq = 93700;

  printf("Radio\n");

  if (params) {
    if (isdigit(*params)) {
      initFreq = strtol(params, NULL, 0);
    } else if (*params == 's') {
      scanMode = 1;
      iqCompDisable = 1;
    }
  }

  // Inform the radio model that we are listening to
  // messages in function "FmModelReceiver"
  SetCallbackFunction((u_int32)FmModelReceiver);
	
  // Start the fm radio task to run in the background
  StartTask(TASK_DECODER, FmModelTask); //in fmModel.c
  Yield(); // Release the CPU to allow the radio task to initialize
  Delay(10);

  // Load channelsetup and set initial channel
  SetFrequency(FM_LOW);
  SetFrequency(initFreq);
  currentChannel.frequency = initFreq;
  SetMono();

  if (scanMode) {
    u_int32 currFreq = FM_LOW-1*FM_STEP;
    Delay(2*TICKS_PER_SEC);
    while (currFreq <= FM_HIGH+1*FM_STEP) {
      SetFrequency(currFreq);
      Delay(50);
      PERIP(ANA_CF3) =
	(PERIP(ANA_CF3) & ~(ANA_CF3_GAIN1MASK | ANA_CF3_GAIN2MASK)) |
	(ANA_CF3_GAIN1_11DB | ANA_CF3_GAIN2_11DB);
      Delay(1);
      printf("f %6ld IQ %3d ",
	     currFreq, FmCalcIQ());

      PERIP(ANA_CF3) =
	(PERIP(ANA_CF3) & ~(ANA_CF3_GAIN1MASK | ANA_CF3_GAIN2MASK)) |
	(ANA_CF3_GAIN1_20DB | ANA_CF3_GAIN2_20DB);
      Delay(1);
      printf("%3d Lock %d\n",
	     FmCalcIQ(), CheckRFLck(0));
      currFreq += FM_STEP;
    }
    goto finally;
  }

#ifdef USE_DEVBOARD_BUTTONS
  /* Make DevBoard S1-S4 buttons GPIO inputs */
  PERIP(GPIO0_MODE) &= ~0xF;
  PERIP(GPIO0_DDR) &= ~0xF;
#endif

  // Goes to the next channel that can be found once every 10 seconds.
  while (appRunning) {
    int i, c = 0;
    static u_int32 oldFreq = 0;
    
    if (appRunning < 0) {
      /* Make sure we run "next channel" operation every time we start. */
      //c = 'n';
      appRunning = 1;
    }
#ifdef USE_STDIN
    if (!c && ioctl(stdin, IOCTL_TEST, NULL) > 0) {
      c = fgetc(stdin);
    }
#endif
#ifdef USE_DEVBOARD_BUTTONS
    if (!c) {
      static u_int16 oldDevBoardButtons = 0;
      u_int16 buttons = PERIP(GPIO0_IDATA) & 0xF;
      u_int16 newButtons = buttons & ~oldDevBoardButtons;
      oldDevBoardButtons = buttons;
      if (newButtons & 1) {
	/* S1 = Previous channel */
	c = -'p';
      }
      if (newButtons & 2) {
	/* S2 = Next channel */
	c = -'n';
      }
      if (buttons & 4) {
	/* S3 = Volume down */
	c = -'<';
      }
      if (buttons & 8) {
	/* S4 = Volume up */
	c = -'>';
      }
    }
#endif
    switch (abs(c)) {
    case '?':
      printf("Frequency %7.2f MHz, ", currentChannel.frequency*0.001);
      printf("Signal level %2.1f dB, ", 0.1*FmCalcIQ());
      printf("ANA_CF3 0x%04x\n", PERIP(ANA_CF3));
      break;
    case '.':
      currentChannel.frequency += 10;
      SetFrequency(currentChannel.frequency);
      break;
    case ',':
      currentChannel.frequency -= 10;
      SetFrequency(currentChannel.frequency);
      break;
    case ':':
      currentChannel.frequency += 100;
      SetFrequency(currentChannel.frequency);
      break;
    case ';':
      currentChannel.frequency -= 100;
      SetFrequency(currentChannel.frequency);
      break;
    case 'n':
      printf("\nNext channel\n");
      memset(&currentChannel, 0, sizeof(currentChannel));
      SimpleNext();
      break;
    case 'p':
      printf("\nPrevious channel\n");
      memset(&currentChannel, 0, sizeof(currentChannel));
      SimplePrev();
      break;
    case 'T':
      printf("T %5.3f s\n", 0.001*ReadTimeCount());
      break;
    case 't':
      SimpleTune();
      break;
    case '1':
      printf("#1\n");
      fmBoundary[1] = 103000;
      currentChannel.frequency = 104700;
      SetFrequency(100000);
      Delay(1000);
      SetFrequency(currentChannel.frequency);
      break;
    case '2':
      printf("#2\n");
      fmBoundary[1] = 105000;
      SetFrequency(100000);
      Delay(1000);
      currentChannel.frequency = 104700;
      SetFrequency(currentChannel.frequency);
      break;
    case 'D':
      dump = 1;
      break;
    case 'd':
      rdsDebug = !rdsDebug;
      break;
    case '<':
      AdjustVolume(-2);
      break;
    case '>':
      AdjustVolume(+2);
      break;
    }

    if (oldFreq && newChannel.frequency == oldFreq) {
      currentChannel.frequency = newChannel.frequency;
      printf("Frequency: %7.2f MHz\n", currentChannel.frequency*0.001);
      newChannel.frequency = 0;
    }
    /* If the channel has a new name */
    if (newChannel.name[0]) {
      /* Compare whether it changed from the old one. */
      if (strcmp(newChannel.name, currentChannel.name)) {
	/* If so, print it. */
	strcpy(currentChannel.name, newChannel.name);
	printf("Channel name: %s\n", currentChannel.name);
      }
      newChannel.name[0] = '\0';
    }
    /* If there may be new radio text... */
    if (radioTextCandidate[0]) {
      int newRadioText = 0;
      Forbid();
      /* ... then check whether it has changed from the old one. */
      if (strcmp(radioText, radioTextCandidate)) {
	/* If yes, copy it (and later print) */
	strcpy(radioText, radioTextCandidate);
	newRadioText = 1;
      }
      Permit();
      /* Clear radio text candidate */
      radioTextCandidate[0] = '\0';
      /* If changed and not "  ", print radio text */
      if (newRadioText && strcmp(radioText, "  ")) {
	printf("RadioText: \"%s\"\n", radioText);
      }
    }

    oldFreq = newChannel.frequency;
    if (c <= 0) {
      Delay(TICKS_PER_SEC/20);
    }
    if (appFlags & APP_FLAG_QUIT) {
      appRunning = 0;
    }
    oldTimeCount += 10L*TICKS_PER_SEC;
  }

 finally:
  printf("\nClosing... ");
  PleaseDie();

  // Wait for all threads to finish before returning
  while(pSysTasks[TASK_DECODER].task.tc_State && pSysTasks[TASK_DECODER].task.tc_State != TS_REMOVED) {
    Delay(1);
  }

  printf("ok.\n");
  return S_OK;
}
