#include <vo_stdio.h>
#include <string.h>
#include <vo_fat.h>
#include <vsos.h>
#include <vsostasks.h>
#include <codecmpg.h>
#include <codec.h>
#include <timers.h>
#include <exec.h>
#include <swap.h>
#include <ctype.h>
#include <vs1005h.h>
#include <uimessages.h>
#include <devAudio.h>
#include <apploader.h>
#include <clockspeed.h>
#include <time.h>

#include "fmMessages.h"
#include "fmModel.h"
#include "rds.h"
#include "fm_init.h"
#include "fm_function.h"
#include "audioInit.h"

#define IQ_AVERAGE 32 // iq averaging rounds per result value

#if 0
#define DEBUG_PRINT
#endif

extern TIMESTRUCT currentTime;

/* Function pointer to caller callback function. */
void (*fmCallbackFunction)(s_int16 index, u_int16 message, u_int32 value)
  = NULL;
  


/* List of user interface message items this model can handle. */
UiMessageListItem __mem_y fmUiMessageListItem[] = {
	{UIMSG_HINT,"LAY=1/DIV=40"},//Suggestion of screen layout strategy
	{UIMSG_VSOS_SET_CALLBACK_FUNCTION,"Set callback"},
	{UIMSG_VSOS_GET_MSG_LIST,"Get MSG list"},

	// Model's input user interface 
	// (the messages for controlling the operation of the model 
	// which are relevant for automatic generation of the user interface)
	//{UIMSG_INPUTS, "FM Controls"}, //Start of input category
	{UIMSG_BUT_PREVIOUS, "Previous"},
	{UIMSG_BUT_NEXT, "Next"},
	{UIMSG_U16_SCAN_FM, "Scan"},
	{UIMSG_U32_SET_FM_FREQ," "},
	
	// from Model
	{UIMSG_U16_FM_SCAN_RES, " "}, // value: pointer to array of u_int32 frequencies
	{UIMSG_U32_FM_FREQ, " "}, // value: current fm frequency in kHz
	
	// Model's output user interface 
	// (those status messages which the model generates,
	// which are relevant for automatic generation of the user interface)
	{UIMSG_OUTPUTS, "Info"}, //Start of output category "Info"
	{UIMSG_TEXT,""},
	
	{UIMSG_VSOS_END_OF_LIST,NULL}
};


// internal commands
u_int16 shouldSearchNext = 0;
u_int16 shouldSearchPrev = 0;
u_int16 shouldScanFm = 0;
u_int32 shouldSetCh = 0;

// Flag for ending phase compensation algorithm
u_int16 endPhComp = 0;

#define TP_MESSAGE "Traffic announcement"

// This callback function gets called by the View / Controller part (UI)
#define OP_IDLE  0
#define OP_PREV  1
#define OP_NEXT  2
#define OP_INFO  3
#define OP_SCAN  4
#define OP_SETFM 5
#define OP_TERMINATE 6
#define OP_TUNE 7

u_int16 command = OP_IDLE;
u_int32 parameter = 0;


void SetCallbackFunction(register u_int32 value) {
	fmCallbackFunction = (void *)value;
}

void SimplePrev(void) {
	command = OP_PREV;
	parameter = 0;
}

void SimpleNext(void) {
	command = OP_NEXT;
	parameter = 0;
}

void SimpleTune(void) {
	command = OP_TUNE;
	parameter = 0;
}

void MenuScan(void) {
	command = OP_SCAN;
	parameter = SEARCH_CH;
}

void SetFrequency(register u_int32 freq) {
	command = OP_SETFM;
	parameter = freq;
}

void ToggleMS(void) {
	PERIP(FM_CF) ^= FM_CF_MONO;
}

void SetMono(void) {
#ifdef DEBUG_PRINT
	printf("Set to Mono\n");
#endif
	PERIP(FM_CF) |= FM_CF_MONO;
}
	
void SetStereo(void) {
#ifdef DEBUG_PRINT
	printf("Set to Stereo\n");
#endif
	PERIP(FM_CF) &= ~FM_CF_MONO;
}


void PleaseDie(void) {
	command = OP_TERMINATE;
	parameter = 0;
}

/*
void MenuGain(regz StdWidget* w,regz s_int16 x,regz s_int16 y,regz EventType e) {
	static char gMsg[10];
	if ((PERIP(ANA_CF3) & ANA_CF3_GAIN1MASK) == ANA_CF3_GAIN1_20DB) {
		// max->min gain
		PERIP(ANA_CF3) &= ~ANA_CF3_GAIN1MASK;
		PERIP(ANA_CF3) |= ANA_CF3_GAIN1_11DB;
		PERIP(ANA_CF3) &= ~ANA_CF3_GAIN2MASK;
		PERIP(ANA_CF3) |= ANA_CF3_GAIN2_11DB;
		
		strcpy(gMsg,"Gain 11dB");
		fmCallbackFunction(-1,UIMSG_TEXT_RDS_RADIOTEXT,(u_int32)gMsg);
		printf("ADC1,2 gain is now 11dB (min)\n");
	}
	else if ((PERIP(ANA_CF3) & ANA_CF3_GAIN1MASK) == ANA_CF3_GAIN1_11DB) {
		// 
		PERIP(ANA_CF3) &= ~ANA_CF3_GAIN1MASK;
		PERIP(ANA_CF3) |= ANA_CF3_GAIN1_14DB;
		PERIP(ANA_CF3) &= ~ANA_CF3_GAIN2MASK;
		PERIP(ANA_CF3) |= ANA_CF3_GAIN2_14DB;
		
		strcpy(gMsg,"Gain 14dB");
		fmCallbackFunction(-1,UIMSG_TEXT_RDS_RADIOTEXT,(u_int32)gMsg);
		printf("ADC1,2 gain is now 14dB\n");
	}
	else if ((PERIP(ANA_CF3) & ANA_CF3_GAIN1MASK) == ANA_CF3_GAIN1_14DB) {
		// maximum gain
		PERIP(ANA_CF3) &= ~ANA_CF3_GAIN1MASK;
		PERIP(ANA_CF3) |= ANA_CF3_GAIN1_17DB;
		PERIP(ANA_CF3) &= ~ANA_CF3_GAIN2MASK;
		PERIP(ANA_CF3) |= ANA_CF3_GAIN2_17DB;
		
		strcpy(gMsg,"Gain 17dB");
		fmCallbackFunction(-1,UIMSG_TEXT_RDS_RADIOTEXT,(u_int32)gMsg);
		printf("ADC1,2 gain is now 17dB\n");
	}
	else if ((PERIP(ANA_CF3) & ANA_CF3_GAIN1MASK) == ANA_CF3_GAIN1_17DB) {
		// maximum gain
		PERIP(ANA_CF3) &= ~ANA_CF3_GAIN1MASK;
		PERIP(ANA_CF3) |= ANA_CF3_GAIN1_20DB;
		PERIP(ANA_CF3) &= ~ANA_CF3_GAIN2MASK;
		PERIP(ANA_CF3) |= ANA_CF3_GAIN2_20DB;
		
		strcpy(gMsg,"Gain 20dB");
		fmCallbackFunction(-1,UIMSG_TEXT_RDS_RADIOTEXT,(u_int32)gMsg);
		printf("ADC1,2 gain is now 20dB (max)\n");
	}
}
*/

/*
void FmModelCallback(s_int16 index, u_int16 message, u_int32 value) {
	
	switch (message) {
		case UIMSG_VSOS_SET_CALLBACK_FUNCTION:
			fmCallbackFunction = (void *)value;
			break;
		case UIMSG_VSOS_GET_MSG_LIST:
			if (fmCallbackFunction) {
				fmCallbackFunction(-1, UIMSG_VSOS_MSG_LIST, (u_int32)fmUiMessageListItem);
			}
			break;
		case UIMSG_BUT_NEXT:
			shouldSearchNext = 1;
			break;
		case UIMSG_BUT_PREVIOUS:
			shouldSearchPrev = 1;
			break;
		case UIMSG_U16_SCAN_FM:
			if (value < FMCHANS) {
				shouldScanFm = (u_int16)value;
			}
			else {
				shouldScanFm = FMCHANS;
			}
			break;
		case UIMSG_U32_SET_FM_FREQ:
			shouldSetCh = value;
			break;
	}
		
}
*/


struct fmStatus fmStatus;
#ifdef USE_RDS
struct rdsGroup rdsGroup;
struct rdsInfo rdsInfo;
#endif
struct eonSet eonSet[MAX_EON];

#define FM_AUDIO_BUFSIZE 128
s_int16 fmAudioBuf[FM_AUDIO_BUFSIZE];

void FmModelTask(void) {
  u_int16 i = 0;
  u_int16 sync = 0;
#ifdef USE_RDS
  u_int16 rdsBlockIdx = 0;
#endif
  u_int16 success;
  u_int16 result = 0;
  u_int16 rtCleared = 0;
  u_int16 tpRunning = 0;

  u_int16 rtPartial = 0;

  /* CPU may not run from the same clock as our RF receiver. */
  if (clockSpeed.masterClkSrc == MASTER_CLK_SRC_RF) {
    RunProgram("SETCLOCK", "60");
  }
	

  // Start audio
  InitAudio();
	
  InitFM();
  PERIP(FM_CF) |= FM_CF_MONO;
//	PERIP(FM_CF) &= FM_CF_MONO;// DEBUG ONLY
	

#ifdef USE_RDS
  InitFmStatus(&fmStatus);
  InitRds(&rdsInfo, eonSet);
	
  fmStatus.rdsScan = 1; // find channel names, but nothing more
  fmStatus.rdsEna = 1; // enable RDS reception/decoding
#endif

  // Set frequency for start
//	fmStatus.freq = 99900U;
//	SetFMFreq(fmStatus.freq);
//	RunIQComp();
#ifdef USE_RDS
  InitRds(&rdsInfo, eonSet); // clear
#endif
	
  Delay(100);
	
  // some adjustements for testing purposes, REMOVE when done
  //PERIP(ANA_CF3) &= ~0x03f0; // minimum gain
  
#if 0
  // minimum gain
  PERIP(ANA_CF3) = (PERIP(ANA_CF3) & ~(ANA_CF3_GAIN1MASK  | ANA_CF3_GAIN2MASK))
    | (ANA_CF3_GAIN1_11DB | ANA_CF3_GAIN2_11DB);
#else
  // maximum gain
  PERIP(ANA_CF3) = (PERIP(ANA_CF3) & ~(ANA_CF3_GAIN1MASK  | ANA_CF3_GAIN2MASK))
    | (ANA_CF3_GAIN1_20DB | ANA_CF3_GAIN2_20DB);
#endif

  if (!fmCallbackFunction) {
    goto finally;
  }

  fmCallbackFunction(-1, UIMSG_VSOS_MSG_LIST, (u_int32)fmUiMessageListItem);
	
  // status to GUI
  fmCallbackFunction(-1,UIMSG_U32_FM_FREQ, fmStatus.freq);
	
  Delay(100);
	
  while (1) {
    u_int32 targetFreq = 0;
    // Read stereo samples from stdaudioin into myBuf.
    // By default, stdaudioin comes from line in.
    // By default both input and output are 16-bit stereo at 48 kHz.
    fread(fmAudioBuf, sizeof(s_int16), FM_AUDIO_BUFSIZE, stdaudioin);
    
    // Here you may process audio which is in 16-bit L/R stereo format.
		
    // Write stereo samples from myBuf into stdaudioout.
    // By default, stdaudioout goes to line out.
    fwrite(fmAudioBuf, sizeof(s_int16), FM_AUDIO_BUFSIZE, stdaudioout);
    // look for Phase compensation end flag and finalize algortihm run
    if (endPhComp) {
      CloseIQComp();
    }
    
		
#ifdef USE_RDS
    if (fmStatus.rdsEna == 1) {
      while (!GetRds(&rdsGroup, &rdsInfo, eonSet, &fmStatus, &sync,
		     &rdsBlockIdx, &success));
    }
#endif
		
    if (command == OP_TUNE) {
      fmStatus.freq = FmTune(fmStatus.freq);
      fmCallbackFunction(-1,UIMSG_U32_FM_FREQ, fmStatus.freq);
      command = OP_IDLE;
    } else if (command == OP_NEXT || command == OP_PREV) {
      u_int32 newFreq = 0;
			
      CloseIQComp();
      newFreq = FmNext(fmStatus.freq, command == OP_PREV);
      if (!newFreq) {
	// nothing found, go back to where search started
	SetFMFreq(fmStatus.freq);
      } else {
	// Set new frequency
#if 1
	newFreq = FmTune(newFreq);
	fmStatus.freq = newFreq;
	SetFMFreq(fmStatus.freq);
#else
	fmStatus.freq = newFreq;
	SetFMFreq(fmStatus.freq);
#endif
				
#ifdef USE_RDS
	InitRds(&rdsInfo, eonSet); // clear
#endif
      }
      RunIQComp();
      fmCallbackFunction(-1,UIMSG_U32_FM_FREQ, fmStatus.freq);
      command = OP_IDLE;
    }
#if 0 // only in testmode
    else if (command == OP_INFO) {
      u_int16 idx = 0;
      // print status
      printf("\n");
      printf("pi:    0x%04x\n", rdsInfo.pi);
      printf("group: 0x%04x\n", rdsInfo.group);
      printf("b0:    0x%04x\n", rdsInfo.b0);
      printf("pty:   0x%04x ", rdsInfo.pty);
      printf(":\"%s\"\n",PTYTable[rdsInfo.pty]);
      printf("tp:    0x%04x\n", rdsInfo.tp);
      printf("ta:    0x%04x\n", rdsInfo.ta);
      printf("ms:    0x%04x\n", rdsInfo.ms);
      printf("di:    0x%04x\n", rdsInfo.di);
      
      if ((rdsInfo.di & 0x0001) == 1) {
	printf(" --> Stereo, ");
      }
      else {
	printf(" --> Mono, ");
      }
      if ((rdsInfo.di & 0x0002) == 2) {
	printf("Uses artificial head, ");
      }
      else {
	printf("No artificial head, ");
      }
      if ((rdsInfo.di & 0x0004) == 4) {
	printf("Compressed, ");
      }
      else {
	printf("Not compressed, ");
      }
      if ((rdsInfo.di & 0x0008) == 8) {
	printf("Dynamic PTY\n");
      }
      else {
	printf("Static PTY\n");
      }
      printf("Radio Paging Codes:    0x%04x\n", rdsInfo.radioPagingCodes);
      printf("Slow Labelling Codes:  0x%04x\n", rdsInfo.slowLabellingCodes);
      printf("ProgItemNum:0x%04x ", rdsInfo.programmeItemNumber);
      printf("day %u, ", rdsInfo.programmeItemNumber>>11);
      printf("%02u:%02u\n", ((rdsInfo.programmeItemNumber>>6) & 0x001f),(rdsInfo.programmeItemNumber & 0x003f));
      
      printf ("Alternative Frequencies: Currently got %u pairs Method %u\n",rdsInfo.afIdx, rdsInfo.afMethod);
#if 0
      for (idx = 0; idx < rdsInfo.afIdx; idx++) {
	printf("0x%04x ", rdsInfo.alternativeFreq[idx]);
	
	if ((rdsInfo.alternativeFreq[idx] >> 8) != 0
	    && (rdsInfo.alternativeFreq[idx] >> 8) < 205) {
	  // upper
	  printf("(%u) ", 875 + (rdsInfo.alternativeFreq[idx] >> 8));
	}
	else if ((rdsInfo.alternativeFreq[idx] >> 8) > 224
		 && (rdsInfo.alternativeFreq[idx] >> 8) < 250) {
	  // number of freqs
	  printf("(#%u) ", ((rdsInfo.alternativeFreq[idx] >> 8) -224));
	}
	
	if ((rdsInfo.alternativeFreq[idx] & 0x00ff) != 0
	    && (rdsInfo.alternativeFreq[idx] & 0x00ff) < 205) {
	  //lower
	  printf("(%u) ", 875 + (rdsInfo.alternativeFreq[idx] & 0x00ff));
	}
	else if ((rdsInfo.alternativeFreq[idx] & 0x00ff) == 0xcd) {
	  printf("(-) ");
	}
	
	
	if (idx % 4 == 3) {
	  printf("\n");
	}
      }
      printf("\n");
#endif
#if 1
#ifdef USE_RDS
      printf ("EON pi(on):  ");
      for (idx = 0; idx < rdsInfo.eonPiFound; idx++) {
	printf ("0x%04x, ", eonSet[idx].piOn);
      }
      printf("\n");
#endif
#endif
      
      
      command = OP_IDLE;
    }
#endif
    else if (command == OP_SETFM) {
      u_int32 target = parameter;
      // Set channel to frequency
      if (FM_LOW <= target && target <= FM_HIGH) {
	fmStatus.freq = target;
	SetFMFreq(fmStatus.freq);
#ifdef USE_RDS
	InitRds(&rdsInfo, eonSet); // clear
#endif
	RunIQComp();
      }
			
      fmCallbackFunction(-1,UIMSG_U32_FM_FREQ, fmStatus.freq);
			
      parameter = 0;
      command = OP_IDLE;
    }
    else if (command == OP_TERMINATE) {
      // Cleanup and return
      // disable RDS and FM
#ifdef USE_RDS
#if 0
      PERIP(INT_ENABLE1_HP) &= ~(INTF1_RDS|INTF1_FM);
      PERIP(INT_ENABLE1_LP) &= ~(INTF1_RDS|INTF1_FM);
      PERIP(FM_CF) &= ~FM_CF_RDSENA;
#else
      PERIP(INT_ENABLE0_LP) &= ~INTF_MAC0;
      PERIP(INT_ENABLE0_HP) &= ~INTF_MAC0;
      PERIP(INT_ENABLE1_LP) &= ~INTF1_FM;
      PERIP(INT_ENABLE1_HP) &= ~INTF1_FM;
#endif
#endif			
#if 0
      PERIP(FM_CF) &= ~FM_CF_FM_ENA;
#endif
      return;
    }
		
// Information ready:

#if 1
#ifdef USE_RDS
    // Partial Radiotext messages
    if (rdsInfo.radioTextValid < 0x0000ffff && ((rdsInfo.radioTextValid & 0x0000000f) == 0x0000000f) && rtPartial == 0) {
      // first 8/16 symbols
      // Note: clearing readiotext message is separate event
      static char rtMsg8[18];
      RdsParseText(rtMsg8, rdsInfo.radioText, 8);
      
      fmCallbackFunction(-1,UIMSG_TEXT_RDS_RADIOTEXT,(u_int32)rtMsg8);
      rtCleared = 0;
      rtPartial = 8;
#ifdef DEBUG_PRINT
      printf("put RT8: %s\n", rtMsg8);
#endif
    }
    if (rdsInfo.radioTextValid < 0x0000ffff && ((rdsInfo.radioTextValid & 0x000000ff) == 0x000000ff) && rtPartial < 16) {
      // first 16/32 symbols
      // Note: clearing radiotext message is separate event
      static char rtMsg16[34];
      RdsParseText(rtMsg16, rdsInfo.radioText, 16);
      
      fmCallbackFunction(-1,UIMSG_TEXT_RDS_RADIOTEXT,(u_int32)rtMsg16);
      rtCleared = 0;
      rtPartial = 16;
#ifdef DEBUG_PRINT
      printf("put RT16: %s\n", rtMsg16);
#endif
    }
#endif /* USE_RDS */
#endif
    
#ifdef USE_RDS		
    if (rdsInfo.progNameValid == 0x00ff) {
      // New Programme Service name is available
      static char PSNameMsg[9];
      RdsParseText(PSNameMsg, rdsInfo.progName, 4);
      fmCallbackFunction(-1,UIMSG_TEXT_RDS_PROGRAMME_SERVICE,(u_int32)PSNameMsg);
      rdsInfo.progNameValid = 0xffff;
    }
    
    if (rdsInfo.radioTextValid == 0x0000ffff) {
      // radiotest message is completed
      // Note: clearing readiotext message is separate event
      static char rtMsg[65];
      RdsParseText(rtMsg, rdsInfo.radioText, 32);
      
      fmCallbackFunction(-1,UIMSG_TEXT_RDS_RADIOTEXT,(u_int32)rtMsg);
      rtCleared = 0;
      rdsInfo.radioTextValid = 0xffffffff;
    }
    if ((rdsInfo.radioTextValid == 0x0 && rtCleared == 0) || (tpRunning == 1 && fmStatus.tpActive == 0)) {
      static char rtClearMsg[5];
      strcpy(rtClearMsg, "  ");
      
      fmCallbackFunction(-1,UIMSG_TEXT_RDS_RADIOTEXT,(u_int32)rtClearMsg);
      rtCleared = 1;
      rtPartial = 0;
      
      if (tpRunning == 1) {
	rdsInfo.radioTextValid = 0; // make sure new rt is shown when ta ends
	tpRunning = 0;
      }
    }
    if (tpRunning == 0 && fmStatus.tpActive == 1) {
      // Notify UI about active Traffic announcement using RT-message
#ifdef DEBUG_PRINT
      printf("Note: Put TA-note\n");
#endif
      tpRunning = 1;			
      fmCallbackFunction(-1,UIMSG_TEXT_RDS_RADIOTEXT,(u_int32)TP_MESSAGE);
      rtCleared = 0;
    }

    if (rdsInfo.timeValid == 1) {
      // new time available
      ParseTime(&rdsInfo);
      
#if 0
      printf("Date (dd.mm.yyyy): %02u.%02u.%u\n",
	     currentTime.tm_mday, currentTime.tm_mon+1,
	     currentTime.tm_year+1900);
      printf("Time is %02u.%02u\n", currentTime.tm_hour, currentTime.tm_min);
#endif
      
      rdsInfo.timeValid = 0xffff;
    }
    
    if (rdsInfo.ptynValid == 0x00ff) {
      // new Programme type name available
      static char ptynMsg[9];
      RdsParseText(ptynMsg, rdsInfo.ptyn, 4);
#ifdef DEBUG_PRINT
      printf("PTY: %s\n", PTYTable[rdsInfo.pty]);
      printf("PTYN: %s\n", ptynMsg);
#endif
      
      rdsInfo.ptynValid = 0xffff;
    }
#endif /* USE_RDS */
    
  } // end of main while loop
 finally:
  PERIP(0);
}



void ParseTime(register struct rdsInfo *rdsInfo) {
  u_int32 tp;
  s_int16 offsetAdjust = 1800*(rdsInfo->timeOffset & 0x001f);

  if (rdsInfo->timeOffset & 0x20) {
    offsetAdjust = -offsetAdjust;
  }
	
  /* -51544 comes from epoch difference between RDS and VSOS */
  tp = (rdsInfo->dayCode-51544) * (24*3600) +
    (rdsInfo->hour*3600 + rdsInfo->minute*60) + offsetAdjust;
  memcpy(&currentTime, localtime(&tp), sizeof(currentTime));
}
