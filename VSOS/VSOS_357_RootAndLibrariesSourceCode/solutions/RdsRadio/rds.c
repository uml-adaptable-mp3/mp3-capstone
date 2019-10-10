//
//
// Functions for RDS
//
//

#include <vo_stdio.h>
#include <aucommon.h>
#include <vs1005h.h>
#include <consolestate.h>
#include <ctype.h>
#include <imem.h>
#include"rds.h"
#include "fmmain.h"

#ifdef USE_RDS

#define ON_TA (1<<3)
#define ON_TP (1<<4)


#define DBG 1
#if 0
// 8 Character display
const char* const PTYTable[] = {
	" ",
	"News",
	"Affairs",
	"Info",
	"Sport",
	"Educate",
	"Drama",
	"Culture",
	"Science",
	"Varied",
	"Pop M",
	"Rock M",
	"Easy M",
	"Light M",
	"Classics",
	"Other M",
	"Weather",
	"Finance",
	"Children",
	"Social",
	"Religion",
	"Phone In",
	"Travel",
	"Leisure",
	"Jazz",
	"Country",
	"Nation M",
	"Oldies",
	"Folk M",
	"Document",
	"TEST",
	"Alarm !"
};

#else
// 16 Character display
const char* const PTYTable[] = {
	" ",
	"News",
	"Current Affairs",
	"Information",
	"Sport",
	"Education",
	"Drama",
	"Cultures",
	"Science",
	"Varied Speech",
	"Pop Music",
	"Rock Music",
	"Easy Listening",
	"Light Classics M",
	"Serious Classics",
	"Other Music",
	"Weather & Metr",
	"Finance",
	"Childrens Progs",
	"Social Affairs",
	"Religion",
	"Phone In",
	"Travel & Touring",
	"Leisure & Hobby",
	"Jazz Music",
	"Country Music",
	"National Music",
	"Oldies Music",
	"Folk Music",
	"Documentary",
	"Alarm Test",
	"Alarm - Alarm !"
};
#endif

// return 1 if TP-visit is to be terminated, 0 otherwise
u_int16 RdsEndTP(register struct rdsGroup *rdsGroup) {
  // Group 0
  if ((rdsGroup->data[1] >> 12) == 0) {
    // just keep an eye for TP and TA
    if (((rdsGroup->data[1] >> 10) & 0x0001) == 0
	|| ((rdsGroup->data[1] >> 4) & 0x0001) == 0) {
      return 1;
    }
  }
}

// See if radiotext message is complete and mark it such
void RdsCheckRT(register struct rdsInfo *rdsInfo) {
  // See if radiotext message is complete:
  if (rdsInfo->radioTextValid > 0 && rdsInfo->radioTextValid < 0x0000ffff) {
    u_int16 idx = 0;
    // Complete if message is filled from start to CR(0x0d) mark (or completetly full)
    for (idx = 0; idx < 32; idx++) {
      // 32 segment address codes in both A and B types
      if (((rdsInfo->radioTextValid >> idx) & 0x1) == 1) {
	// look for -CR-
	if (rdsInfo->b0 == 0) {
	  // A
	  if (((rdsInfo->radioText[idx*2] >> 8) == 0x0d) || ((rdsInfo->radioText[idx*2] & 0x00ff) == 0x0d)) {
	    rdsInfo->radioTextValid = 0x0000ffff;
	    return;
	  }
	  if (((rdsInfo->radioText[idx*2 +1] >> 8) == 0x0d) || ((rdsInfo->radioText[idx*2 +1] & 0x00ff) == 0x0d)) {
	    rdsInfo->radioTextValid = 0x0000ffff;
	    return;
	  }
	} else {
	  // B
	  if (((rdsInfo->radioText[idx] >> 8) == 0x0d) || ((rdsInfo->radioText[idx] & 0x00ff) == 0x0d)) {
	    rdsInfo->radioTextValid = 0x0000ffff;
	    return;
	  }
	}
      } else {
	// incomplete, leading zero
	return;
      }
    }
  }
}

const u_int16 rdsconv[] = {
  0xe1, 0xe0, 0xe9, 0xe8, 0xed, 0xec, 0xd3, 0xd2,
  0xfa, 0xf9, 0xd1, 0xc7, 0x53, 0xdf, 0xa1, 0x4a,
  0xe2, 0xe4, 0xea, 0xeb, 0xee, 0xef, 0xf4, 0xf6,
  0xfb, 0xfc, 0xf1, 0xe7, 0x73, 0x67, 0x7c, 0x6a,
  0xaa, 0x61, 0xa1, 0x89, 0x47, 0x65, 0xf1, 0xd6,
  0x1e, 0x80, 0xa3, 0x24, 0x08, 0x8d, 0x7f, 0x9d,
  0xb0, 0xb9, 0xb2, 0xb3, 0xb1, 0xa1, 0x6e, 0xfc,
  0xb5, 0xbf, 0xf7, 0xba, 0xbc, 0xbd, 0xbe, 0xa7,
  0xc1, 0xc0, 0xc9, 0xc8, 0xcd, 0xcc, 0xd3, 0xd2,
  0xda, 0xd9, 0x52, 0x43, 0x8a, 0x83, 0xd0, 0x4c,
  0xc5, 0xc4, 0xca, 0xcb, 0xce, 0xc0, 0xd4, 0xd6,
  0xdb, 0xdc, 0x72, 0x63, 0x9a, 0x9e, 0xf0, 0x6c,
  0xc3, 0xc5, 0xc6, 0x8c, 0x79, 0xdd, 0xd5, 0xd8,
  0xfe, 0x6e, 0x52, 0x43, 0x53, 0x5a, 0x87, 0xf0,
  0xe3, 0xe5, 0xe6, 0x9c, 0x77, 0xfd, 0xf5, 0xf8,
  0xfe, 0x6e, 0x72, 0x63, 0x73, 0x7a, 0x87, 0x20
};


void RdsParseText(register char *target, register const u_int16 *rdsText, register u_int16 rdsWords) {
  u_int16 idx = 0;
  u_int16 skip = 0;
  char *tp = target;
  int lastNotSpaceIdx = 0;
  int rdsChars = rdsWords*2;

  for (idx = 0; idx < rdsChars; idx++) {
    if (!(idx&1)) {
      *tp = *rdsText >> 8;
    } else {
      *tp = *rdsText++ & 0x00ff;
    }
    if (*tp > 0x20) {
      lastNotSpaceIdx = idx;
    } else {
      *tp = 0x20;
    }
    tp++;
  }

  tp = target;
  // convert characters
  for (idx = 0; idx <= lastNotSpaceIdx; idx++) {
    if (*tp >= 128) {
      *tp = rdsconv[*tp-128];
    }
    tp++;
  }
  *tp = '\0';
}

void ParseEon(register struct rdsGroup *rdsGroup,
	      register struct rdsInfo *rdsInfo,
	      register struct eonSet *eonSet) {
  u_int16 variant = rdsGroup->data[1] & 0x000f;
  s_int16 eonIdx = -1;
  u_int16 idx = 0;

  // is this PI familiar?
  for (idx = 0; idx < rdsInfo->eonPiFound; idx++) {
    if (rdsGroup->data[3] == eonSet[idx].piOn) {
      eonIdx = idx;
      break;
    }
  }
  /* If not found, add */
  if (eonIdx < 0) {
    if (rdsInfo->eonPiFound >= MAX_EON) {
      /* EON table full, abort */
      return;
    }
    eonIdx = rdsInfo->eonPiFound++;
  }

  eonSet += eonIdx;
  eonSet->piTn = rdsGroup->data[0];
  eonSet->piOn = rdsGroup->data[3];
  eonSet->tpOn = (rdsGroup->data[1]>>4) & 0x0001;
	
  // parse by variant
  if (variant < 4) {
    // PS (characters)
    eonSet->ps[variant] = rdsGroup->data[2];
  } else if (variant == 4) {
    // method
    eonSet->freqsA = rdsGroup->data[2];
  } else if (variant < 10) {
    // tuning freq, AM in slot 4
    eonSet->freqs[variant-5] = rdsGroup->data[2];
  } else if (variant == 12) {
    eonSet->linkage = rdsGroup->data[2];
  } else if (variant == 13) {
    eonSet->ptyTa = rdsGroup->data[2];
  } else if (variant == 14) {
    eonSet->pin = rdsGroup->data[2];
  }
}





u_int16 RdsParser(register struct rdsGroup *rdsGroup,
		  register struct rdsInfo *rdsInfo,
		  struct eonSet *eonSet, register struct fmStatus *fmStatus) {
  // if we are in middle of channel visit, dont make changes in values
  if (fmStatus->tpActive == 1) {
    rdsInfo->tpReturn = RdsEndTP(rdsGroup);
    // nothing else matters
    return 0;
  }
  
  // Block A:
  // This is always Programme Identification PI
  if ((rdsInfo->pi != 0) && (rdsInfo->pi != rdsGroup->data[0])) {
    // PI may toggle in case of programme being switched between local and national
    // to be sure rds info may be invalidated here
    //printf("PI is changing from 0x%04x to 0x%04x\n", rdsInfo->pi, rdsGroup->data[0]);
    
    InitRds(rdsInfo, eonSet);
    return 0;
  }
  rdsInfo->pi = rdsGroup->data[0];

	
  // Block B:
  rdsInfo->group = rdsGroup->data[1] >> 12;
  rdsInfo->b0 = (rdsGroup->data[1] >> 11) & 0x0001;
  rdsInfo->tp = (rdsGroup->data[1] >> 10) & 0x0001;
  rdsInfo->pty = (rdsGroup->data[1] >> 5) & 0x001f;
	
  switch (rdsInfo->group) {
  case 0:
    // Basic tuning and switching information
    // 4 REPEATS
    rdsInfo->ta = (rdsGroup->data[1] >> 4) & 0x0001;
			
    rdsInfo->ms = (rdsGroup->data[1] >> 3) & 0x0001;
			
    // block D has channel name, both A and B types
    rdsInfo->progName[rdsGroup->data[1] & 0x0003] = rdsGroup->data[3];
    rdsInfo->progNameValid |= 0x0003 << ((rdsGroup->data[1] & 0x0003) << 1);
			
    // should overwrite itself
    if (((rdsGroup->data[1] & 0x004)>>2) == 1) {
      rdsInfo->di |= 1 << (3-(rdsGroup->data[1] & 0x0003));
    } else {
      rdsInfo->di &= 0xffff & (~(1 << (3-(rdsGroup->data[1] & 0x0003))));
    }
    
    if (rdsInfo->b0 == 0) {
      // find alternative frequencies, only type A
      GetAF(rdsInfo, fmStatus, rdsGroup->data[2]);
    } else {
      rdsInfo->pi = rdsGroup->data[2];
    }
    
    break;
  case 1:
    // Programme item number code in A and B
    rdsInfo->programmeItemNumber = rdsGroup->data[3];
    if (rdsInfo->b0 == 0) {
      // A
      // Radio paging codes
      rdsInfo->radioPagingCodes = rdsGroup->data[1] & 0x001f;
      
      // Slow labelling codes
      rdsInfo->slowLabellingCodes = rdsGroup->data[2];
    } else {
      // B
      rdsInfo->pi = rdsGroup->data[2];
    }
    break;
    
  case 2:
    // Radiotext
    // if textAB toggles, clear radioText
    if (rdsInfo->textAB != ((rdsGroup->data[1] >> 4) & 0x0001)) {
      memset(rdsInfo->radioText, 0, sizeof(rdsInfo->radioText));
      rdsInfo->radioTextValid = 0;
      
      //puts("ABtoggle");
    }
    rdsInfo->textAB = (rdsGroup->data[1] >> 4) & 0x0001;
    
    if (rdsInfo->b0 == 0) {
      // A
      rdsInfo->radioText[((rdsGroup->data[1] & 0x000f)<<1)] = rdsGroup->data[2];
      rdsInfo->radioText[((rdsGroup->data[1] & 0x000f)<<1) +1] = rdsGroup->data[3];
      rdsInfo->radioTextValid |= 1L << (rdsGroup->data[1] & 0x000f);
    } else {
      // B
      rdsInfo->pi = rdsGroup->data[2];
      rdsInfo->radioText[rdsGroup->data[1] & 0x000f] = rdsGroup->data[3];
      rdsInfo->radioTextValid |= 0x0001 << ((rdsGroup->data[1] & 0x000f));
    }
    //			printf("0x%08lx \n",rdsInfo->radioTextValid);
    //			PrintRdsText(rdsInfo->radioText, 64);
    
			// See if radiotext message is complete:
    RdsCheckRT(rdsInfo);
    
    break;
    
  case 4:
    // Clock-time and date
    if (rdsInfo->b0 == 0) {
      // A
      rdsInfo->dayCode = 0;
      rdsInfo->dayCode |= rdsGroup->data[1] & 0x0003; // 2 bits to lsb
      rdsInfo->dayCode <<= 15; // Move previous towards msb (no need to typecast)
      rdsInfo->dayCode |= rdsGroup->data[2] >> 1;
      rdsInfo->hour = (rdsGroup->data[2] & 0x0001) << 4;
      rdsInfo->hour |= (rdsGroup->data[3] & 0xf000) >> 12;
      rdsInfo->minute = (rdsGroup->data[3] & 0x0fc0) >> 6;
      rdsInfo->timeOffset = rdsGroup->data[3] & 0x003f;
      rdsInfo->timeValid = 1;
    } else {
      // B
      //if(DBG)printf ("NOTE: type 4B groups not implemented\n");
    }
    break;
    
    
  case 8:
    // TMC / ODA
    // unsupported
    break;
    
  case 10:
    //Programme type name
    if (rdsInfo->b0 == 0) {
      if (((rdsGroup->data[1]>>4)&0x0001) != rdsInfo->ptynAB) {
	rdsInfo->ptynAB = (rdsGroup->data[1]>>4)&0x0001;
	rdsInfo->ptynValid = 0;
	memset(rdsInfo->ptyn, 0, sizeof(rdsInfo->ptyn));
      }
      
      if ((rdsGroup->data[1]&0x0001) == 0) {
	rdsInfo->ptyn[0] = rdsGroup->data[2];
	rdsInfo->ptyn[1] = rdsGroup->data[3];
	rdsInfo->ptynValid |= 0x000f;
      } else {
	rdsInfo->ptyn[2] = rdsGroup->data[2];
	rdsInfo->ptyn[3] = rdsGroup->data[3];
	rdsInfo->ptynValid |= 0x00f0;
      }
    }
    break;
    
  case 14:
    // determine othernetwork PI first,
    // then see if data exists already
    if (rdsInfo->b0 == 0) {
      if (rdsGroup->data[3] == 0) {
	//if(DBG)printf ("NOTE: PI(on) is zero\n");
	// not going to compare well
	return 1;
      } else {
	ParseEon(rdsGroup, rdsInfo, eonSet);
      }
      
    } else {
      // B
      // if TA is 1, jump to TP program
      //	    u_int16 idx = 0;
      //if(DBG)printf ("Got 14B\n");
      
      //	    printf ("A:0x%04x, B:0x%04x, C':0x%04x, D:0x%04x\n",
      //		    rdsGroup->data[0], rdsGroup->data[1], rdsGroup->data[2], rdsGroup->data[3]);
      
      if (((rdsGroup->data[1])&(ON_TA|ON_TP)) == (ON_TA|ON_TP)) {
	u_int16 idx = 0;
	// if allowed move to eon channel providing traffic announcement
	for (idx = 0; idx < rdsInfo->eonPiFound; idx++) {
	  if (rdsGroup->data[3] == eonSet[idx].piOn) {
	    // found
	    if (fmStatus->tpVisitEna == 1) {
	      // Request channel change
	      rdsInfo->tpReq = 1;
	      rdsInfo->tpReqTarget = idx;
	      rdsInfo->tpReturn = 0;
	    }
	  } else if (idx == rdsInfo->eonPiFound -1) {
	    if(DBG)printf ("NOTE: TP starts in unknown channel\n");
	  }
	}
      }
    }
    break;
    
  default:
    
    break;
  }// switch-case
  
  return 0;
}
	


#if 0
#define AU192K_CAPTURE
#endif

extern u_int16 dump, rdsDebug;

/* MAC0 = 0x26 interrupt C code. */
extern u_int16 Mac0IntVector;
void Mac0IntC(void) {
#ifdef AU192K_CAPTURE
  if (dump) {
    s_int16 sample = PERIP(FM_BASEBAND);
    int i;
#define LOOPS 64
    PERIP(GPIO0_ODATA) = (sample >> 9) | 1;
    PERIP(GPIO0_CLEAR_MASK) = 1;
    for (i=0; i<LOOPS; i++) PERIP(0);
    PERIP(GPIO0_ODATA) = (sample >> 1) & ~1;
    PERIP(GPIO0_SET_MASK) = 1;
  } else {
#endif
    rdsfilt.fifowr[0] = PERIP(FM_BASEBAND);
    rdsfilt.fifowr++;
    if (rdsfilt.fifowr >= &bpmem2[RDS_FIFOSIZE])
    rdsfilt.fifowr -= RDS_FIFOSIZE;
#ifdef AU192K_CAPTURE
  }
#endif
}


#if 0
void PrintInt(void) {
  static const char *intName[] = {
    "DAC", "USB", "XPERIP", "SP0", "SP1", "MAC1", "MAC0", "GP0",
    "GP1", "GP2", "MAC2", "I2S", "TX",  "RX",  "TI0", "TI1", 
    "TI2", "FM",  "SRC", "DAO", "RTC", "RDS", "SPDIFR", "SPDIFT", 
    "POS", "REG", "PWM", "SAR", "ERR", "ERR", "ERR", "ERR"
  };
  u_int16 ie[32];
  u_int32 l;
  u_int16 i;
  memset(ie,0,sizeof(ie));
  fprintf(vo_stderr,"Interrupts: ");
  l = ((u_int32)PERIP(INT_ENABLE1_HP) << 16) 
    | (u_int32)PERIP(INT_ENABLE0_HP);
  for (i=0; i<32; i++) if (l&(1L<<i)) ie[i] += 2;
  l = ((u_int32)PERIP(INT_ENABLE1_LP) << 16) 
    | (u_int32)PERIP(INT_ENABLE0_LP);
  for (i=0; i<32; i++) if (l&(1L<<i)) ie[i] += 1;
  for (i=0; i<32; i++) {
    if (ie[i]) {
      fprintf(vo_stderr, "INT%d_%s:%d->0x%04x ",
	      i, intName[i], ie[i],
	      (u_int16)(ReadIMem((void *)(0x20+i))>>6L));
    }
  }
  fprintf(vo_stderr,"\n");
}
#endif


char ToChar(register int c) {
  c &= 0xFF;
  return isprint(c) ? c : '.';
}

u_int16 GetRds(register struct rdsGroup *rdsGroup,
	       register struct rdsInfo *rdsInfo,
	       struct eonSet *eonSet, register struct fmStatus *fmStatus,
	       u_int16 *sync, u_int16 *rdsBlockIdx,
	       u_int16 *success) {
	
  u_int16 result = 0;
  u_int16 rdsData;
  u_int16 rdsCheck;
  u_int16 process = 0;
  static u_int32 rawBits = 0;
  static int groupBitPhase = -1;
  static const u_int16 checkOffset[5] = {
    OFFSET_A, OFFSET_B, OFFSET_C, OFFSET_D, OFFSET_C2
  };
  static u_int32 lastReportTime = 0, lastGoodGroupTime = 0;
  static u_int16 goodBlocks = 0;
  u_int32 timeCount = ReadTimeCount();


#ifdef AU192K_CAPTURE
  if (dump) {
    printf("Dump\n");
    PERIP(INT_ENABLE1_LP) = 0;
    PERIP(INT_ENABLE1_HP) = 0;
    PERIP(INT_ENABLE0_LP) = INTF_MAC0|INTF_UART_RX|INTF_UART_TX;
    PERIP(INT_ENABLE0_HP) = INTF_MAC0;
    while (1);
  }
#endif

#if 1
  if (timeCount-lastReportTime >= (u_int32)(1/1187.5*26*100*1000)) {
    if (goodBlocks > 100) {
      goodBlocks = 100;
    }
    if (rdsDebug) {
      printf("Rds quality %3d%%\n", goodBlocks, PERIP(FM_CF));
    }
    *success = goodBlocks;
    goodBlocks = 0;
    lastReportTime = timeCount;
  } else if (!(PERIP(INT_ENABLE0_HP) & INTF_MAC0)) {
    goodBlocks = 0;
    lastReportTime = timeCount;
  }
#endif

  while (1) {
    static int i, syndr[5];
    int allBad = 1;

    do {
      int t;
      if ((t = RdsGetBit()) < 0) {
	return -1;
      }
      //    printf("%d", t);
      rawBits = (rawBits << 1) | t;
    } while (groupBitPhase >= 0 && (++groupBitPhase < 26));
    rdsData = rawBits >> 10;
    rdsCheck = rawBits & 1023;

#if 0
    for (i=0; i<5; i++) {
      syndr[i] = CheckSyndrome(&rdsData, rdsCheck, checkOffset[i]);
      allBad &= syndr[i] ? 1 : 0;
    }
#endif
#if 0
    if (!allBad) {
      static u_int32 lastGood = 0;
      u_int32 newGood = ReadTimeCount();
      for (i=0; i<5; i++) {
	printf("%c", syndr[i] ? '.':'X');
      }
      printf(" %4ld\n", newGood-lastGood);
      lastGood = newGood;
    }
#endif

    if (!CheckSyndrome(&rdsData, rdsCheck, checkOffset[*rdsBlockIdx]) ||
	(*rdsBlockIdx == 2 &&
	 !CheckSyndrome(&rdsData, rdsCheck, checkOffset[4]))) {
      rdsGroup->data[*rdsBlockIdx] = rdsData;
      rdsGroup->valid = 1;
      *sync = 1;
#if 0
      printf("#1 %d\n", *rdsBlockIdx);
#endif
      (*rdsBlockIdx)++;
      groupBitPhase = 0; 
      goodBlocks++;
    } else {
      rdsGroup->data[*rdsBlockIdx] = 0;
      rdsGroup->valid = 0;
      *sync = 0;
#if 0
      printf("#3 %d\n", *rdsBlockIdx);
#endif
      *rdsBlockIdx = 0;
      groupBitPhase = -1;
    }
    if (*rdsBlockIdx == 4) {
      lastGoodGroupTime = timeCount;
      *rdsBlockIdx = 0;
      process = 1;
    }

    if (process) {
#if 0
      printf("%04x %04x %04x %04x %c%c %c%c\n",
	     rdsGroup->data[0],
	     rdsGroup->data[1],
	     rdsGroup->data[2],
	     rdsGroup->data[3],
	     ToChar(rdsGroup->data[2] >> 8),
	     ToChar(rdsGroup->data[2]),
	     ToChar(rdsGroup->data[3] >> 8),
	     ToChar(rdsGroup->data[3]));
#endif
      result = RdsParser(rdsGroup, rdsInfo, eonSet, fmStatus);
	    	
      // See if need to change channel
      if (rdsInfo->tpReq && fmStatus->tpVisitEna && !fmStatus->tpActive) {
	// Traffic program detected
	// determine if necessary information is present to jump channel
	u_int32 target = FM_LOW +
	  (eonSet[rdsInfo->tpReqTarget].freqs[0] & 0x00ff)*100L;
	
	if (target >= FM_LOW && target <= FM_HIGH) {
	  // Frequency is usable
	  fmStatus->tpActive = 1;
	  
	  // jump
	  SetFMFreq(target);
	  printf("TP requested jump to channel %lu\n", target);
	}
      }
      
      if (rdsInfo->tpReturn == 1 ||
	  ((int)((lastGoodGroupTime-timeCount/1000) > fmStatus->tpRetLimit) &&
	   fmStatus->tpActive == 1)) {
	printf ("Return to %5.3f MHz after TP jump\n", fmStatus->freq);
	if (rdsInfo->tpReturn == 1) {
	  printf ("  Reason: TP went down in RDS\n");
	} else {
	  printf ("  Reason: No RDS in new channel for %d seconds\n",
		  fmStatus->tpRetLimit);
	}
	
	rdsInfo->tpReqTarget = 0;
	rdsInfo->tpReq = 0;
	rdsInfo->tpReturn = 0;
	fmStatus->tpActive = 0;
	
	SetFMFreq(fmStatus->freq);
	return -1;
      }
      return 0;
    } /* if (process) */
  } /* while (1) */

  return -1;
}




u_int16 GetAF(register struct rdsInfo *rdsInfo,
	      register struct fmStatus *fmStatus,
	      register u_int16 data) {
	u_int16 idx = 0;
	
	// determine method
	if (rdsInfo->afMethod == 0) {
		// determine if incoming data has number of following AF:s in high byte
		if (((data>>8) >= 225) && ((data>>8) <= 249)) {
			// see if first frequency is current tuned
			if ((FM_LOW + (data & 0x00ff)*100L) == fmStatus->freq) {
				// combination of number of freqs and tuned freq is B
				// OR A for channel with repeaters
				
				// pick candidate and vote:
				rdsInfo->afMethodCandidate = data & 0x00ff;
				rdsInfo->afMethodCount = 1;
			}
			else {
				// Possible results are invalid frequency B and valid A
				// pick candidate and see what follows
				rdsInfo->afMethodCandidate = data & 0x00ff;
				rdsInfo->afMethodCount = 1;
			}
		}
		else {
			// are we trying to vote for method?
			if (rdsInfo->afMethodCount == 1) {
				if ((rdsInfo->afMethodCandidate == (data & 0x00ff))
				|| (rdsInfo->afMethodCandidate == (data >> 8))) {
		    	
					// B list,
					rdsInfo->afMethod = 2;
					rdsInfo->afMethodCount = 0;
				}
				else {
					// propable A
					rdsInfo->afMethod = 1;
					rdsInfo->afMethodCount = 0;
				}
			}
		}
	}
	else {
		// Gather actual data after determining AF signaling method
		
		if ((rdsInfo->afMethod == 1)
		|| ((rdsInfo->afMethod == 2) &&
		(((FM_LOW + (data & 0x00ff)*100L) == fmStatus->freq)
		|| ((FM_LOW + (data >> 8)*100L) == fmStatus->freq) ))) {
			// get data
			// all data
			// this includes initial data & af number
	    	
			for (idx = 0; idx < rdsInfo->afIdx; idx++) {
				// do we already have this
				if (rdsInfo->alternativeFreq[idx] == data) {
					// nothing to add
					return 0;
				}
			}
	    	
			// try to add this data
			if (rdsInfo->afIdx < MAX_AF -1) {
				// there is room
				rdsInfo->alternativeFreq[rdsInfo->afIdx] = data;
				rdsInfo->afIdx++;
				return 1;
			}
			else {
				// no more room
				return 0;
			}
		}
		else {
			// do nothing as data is uninteresting or coming at bad time
			//	    printf ("data: 0x%04x\n",data);
		}
	}
	
	return 0;
}



void InitRds(register struct rdsInfo *rdsInfo, register struct eonSet *eonSet) {
  u_int16 idx = 0;
	
  /* Clear the whole structures */
  memset(rdsInfo, 0, sizeof(*rdsInfo));
  memset(eonSet, 0, sizeof(*eonSet));
	
  rdsInfo->afMethodCount = 2;
  memset(rdsInfo->progName,0x2020,sizeof(rdsInfo->progName)); // set to ascii "  "

  /* Set interrupt vector. */
  WriteIMem(0x20+INTV_MAC0, ReadIMem((u_int16)(&Mac0IntVector)));
	
  // Set some registers
#if 0
  USEY(FM_CF) |= FM_CF_RDSENA;
#else
  PERIP(INT_ENABLE0_LP) |= INTF_MAC0;
  PERIP(INT_ENABLE0_HP) |= INTF_MAC0;
#endif	
}


void InitFmStatus(register struct fmStatus *fmStatus) {
	u_int16 idx = 0;
	
	// set everyting to zero, only set to non-zeros are requred afterwards
	memset(fmStatus, 0, sizeof(*fmStatus));
	
	fmStatus->activeSelection = 0; // no active
	
	fmStatus->freq = 87600;    // Defaults to fm low 87.6MHz
	fmStatus->audioEna = 1;    // Audio on
	fmStatus->rdsEna = 1;      // RDS on
	fmStatus->rdsScan = 0;     // Channel search off
	fmStatus->tpVisitEna = 1;  // Allow channel visit for reception of Traffic Program
	fmStatus->tpRetLimit = 10;// umber of seconds without a single good RDS group before returning from TP
	fmStatus->tpActive = 0;    // Visiting channel for traffic program
	fmStatus->rdsStatus = 999; // RDS data reception quality estimate, 0 bad, 100 best
								   // 101 failure, 999 no value yet
	fmStatus->rdsDrops = 0;    // number RDS blocks dropped in row for bad crc
    
	fmStatus->foundCh = 0;
	memset(fmStatus->channels, 0, sizeof(fmStatus->channels));
	

	fmStatus->smEna = 1;      // Enable automatic stereo/mono toggling
	
	fmStatus->iqLevel = 0;
	fmStatus->iqLevelRaw = 0;
	fmStatus->iqLevelCount = IQ_AVERAGE;
	
	memset(fmStatus->iqTable, 0, sizeof(fmStatus->iqTable));
	
	fmStatus->iqRecent = 0x00ff; // low to max, hi to min
	fmStatus->iqDistant = 0x00ff;
	
	fmStatus->enaPartialDecode = 1;
}


#if 0 // unused...
void PrintRdsInfo(register struct rdsInfo rdsInfo, register struct eonSet *eonSet) {
	u_int16 idx = 0;
	u_int16 hour = 0;
	u_int16 year = 0;
	u_int16 yearP = 0;
	u_int16 month = 0;
	u_int16 monthP = 0;
	u_int16 day = 0;
	
	printf("\tpi:      0x%04x\n", rdsInfo.pi);
	printf("\tgroup:   0x%04x\n", rdsInfo.group);
	printf("\tb0:      0x%04x\n", rdsInfo.b0);
	printf("\tpty:     0x%04x ", rdsInfo.pty);
	printf(": \"%s\"\n",PTYTable[rdsInfo.pty]);
	printf("\ttp:      0x%04x\n", rdsInfo.tp);
	printf("\tta:      0x%04x\n", rdsInfo.ta);
	printf("\tms:      0x%04x\n", rdsInfo.ms);
	printf("\tdi:      0x%04x\n", rdsInfo.di);

	if ((rdsInfo.di & 0x0001) == 1) {
		printf("\t --> Stereo, ");
	}
	else {
		printf("\t --> Mono, ");
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
	
	printf("\tRadio Paging Codes:    0x%04x\n", rdsInfo.radioPagingCodes);
	printf("\tSlow Labelling Codes:  0x%04x\n", rdsInfo.slowLabellingCodes);
	printf("\tProgramme Item Number: 0x%04x ", rdsInfo.programmeItemNumber);
	
	printf("day: %u, ", rdsInfo.programmeItemNumber>>11);
	printf("%02u:%02u\n", ((rdsInfo.programmeItemNumber>>6) & 0x001f),
	(rdsInfo.programmeItemNumber & 0x003f));
	
	printf("\ttextAB:    0x%04x\n", rdsInfo.textAB);
	printf("\tdayCode:    0x%08lx\n", rdsInfo.dayCode);
	printf("\thour:       %u\n", rdsInfo.hour);
	printf("\tminute:     %u\n", rdsInfo.minute);
	printf("\ttimeOffset: 0x%04x\n", rdsInfo.timeOffset);
	
	if ((rdsInfo.timeOffset >> 5) == 0) {
		// positive offset
		hour = rdsInfo.hour + ((rdsInfo.timeOffset & 0x001f)>>1);
		if (hour > 23) {
			// overflow
			hour -= 24;
		}
	}
	else {
		// negative offset
		if (rdsInfo.hour > ((rdsInfo.timeOffset & 0x001f)) >>1) {
			hour = rdsInfo.hour - ((rdsInfo.timeOffset & 0x001f)>>1);
		}
		else {
			// underflow
			hour = 24 - (((rdsInfo.timeOffset & 0x001f)>>1) - rdsInfo.hour);
		}
	}
    	
    	
    	
	//    printf ("DBG:daycode is %lu\n", rdsInfo.dayCode);
	year = (u_int16)(((float)rdsInfo.dayCode - 15078.2)/365.25);
    	
	month = (u_int16)((((float)rdsInfo.dayCode - 14956.1) - ((u_int16)(year*365.25)))/30.6001);
    	
	day = (u_int16)(rdsInfo.dayCode - 14956 - (u_int16)(year * 365.25) - (u_int16)(month * 30.6001));
    	
	if (month == 14 || month == 15) {
		monthP = month - 1 - 12;
		yearP = 1900 + year + 1;
	}
	else {
		monthP = month -1;
		yearP = 1900+year;
	}
    	
	printf ("Date (dd.mm.yyyy): %02u.%02u.%u\n", day, monthP, yearP);
    	
	//    if (rdsInfo.dayCode == 0) {
	//	printf ("Time is uninitialized\n");
	//    }
	//    else {
	printf ("Time is %02u.%02u\n", hour, rdsInfo.minute);
	//    }
    	
	printf ("ProgName: \t");
	PrintRdsText(rdsInfo.progName, 8);
	
	printf ("RadioText:\n");
	PrintRdsText(rdsInfo.radioText, 64);
	
	printf ("Programme Type Name:\t");
	PrintRdsText(rdsInfo.ptyn, 8);
   	 
	printf ("Alternative Frequencies: Currently got %u pairs Method %u\n",
	rdsInfo.afIdx, rdsInfo.afMethod);
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
   	 
   	 
	printf ("EON pi(on):\t");
	for (idx = 0; idx < rdsInfo.eonPiFound; idx++) {
		printf ("%u: 0x%04x ",idx, eonSet[idx].piOn);
	}
	printf("\n");
	
	for (idx = 0; idx < rdsInfo.eonPiFound; idx++) {
		u_int16 iii = 0;
		printf ("EON PS:(%u), TP: %u\t", idx, eonSet[idx].tpOn);
		PrintRdsText(eonSet[idx].ps, 8);
		printf("  Freqs: ");
		for (iii = 0; iii < 5; iii++) {
			if ((eonSet[idx].freqs[iii] >> 8) != 0
			&& (eonSet[idx].freqs[iii] >> 8) < 205) {
				// upper
				printf("(%u) ", 875 + (eonSet[idx].freqs[iii] >> 8));
			}
			if ((eonSet[idx].freqs[iii] & 0x00ff) != 0
			&& (eonSet[idx].freqs[iii] & 0x00ff) < 205) {
				//lower
				printf("(%u) ", 875 + (eonSet[idx].freqs[iii] & 0x00ff));
			}
		}
		printf("\n");
	}
    	
	printf("\n");
}
#endif

#if 0
void PrintRdsText(register u_int16 charTable[], register u_int16 characters) {
	u_int16 idx;


	printf("PRINTRDS\n");
	
	for (idx = 0; idx < (characters+1)>>1; idx++) {
		// \todo skandit
		if ((charTable[idx]>>8) == 0x00d1) {
			printf("A");
		}
		else if ((charTable[idx]>>8) == 0x0091) {
			printf("a");
		}
		else if ((charTable[idx]>>8) == 0x00d7) {
			printf("O");
		}	
		else if ((charTable[idx]>>8) == 0x0097) {
			printf("o");
		}
		else if ((charTable[idx]>>8) == 0x000a) {
			// line feed (preferred line break)
			printf("\\");
		}
		else if ((charTable[idx]>>8) == 0x000d) {
			// end (was \r)
			printf("\n");
			return;
		}
		else if ((charTable[idx]>>8) == 0x0000) {
			// No char here yet
			printf(" ");
		}
		else if ((charTable[idx]>>8) != 0x0000){
			printf("%c",charTable[idx]>>8 );
			fflush (stdout);
		}
		
		
		if ((charTable[idx]&0x00ff) == 0x00d1) {
			printf("A");
		}
		else if ((charTable[idx]&0x00ff) == 0x0091) {
			printf("a");
		}
		else if ((charTable[idx]&0x00ff) == 0x00d7) {
			printf("O");
		}
		else if ((charTable[idx]&0x00ff) == 0x0097) {
			printf("o");
		}
		else if ((charTable[idx]&0x00ff) == 0x000a) {
			// line feed (preferred line break)
			printf("\\");
		}
		else if ((charTable[idx]&0x00ff) == 0x000d) {
			// end (was \r)
			printf("\n");
			return;
		}
		else if ((charTable[idx]&0x00ff) == 0x0000) {
			// No char here yet
			printf(" ");
		}
		else if ((charTable[idx]&0x00ff) != 0x0000) {
			printf("%c",charTable[idx]&0x00ff);
			fflush (stdout);
		}
	}
	printf("\n");
	
}
#endif

#endif /* USE_RDS */
