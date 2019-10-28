//
//
// Functions for RDS
//
//
#ifndef RDS_H
#define RDS_H

#if 1
/* If undefined, don't compile or use RDS features */
#define USE_RDS
#endif

#include <vo_stdio.h>
#include <vs1005h.h>

#include "fm_function.h"
#include "fm_init.h"
#include "ecc.h"
#include "audio.h"
#include "fmModel.h"

// DEBUG:
//print quality of rds reception after 100 blocks (q:xx) % (partial decoding affects this)
#if 0
#define PRINT_QUALITY
#endif


extern u_int16 *rdsBufferWP;
extern u_int16 *rdsBufferRP;


#define RDS_FORCESYNC   0x406F

#define OFFSET_A  0x0fc
#define OFFSET_B  0x198
#define OFFSET_C  0x168
#define OFFSET_C2 0x350
#define OFFSET_D  0x1b4
#define OFFSET_E  0x000

#define MAX_EON 32 // eon sets stored
#define MAX_AF  25 // Alternate frequencies stored

//#define FM_LOW 87500
//#define FM_HIGH 108000

//#define IQ_AVERAGE 512 // iq averaging rounds per result value
//#define IQ_AVERAGE 64 // iq averaging rounds per result value
#define IQ_AVERAGE 32 // iq averaging rounds per result value

// maximum number of time extensions for channel name search
// this is used if partial name is received
#define SEARCH_GRACE 4 



// storage for one RDS block
struct rdsBlock {
    u_int16 data;
    u_int16 crc;
    u_int16 type;
};

// storage for one RDS group
struct rdsGroup {
    u_int16 data[4];
    u_int16 valid;
};

// received RDS information
struct rdsInfo {
	u_int16 pi;
	u_int16 group;
	u_int16 b0;
	u_int16 tp;
	u_int16 pty;
	u_int16 ta;
	u_int16 ms;
	u_int16 di;
	u_int16 afCount; // number of received AF:s
	u_int16 afIdx; // index of next AF
	u_int16 afExpected; // number of expected AF:s
	
	u_int16 afMethod; // used method for AF: 0:unknown, 1:A, 2:B
	u_int16 afMethodCount;
	u_int16 afMethodCandidate;
	u_int16 alternativeFreq[MAX_AF]; // AF:data
	u_int16 progName[4];
	u_int16 progNameValid;
	
	// type1
	u_int16 radioPagingCodes;
	u_int16 slowLabellingCodes;
	u_int16 programmeItemNumber;
	
	// type2
	u_int16 textAB;
	u_int16 radioText[32];
	u_int32 radioTextValid;
	
	// type4
	u_int32 dayCode;
	u_int16 hour;
	u_int16 minute;
	u_int16 timeOffset;
	u_int16 timeValid;
	
	// type 10
	u_int16 ptynAB;
	u_int16 ptyn[4];
	u_int16 ptynValid;
	
	// type 14
	u_int16 eonPiOn[MAX_EON];
	u_int16 eonPiFound;
	
	u_int16 eonPs[4];
	u_int16 eonMethod;
	u_int16 eonFreqs[5];
	u_int16 eonLinkage;
	u_int16 eonPtyTa;
	u_int16 eonPin;
	
	u_int16 tpReq;
	u_int16 tpReqTarget;
	u_int16 tpReturn;
};

// one set of Enhanced Other Network information
struct eonSet {
    u_int16 piTn;
    u_int16 piOn;
    u_int16 ps[4];
    u_int16 freqsA;
    u_int16 freqs[5];
    u_int16 linkage;
    u_int16 ptyTa;
    u_int16 pin;
    u_int16 tpOn;
    
};

// determines limit for moving between mono and stereo reception
#define MONO_THRESHOLD 8
#define GUI_DEPTH 8

// RDS radio status, controls and stored information for channel and analysis
struct fmStatus {
	u_int32 freq;       // Defaults to fm low 87.6MHz (1kHz step)
	u_int16 audioEna;   // Audio on/off
	u_int16 rdsEna;     // RDS on/off
	u_int16 rdsScan;    // Channel search mode on/off
	u_int16 tpVisitEna; // Allow channel visit for reception of Traffic Program
	u_int16 tpRetLimit; // Number of seconds without a single good RDS group before returning from TP
	u_int16 tpActive;   // Visiting channel for traffic program
	u_int16 rdsStatus;   // RDS data reception quality estimate, 0 worst, 100 best
							// 101 failure, 999 no value yet
	u_int16 rdsDrops;   // number RDS blocks dropped in row for bad crc
	
	u_int16 foundCh;    // number of (auto)detected channels
	u_int32 channels[FMCHANS]; // list of found (or manually set) channel freqs (in 10kHz)
	u_int16 chNames[FMCHANS][4]; // names of found channels
	
	u_int16 smEna;      // enable automatic Stereo / mono toggling
	
	u_int16 iqLevel;     // absolute iq level average for quality estimate
	u_int32 iqLevelRaw;  // raw for previous
	u_int16 iqLevelCount;// counter for previous
	
	u_int32 iqTable[2];  // Saved IQ data, 4bit/sample, 16 samples (0-14 + 15 and over)
							// most recent in high bits of high index
    
	u_int16 iqRecent; // from iqTable[1]: smallest value in low, largest in high byte
	u_int16 iqDistant;   // from iqTable[0]: smallest value in low, largest in high byte
	
	u_int16 enaPartialDecode; // Enable or disable decoding of partially received groups
	 
	u_int16 activeSelection; // 0: no selection, [1,FMCHANS]: active channel as in ui messages 

	
	u_int16 dbgPartial;
	u_int16 dbgGroups;	
};


extern const char* const PTYTable[32];


void RdsParseText(register char *target, register const u_int16 *rdsText, register u_int16 rdsWords);

// FOR ALL FOLLOWING:
// struct rdsGroup *rdsGroup : structure containing one RDS data group
// struct rdsInfo *rdsInfo   : structure containing current channel RDS information
// struct eonSet *eonSet     : structure containing current channel EON information
// struct fmStatus *fmStatus : structure containing current FM-radio status and control


// parse received rds group
// return: 1: failure, 0: success
u_int16 RdsParser(register struct rdsGroup *rdsGroup,
		  register struct rdsInfo *rdsInfo,
		  struct eonSet *eonSet,
		  register struct fmStatus *fmStatus);

// Gather group and call parser
// u_int16 *sync        : last valid block, reset to 0, otherwise do not modify
// u_int16 *rdsBlockIdx : expected block, reset to 0, otherwise do not modify
// u_int16 *success : number of failed blocks during last 100 blocks
// return: 0: success
u_int16 GetRds(register struct rdsGroup *rdsGroup,
	       register struct rdsInfo *rdsInfo,
	       struct eonSet *eonSet, register struct fmStatus *fmStatus,
	       u_int16 *sync, u_int16 *rdsBlockIdx,
	       u_int16 *success);

// Scans for rds channel names(program service name). Other RDS info is disregarded
// u_int16 *sync        : last valid block, reset to 0, otherwise do not modify
// u_int16 *rdsBlockIdx : expected block, reset to 0, otherwise do not modify
// u_int16 *shortErrors : number of failed blocks
// u_int16 *shortCount  : total number of blocks (failed + ok)
void ScanRds(struct rdsGroup *rdsGroup, struct rdsInfo *rdsInfo,
	     struct eonSet *eonSet, struct fmStatus *fmStatus,
	     u_int16 *sync, u_int16 *rdsBlockIdx,
	     u_int16 *shortErrors, u_int16 *shortCount);

// Analyzes channel quality and adjusts mono/stereo quality
// Function gathers set of IQ data to determine changes in reception over time
// sets mono if 8 previous values are below MONO_THRESHOLD 
//  OR fmStatus.iqLevel goes zero (average USEY(FM_IQ_LEV) >> 8 over SHORT time)
// u_int16 *IQValue : IQ-value for current channel
void AnalyzeLevel(u_int16 *IQValue,  struct fmStatus *fmStatus);

// returns 0 if suitable data is not available, 1 if freq not changed, 2 if freq is changed
u_int16 FindBestAlt(struct rdsInfo *rdsInfo, struct fmStatus *fmStatus, struct eonSet *eonSet);

// returns IQValue average over set amount of iterations, called by FindBestAlt 
// u_int32 candidate  : frequency to test in AF-form
// u_int16 iterations : number of rounds per freq (power of two recommended)
// return : IQ average
u_int16 TestFreq(u_int32 candidate, u_int16 iterations);

// Adds AF to rdsInfo
// u_int16 data : data to be added
// return: 0 if no data is added to AF list, otherwise 1
u_int16 GetAF(register struct rdsInfo *rdsInfo,
	      register struct fmStatus *fmStatus,
	      register u_int16 data);

// initializations
void InitRds(register struct rdsInfo *rdsInfo, register struct eonSet *eonSet);
void InitFmStatus(register struct fmStatus *fmStatus);



// Debug
void PrintRdsInfo(register struct rdsInfo rdsInfo, register struct eonSet *eonSet);
void PrintRdsText(register u_int16 charTable[], register u_int16 characters);



// legacy
struct rdsCrc {
    u_int16 correctTab[1024];
    u_int16 nextBlock;
};


/*
  All following are for RDS software detection
 */
#define RDS_FIFOSIZE 4096

extern __align __mem_y s_int16 bpmem2[RDS_FIFOSIZE];

struct RDSFILTER {
  __align __mem_y s_int16 *fifowr;
  __align __mem_y s_int16 *fiford;
  s_int16 *coeffsin;
  s_int16 *coeffcos;
  s_int16 size;   //number of taps/coefficients
  u_int16 modulo; //backwards
  s_int16 resi, resq;
  u_int16 angle;
};

extern struct RDSFILTER rdsfilt;

/* Returns -1 (no data available), 0 or 1 */
int RdsGetBit(void);

#endif
