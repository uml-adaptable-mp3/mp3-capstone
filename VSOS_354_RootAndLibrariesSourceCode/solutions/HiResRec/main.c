/*

  HiResRec - Record by writing in RAW mode to SD card using 32-bit FAT.

*/
#include <vo_stdio.h>
#include <volink.h>     /* Linker directives like DLLENTRY */
#include <apploader.h>  /* RunLibraryFunction etc */
#include <kernel.h>
#include <string.h>
#include <stdlib.h>
#include <audio.h>
#include <consolestate.h>
#include <saturate.h>
#include <vo_fat.h>
#include <swap.h>
#include <rtc.h>
#include <crc32.h>
#include <aucommon.h>
#include <unistd.h>
#include <math.h>
#include <vo_gpio.h>
#include <libaudiodec.h>
#include <uimessages.h>
#include <exec.h>
#include <vsostasks.h>
#include <hwLocks.h>
#include <clockspeed.h>
#include <ByteManipulation.h>
#include <iochannel_devboard.h>
#include <lcd.h>
#include <cyclic.h>
#include <time.h>
#include "hiresAsm.h"
#include "packed.h"

#define VERSION1 "Hi-Res Recorder v0.40"
#define VERSION2 "VLSI Solution 2018"

/*

  Note: The following options can be defined or undefined at will.
  They will affect the user interfaces available. Note that any
  number of input interfaces can be used, you are not limited to one.

  USE_LCD: Use a 160x128 pixels 1.77" TFT LCD, requires LCD177.dl3 to be loaded
  USE_UART_KEYS: Use UART key input to control the recorder.
  USE_DEVBOARD_KEYS: Use VS1005 Developer Board keys S1-S4 to control
    certain aspects of the recorder.
  USE_EXT1_BOARD_KEYS: USE "V1005 Dev. Bard Expension 1" board keys
    and control wheel to control certain aspects of the recorder.

  USE_BWF_FILE_LINKING: If defined, write XML information as a LINK
    chunk to the end of each file.
 */

#if 1
#  define USE_LCD
#else
#  define LcdFilledRectangle error_LcdFilledRectangle
#  define LcdTextOutXY error_LcdTestOutXY
#  define lcd0 error_lcd0
#endif

#if 1
#  define USE_UART_KEYS
#endif

#if 1
#  define USE_DEVBOARD_KEYS
#endif

#if 1
#  define USE_EXT1_BOARD_KEYS
#endif

#if 1
#  define USE_BWF_FILE_LINKING
#endif


struct RtcParameters {
  u_int16 volume  : 5;
  u_int16 rateIdx : 2;
  u_int16 bits24  : 1;
  u_int16 unused  : 8;
};

struct FreeFragment {
  u_int32 firstCluster;
  u_int32 clusters;
  u_int32 firstBlock;
  u_int32 blocks;
};

#ifdef USE_LCD
extern const u_int16 icons_recsym[6][64]; /* from recsym.c */
extern const u_int16 icons_vu[1][]; /* from vu.c */
extern const u_int16 icons_vlsi[1][]; /* from vlsi.c */
#endif

u_int16 outBuf[256];
#define IN_BUF_SIZE_32BIT 512
#define IN_BUF_SIZE_24BIT (IN_BUF_SIZE_32BIT*3/4)
u_int16 inBuf[IN_BUF_SIZE_32BIT];
char tStr[80];
DEVICE *sdDev = NULL;
struct FreeFragment freeFrag;

DLLIMPORT(vs23MaxFill)
extern s_int16 vs23MaxFill;
DLLIMPORT(vs23CurrFill)
extern s_int16 vs23CurrFill;
DLLIMPORT(vs23Size)
extern s_int16 vs23Size;

#if 1
/* For actual use. Makes maximum 2073600512 byte files = 3600.001 seconds.
   Note! For 24-bit files to work (MAX_BLOCKS_PER_FILE-1)%6 must be 0. */
#define MAX_BLOCKS_PER_FILE 4050001L
#elif 1
/* For testing purposes. Makes maximum 2095616 byte files.
   Note! For 24-bit files to work (MAX_BLOCKS_PER_FILE-1)%6 must be 0. */
#define MAX_BLOCKS_PER_FILE 4093L
#else
/* For testing purposes. Makes maximum 261632 byte files.
   Note! For 24-bit files to work (MAX_BLOCKS_PER_FILE-1)%6 must be 0. */
#define MAX_BLOCKS_PER_FILE 511L
#endif
#define MAX_BYTES_PER_FILE (MAX_BLOCKS_PER_FILE*512L)


#define KEY_MASK_PLAYBACK         0x0001UL
#define KEY_MASK_PAUSE            0x0002UL
#define KEY_MASK_STOP             0x0004UL
#define KEY_MASK_RECORD           0x0008UL
#define KEY_MASK_VUMETER          0x0010UL
#define KEY_MASK_FAST_FORWARD     0x0020UL
#define KEY_MASK_FAST_REVERSE     0x0040UL
#define KEY_MASK_PREVIOUS         0x0080UL
#define KEY_MASK_NEXT             0x0100UL
#define KEY_MASK_VOLUME_DOWN      0x0200UL
#define KEY_MASK_VOLUME_UP        0x0400UL
#define KEY_MASK_UP               0x0800UL
#define KEY_MASK_DOWN             0x1000UL
#define KEY_MASK_LEFT             0x2000UL
#define KEY_MASK_RIGHT            0x4000UL
#define KEY_MASK_QUIT             0x8000UL
#define KEY_MASK_MENU         0x00010000UL
#define KEY_MASK_SELECT       0x00020000UL
#define KEY_MASK_COMMAND      0x00020000UL


#define VU_Y (128-44)
#define VLSI_Y (0)


void *decoderLibrary = NULL;
AUDIO_DECODER *audioDecoder = NULL;


/*
PCM RIFF WAV header, 0x2c = 44 bytes, or 0x16 = 22 words:
00000000  52 49 46 46 ## ## ## ##  57 41 56 45 66 6d 74 20  |RIFF$�..WAVEfmt |
00000010  10 00 00 00 01 00 CC 00  RR RR RR RR BB BB BB BB  |........@....}..|
00000020  AA 00 TT 00 64 61 74 61  ## ## ## ##              |....data.�..|

##   = 0x04 size of file
CC   = 0x16 number of channels
RR   = 0x18 rate
BB   = 0x1c bytes per sec
AA   = 0x20 align (bytes per sample x channels)
TT   = 0x22 bits per sample
##   = 0x28
Data = 0x2c

*/
#define RIFF_FILE_SIZE_OFFSET          0x04
#define RIFF_NUMBER_OF_CHANNELS_OFFSET 0x16
#define RIFF_SAMPLE_RATE_OFFSET        0x18
#define RIFF_BYTES_PER_SEC_OFFSET      0x1c
#define RIFF_ALIGN_OFFSET              0x20
#define RIFF_BITS_PER_SAMPLE_OFFSET    0x22
#define RIFF_DATA_SIZE_OFFSET          0x28
u_int16 riffWavHeader[0x16] = {
  /* Header for 24-bit 96 kHz stereo recording. */
  0x5249, 0x4646, 0xFFFF, 0xFFFF, 0x5741, 0x5645, 0x666d, 0x7420,
  0x1000, 0x0000, 0x0100, 0x0200, 0x0077, 0x0100, 0x00ca, 0x0800,
  0x0600, 0x1800, 0x6461, 0x7461, 0xFFFF, 0xFFFF
};

typedef struct DevSdSdHardwareInfo {
  u_int16 flags;
  u_int16 rca;
  u_int32 size;
  u_int32 nextBlock;
} SD_HWINFO;



enum RecMode {
  rmStop,
  rmRecord,
  rmPlayback,
  rmPause,
};


struct MiscData {
  u_int32 spaceInBlocks;
  s_int32 recordingTime;
  s_int32 playTimeTotal;
  enum RecMode recMode;
  u_int16 recPause;
  u_int32 bytesPerSec;
  char fileName[14];
  s_int16 level[4], peak[4], peakTime[4];
  s_int32 sampleRate;
  s_int16 bitsPerSample;
  u_int16 audioChannels;
  s_int16 volume;
  s_int16 fatBits;
} miscData;

#ifdef USE_BWF_FILE_LINKING
const char *linkFileString =
  "  <FILE type=\"%s\">\n"
  "    <FILENUMBER>%d</FILENUMBER>\n"
  "    <FILENAME>%s</FILENAME>\n"
  "  </FILE>\n";
#endif

#ifdef USE_LCD
u_int16 DbToPixels(register u_int16 db) {
   if (db <= 120) {
    return 128-((db*137)>>8);
  } else if (db <= 360) {
    return 96-((db*137)>>9);
  }
  return 0;
}

void UpdateLcd(register int force) {
  static u_int32 lastUpdateTime = 0;
  u_int32 now = ReadTimeCount();
  u_int16 timeSinceLastUpdate = (u_int16)(now-lastUpdateTime);
  static char s[22];
  static int lcdPhase = 0;
  int aFill = ioctl(stdaudioin, IOCTL_AUDIO_GET_INPUT_BUFFER_FILL, NULL);
  int aSize = ioctl(stdaudioin, IOCTL_AUDIO_GET_INPUT_BUFFER_SIZE, NULL);

  if (!force && (now-lastUpdateTime < TICKS_PER_SEC/160 ||
		 ((vs23CurrFill > vs23Size/2 || aFill > aSize/2) &&
		  miscData.recMode == rmRecord))) {
    return;
  }

  lcdPhase = (lcdPhase+1) & 3;

  if (force) {
    if (force > 0) {
      LcdFilledRectangle(0,0,lcd0.width-1,lcd0.height-1,NULL,lcd0.backgroundColor);
      LcdFilledRectangle(6,VU_Y,6-1+143,VU_Y+44-1,icons_vu[0],COLOR_COMPRESSED_TEXTURE);
      LcdFilledRectangle(0,VLSI_Y,160-1,VLSI_Y+30-1,icons_vlsi[0],COLOR_COMPRESSED_TEXTURE);

      if (miscData.playTimeTotal) {
	LcdFilledRectangle(8,56,lcd0.width-9,63,NULL,0xFFFF);
	lcd0.textColor = __RGB565RGB(128, 255, 128);
	LcdTextOutXY(8, 66, "0:00");
      }
    }

    lcd0.textColor = __RGB565RGB(128, 128, 255);
    LcdTextOutXY(60+3*7+2, 40, "dB");
    if (miscData.sampleRate < 32000) {
      lcd0.textColor = __RGB565RGB(240, 160, 160);
    } else if (miscData.sampleRate <= 64000) {
      lcd0.textColor = __RGB565RGB(192, 192, 192);
    }
    sprintf(s, "%dk", (int)(miscData.sampleRate/1000));
    LcdTextOutXY(4, 40, s);
    lcd0.textColor = __RGB565RGB(128, 128, 255);
    sprintf(s, "/");
    LcdTextOutXY(4+3*7, 40, s);
    sprintf(s, "%d", miscData.bitsPerSample);
    if (miscData.bitsPerSample <= 16) {
      lcd0.textColor = __RGB565RGB(192, 192, 192);
    }
    LcdTextOutXY(4+4*7, 40, s);
    lcd0.textColor = COLOR_WHITE;
  }

  if (miscData.playTimeTotal) {
    lcd0.textColor = __RGB565RGB(128, 255, 128);
  } else {
    lcd0.textColor = __RGB565RGB(255, 255, 255);
  }

 
  {
    u_int16 recSeconds, recMinutes;
    u_int16 recHours;
    u_int16 levelInPixels;
    int i;
    struct CodecServices *cs = &audioDecoder->cs;

    if (force || lcdPhase == 0) {
      u_int16 symIdx;
      if (miscData.recordingTime < 0) {
	sprintf(s, " -:--:--");
      } else {
	recHours = (u_int16)(miscData.recordingTime/3600);
	recSeconds = (u_int16)(miscData.recordingTime%3600);
	recMinutes = recSeconds/60;
	recSeconds -= recMinutes*60;
	sprintf(s, "%2d:%02d:%02d",
		recHours, recMinutes, recSeconds);
      }
      LcdTextOutXY(11, 32, s);

      symIdx = miscData.recPause ? rmPause : miscData.recMode;
      LcdFilledRectangle(2,32,2+7,32+7,icons_recsym[symIdx],0);
    }

    if (force || lcdPhase == 1) {
      LcdTextOutXY(11+7*9, 32, miscData.fileName);
    }

    if (force || lcdPhase == 2) {
      u_int32 secondsLeft;

      lcd0.textColor = __RGB565RGB(128, 128, 255);
      sprintf(s, "%3d", -miscData.volume/2);
      LcdTextOutXY(60, 40, s);

      if (miscData.playTimeTotal) {
	secondsLeft = miscData.playTimeTotal;
      } else {
	secondsLeft =
	  (u_int32)((miscData.spaceInBlocks*512.0-44.0)*
		    (1.0/(double)(miscData.sampleRate*(s_int32)((miscData.bitsPerSample>>3)*miscData.audioChannels))));
      }
      recHours = (u_int16)(secondsLeft/3600);
      recSeconds = (u_int16)(secondsLeft%3600);
      recMinutes = recSeconds/60;
      recSeconds -= recMinutes*60;

      if (miscData.playTimeTotal) {
	s_int16 totalPixels = lcd0.width-2*8-1;
	s_int16 pixels = cs ? (s_int16)(cs->Tell(cs)/(cs->fileSize/totalPixels)) : 0;

	if (pixels) {
	  LcdFilledRectangle(9,57,9+pixels-1,62,NULL,__RGB565RGB(128, 255, 128));
	}
	if (pixels < totalPixels-1) {
	  LcdFilledRectangle(9+pixels,57,lcd0.width-10,62,NULL,0);
	}
	
	if (miscData.playTimeTotal >= 999999) {
	  sprintf(s, "  -:--");
	} else {
	  sprintf(s, "%3d:%02d", recHours*60+recMinutes, recSeconds);
	}

	lcd0.textColor = __RGB565RGB(128, 255, 128);
	
	LcdTextOutXY(158-7-strlen(s)*7, 66, s);
      } else {
	if (recHours|recMinutes|recSeconds) {
	  if (!recHours && recMinutes < 5 && (recSeconds & 1) &&
	      miscData.recMode == rmRecord) {
	    strcpy(s, "      ");
	  } else {
	    sprintf(s, " %2d:%02d",
		    recHours, recMinutes); 
	  }
	} else {
	  sprintf(s, " FULL");
	}

	lcd0.textColor = __RGB565RGB(128, 128, 255);

	if (!recHours && recMinutes < 16) {
	  u_int16 red   = 128+(15-recMinutes)*16;
	  u_int16 green = recMinutes*4+64;
	  u_int16 blue  = recMinutes*12+64;
	  if (red>255) red=255;

	  lcd0.textColor = __RGB565RGB(red, green, blue);
	  //      lcd0.textColor = __RGB565RGB(128, 128, 255);
	}

	LcdTextOutXY(158-strlen(s)*7, 40, s);
      }
    }


    if (force || lcdPhase == 3) {
      for (i=0; i<2; i++) {
	int ys, ye;
	u_int16 green, orange;
	lcd0.textColor = COLOR_WHITE;

	ys = VU_Y+13+i*11;
	ye = VU_Y+19+i*11;
	green = __RGB565RGB(0, 160, 0);
	orange = __RGB565RGB(255, 160, 0);
	levelInPixels = DbToPixels(miscData.level[i]);
      
	if (!miscData.peak[i]) {
	  green = orange = __RGB565RGB(255, 64, 64);
	} else if (miscData.peak[i] <= 30) {
	  green = orange;
	}
#if 1
	LcdFilledRectangle(16, ys, 16+levelInPixels, ye, NULL, green);
	if (levelInPixels < 128) {
	  LcdFilledRectangle(16+1+levelInPixels, ys, 16+128, ye, NULL,
			     lcd0.backgroundColor);
	}
	levelInPixels = DbToPixels(miscData.peak[i]);
	if (levelInPixels > 2) {
	  LcdFilledRectangle(16-1+levelInPixels, ys, 16+levelInPixels, ye, NULL,
			     orange);
	}
#endif
#if 0
	printf("%4d->%3d ", miscData.level[i], levelInPixels);
#endif
	miscData.level[i] += 5;
      }
    }
  }
  lastUpdateTime = now;
}
#endif


void FindAudioLevels(register s_int32 *d, register u_int16 samples) {
  int i;
  u_int32 __mem_y max[2] = {0, 0};
  s_int16 maxDB10;
  u_int16 tim = (u_int16)ReadTimeCount();

  if (miscData.bitsPerSample > 16) {
#if 0
    for (i=0; i<samples; i++) {
      u_int32 t = labs(*d++);
      if (t > max[0]) max[0] = t;
      t = labs(*d++);
      if (t > max[1]) max[1] = t;
    }
#else
    FindAudioLevels32(d, max, samples);
#endif
  } else {
    u_int16 __mem_y sMax[2] = {0,0};
#if 0
    s_int16 *d2 = (s_int16 *)d;
    for (i=0; i<samples; i++) {
      u_int16 t = abs(*d2++);
      if (t > sMax[0]) sMax[0] = t;
      t = abs(*d2++);
      if (t > sMax[1]) sMax[1] = t;
    }
#else
    FindAudioLevels16((s_int16 *)d, sMax, samples);
#endif
    max[0] = (u_int32)sMax[0] << 16;
    max[1] = (u_int32)sMax[1] << 16;
  }
  for (i=0; i<4; i++) {
    u_int16 t = max[i&1] ? (s_int16)(-200.0*log(max[i&1])/log(10.0))+1866 : 2000;
    if (i < 2 && tim-miscData.peakTime[i] >= TICKS_PER_SEC*2) {
      miscData.peak[i] = t;
      miscData.peakTime[i] = tim;
    }
    if (t < miscData.level[i]) {
      miscData.level[i] = t;
    }
    if (miscData.level[i] <= miscData.peak[i]) {
      miscData.peak[i] = miscData.level[i];
      miscData.peakTime[i] = tim;
    }
  }
}





#ifdef USE_DEVBOARD_KEYS
u_int32 GetDevBoardKeysPushed(void) {
  u_int32 newKeysPushed = 0;
  static u_int32 keysOnMem = 0;
  static u_int32 lastKeysRead = 0;
  u_int32 t32 = ReadTimeCount();

  if (t32 - lastKeysRead >= TICKS_PER_SEC/25) {
    u_int32 currKeys;
    u_int16 origMode, origDDR;
    lastKeysRead = t32;
    ObtainHwLocksBIP(HLB_NONE, HLIO_NF, HLP_NONE);
    Forbid();
    origDDR = PERIP(GPIO0_DDR);
    origMode = PERIP(GPIO0_MODE);
    PERIP(GPIO0_MODE) &= ~0xf;
    PERIP(GPIO0_DDR) &= ~0xf;
    DelayMicroSec(20);
    currKeys = PERIP(GPIO0_IDATA) & 0xF;
    PERIP(GPIO0_MODE) = origMode;
    PERIP(GPIO0_DDR) = origDDR;
    Permit();
    ReleaseHwLocksBIP(HLB_NONE, HLIO_NF, HLP_NONE);

    newKeysPushed = currKeys & ~keysOnMem;

    keysOnMem = currKeys;
  }

  return newKeysPushed;
}
#endif

#ifdef USE_UART_KEYS

struct UartToKey {
  int c;
  u_int32 func;
} __mem_y uartToKey[] = {
  {'p', KEY_MASK_PLAYBACK},
  {' ', KEY_MASK_PAUSE},
  {'s', KEY_MASK_STOP},
  {'r', KEY_MASK_RECORD},
  {'v', KEY_MASK_VUMETER},
  {',', KEY_MASK_FAST_REVERSE},
  {'.', KEY_MASK_FAST_FORWARD},
  {';', KEY_MASK_PREVIOUS},
  {':', KEY_MASK_NEXT},
  {'<', KEY_MASK_VOLUME_DOWN},
  {'>', KEY_MASK_VOLUME_UP},
  {'m', KEY_MASK_MENU},
  {'l', KEY_MASK_LEFT},
  {'r', KEY_MASK_RIGHT},
  {'u', KEY_MASK_UP},
  {'d', KEY_MASK_DOWN},
  {'t', KEY_MASK_SELECT},
  {'q', KEY_MASK_QUIT},
  {'c', KEY_MASK_COMMAND},
};

u_int32 GetUartKeysPushed(void) {
  u_int32 newKeys = 0;
  while (!(newKeys & KEY_MASK_COMMAND) && ioctl(stdin, IOCTL_TEST, NULL) > 0) {
    int newKey = 0;
    int ch = fgetc(stdin);
    int i;
    struct UartToKey __mem_y *utk = uartToKey;
    for (i=0; i<sizeof(uartToKey)/sizeof(uartToKey[0]); i++) {
      if (utk->c == ch) {
	newKeys |= utk->func;
      }
      utk++;
    }
  }
  return newKeys;
}
#endif

void Convert32BitTo24Bit(register u_int16 *inP, register u_int16 n) {
  int i;
  u_int16 *outP = inP;
  for (i=0; i<n; i++) {
    /* Conversion from 32-bit VSDSP-endian to 24-bit little-endian format */
    u_int16 t;
    t = *inP++ & 0xFF00;
    *outP++ = t | (*inP & 0xFF);
    t = *inP++ & 0xFF00;
    *outP++ = t | (*inP++ >> 8);
    *outP++ = Swap16(*inP++);
  }
}

void Convert16BitTo16Bit(register u_int16 *p, register u_int16 n) {
  int i;
  n *= 2;
  for (i=0; i<n; i++) {
    /* Conversion from 16-bit VSDSP-endian to 16-bit little-endian format */
    *p = Swap16(*p);
    p++;
  }
}

ioresult GetLargestFragment(register DEVICE *dev,
			    register struct FreeFragment *frag,
			    u_int16 fast) {
  FatDeviceInfo *di = (FatDeviceInfo *)(dev->deviceInfo);
  u_int32 fatSectors=0;
  u_int32 currEmptyClusters = 0;
  s_int32 i32;
  int i;

  if (!fast) {
    /* In case SD card has been replaced: reset driver, try to create
       directory AUDIO. */
    ioctl(sdDev, IOCTL_RESTART, NULL);
    mkdir("D:AUDIO");

    memset(frag, 0, sizeof(*frag));

    miscData.fatBits = di->fatBits;
    if (di->fatBits != 32) {
      printf("FAT BITS MUST BE 32, BUT IT IS %d!\n", di->fatBits);
      miscData.spaceInBlocks = freeFrag.blocks;
      return S_ERROR;
    }

    fatSectors = di->totalClusters/128; /* Not exact: truncates end of FAT */

    for (i32=0; i32<fatSectors; i32++) {
      u_int32 *bp = (u_int32 *)outBuf;
      int j;

      /* Read the block containing allocation information */
      dev->BlockRead(dev, di->fatStart+i32, 1, outBuf);

      /* Go through free clusters, there are 128 for each 512-byte block */
      for (j=0; j<128; j++) {
	if (*bp++) {
	  if (currEmptyClusters > frag->clusters) {
	    frag->clusters = currEmptyClusters;
	    frag->firstCluster = i32*128+j-currEmptyClusters;
	  }
	  currEmptyClusters = 0;
	} else {
	  currEmptyClusters++;
	}
      }
    }
    if (currEmptyClusters > frag->clusters) {
      frag->clusters = currEmptyClusters;
      frag->firstCluster = i32*128-currEmptyClusters;
    }

    frag->firstBlock = di->dataStart +
      frag->firstCluster * di->fatSectorsPerCluster;
    frag->blocks = frag->clusters * di->fatSectorsPerCluster;
  }

  {
    u_int32 seconds;
    int hours, minutes;
	    
    seconds =
      (u_int32)((double)frag->blocks*
		(512.0/(double)(miscData.sampleRate*
				(s_int32)((miscData.bitsPerSample>>3)*miscData.audioChannels))));

    printf("Continuous space %lds", seconds);

    hours = (int)(seconds / 3600);
    seconds -= (u_int32)hours*3600;
    minutes = (int)(seconds / 60);
    seconds -= minutes*60;

    printf(" (%d:%02d:%02ld) at %ld Hz, %d bits...\n",
	   hours, minutes, seconds, miscData.sampleRate, miscData.bitsPerSample);
  }

  {
    u_int32 sizeInSectors = di->fatSectorsPerCluster * di->totalClusters;
    /* Reserve one extra cluster for each 2 GiB so that we can be sure that
       BWF information will fit even if it makes the files one cluster longer.
       Also leave space for the directory to grow, just in case. */
    u_int32 reserved = di->fatSectorsPerCluster * (3+(sizeInSectors>>22));
    if (reserved > freeFrag.blocks) {
      miscData.spaceInBlocks = 0;
    } else {
      miscData.spaceInBlocks = freeFrag.blocks - reserved;
    }
#if 0
    printf("freeFrag.blocks %lx, reserved %lx, miscData.spaceInBlocks %lx, %d\n",
	   freeFrag.blocks, reserved, miscData.spaceInBlocks, (int)(sizeInSectors>>22));
#endif
  }
  return S_OK;
}


/*
  Usage of RTC memory:
  16-17: Parameters
  18-19: File number
  20-23: StartCluster
  24-27: Blocks copy #1
  28-31: Blocks copy #2
*/
#define RTC_REC_TIME_START_SEED  0xd3398190
#define RTC_REC_TIME_BLOCKS_SEED 0xff6fee07

ioresult SetRtcStartCluster(u_int32 startCluster) {
  u_int32 crc32 =
    CalcCrc32(RTC_REC_TIME_START_SEED, (void *)(&startCluster), 4); 
  int i;
  WriteRtcMem(20, (u_int16)startCluster);
  WriteRtcMem(21, (u_int16)(startCluster>>16));
  WriteRtcMem(22, (u_int16)crc32);
  WriteRtcMem(23, (u_int16)(crc32>>16));
  /* Invalidate length datas. */
  for (i=24; i<32; i++) {
    WriteRtcMem(i, 0);
  }
  return S_OK;
}

u_int32 GetRtcStartCluster(void) {
  u_int32 startCluster =  ReadRtcMem(20) | ((u_int32)ReadRtcMem(21)<<16);
  u_int32 rtcCrc32 =  ReadRtcMem(22) | ((u_int32)ReadRtcMem(23)<<16);
  u_int32 crc32 =
    CalcCrc32(RTC_REC_TIME_START_SEED, (void *)(&startCluster), 4);

  return (crc32 == rtcCrc32) ? startCluster : 0;
  
}

ioresult SetRtcBlocks(u_int32 audioBlocks) {
  static int toggle = 0;
  u_int32 crc32 =
    CalcCrc32(RTC_REC_TIME_BLOCKS_SEED, (void *)(&audioBlocks), 4); 
  int addr = 24+toggle*4;
  WriteRtcMem(addr+0, (u_int16)audioBlocks);
  WriteRtcMem(addr+1, (u_int16)(audioBlocks>>16));
  WriteRtcMem(addr+2, (u_int16)crc32);
  WriteRtcMem(addr+3, (u_int16)(crc32>>16));
  toggle = !toggle;
}

u_int32 GetRtcBlocks(void) {
  u_int32 blocks, rtcCrc32, crc32;
  int addr;
  u_int32 blocks = 0;

  for (addr = 24; addr <= 28; addr += 4) {
    u_int32 tBlocks = ReadRtcMem(addr) | ((u_int32)ReadRtcMem(addr+1)<<16);
    rtcCrc32 =  ReadRtcMem(addr+2) | ((u_int32)ReadRtcMem(addr+3)<<16);
    crc32 = CalcCrc32(RTC_REC_TIME_BLOCKS_SEED, (void *)(&tBlocks), 4);
#if 0
    printf("#1#addr = %d, blocks = %ld, crc %08lx %08lx#\n",
	   addr, tBlocks, crc32, rtcCrc32);
#endif
    if (crc32 == rtcCrc32 && tBlocks > blocks) {
#if 0
      printf("#2#addr = %d, blocks = %ld#\n", addr, tBlocks);
#endif
     blocks = tBlocks;
    }
  }

  return blocks;  
}


#define RTC_FILE_NUMBER_SEED 0x2713ca4a

ioresult SetRtcFileNumber(u_int16 fileNumber) {
  u_int16 crc16 = (u_int16)CalcCrc32(RTC_FILE_NUMBER_SEED, (void *)(&fileNumber), 2);
  int i;
  WriteRtcMem(18, fileNumber);
  WriteRtcMem(19, crc16);
  return S_OK;
}

u_int16 GetRtcFileNumber(void) {
  u_int16 fileNumber =  ReadRtcMem(18);
  u_int16 rtcCrc16 = ReadRtcMem(19);
  u_int16 crc16 = (u_int16)CalcCrc32(RTC_FILE_NUMBER_SEED, (void *)(&fileNumber), 2);

  return (crc16 == rtcCrc16) ? fileNumber : 1;
}


#define RTC_PARAMETERS_SEED 0x19cf912d

ioresult SetRtcParameters(void) {
  struct RtcParameters rtcParameters;
  u_int16 crc16;

  rtcParameters.volume = miscData.volume/4+1;
  rtcParameters.rateIdx = (int)(miscData.sampleRate / 48000);
  rtcParameters.bits24 = miscData.bitsPerSample / 24;

  crc16 = (u_int16)CalcCrc32(RTC_FILE_NUMBER_SEED, (void *)(&rtcParameters), 2);
  WriteRtcMem(16, *((u_int16 *)(&rtcParameters)));
  WriteRtcMem(17, crc16);

  return S_OK;
}

void GetRtcParameters(void) {
  struct RtcParameters rtcParameters;
  u_int16 rtcCrc16, crc16;
  static const s_int32 sampleRates[4] = {24000, 48000, 96000, 96000/*ERROR*/};

  *((u_int16 *)&rtcParameters) = ReadRtcMem(16);
  rtcCrc16 = ReadRtcMem(17);
  crc16 = (u_int16)CalcCrc32(RTC_FILE_NUMBER_SEED, (void *)(&rtcParameters), 2);

  if (rtcCrc16 == crc16) {
    miscData.volume = (rtcParameters.volume-1)*4;
    miscData.sampleRate = sampleRates[rtcParameters.rateIdx];
    miscData.bitsPerSample = 16+8*rtcParameters.bits24;
  }
}


ioresult FatImageSectorReadNoDirty(register __i0 VO_FILE *f, u_int32 sector) {
  if (sector != f->currentSector) {
    f->currentSector = sector;
    return f->dev->BlockRead(f->dev, sector, 1, f->sectorBuffer);
  }
  return S_OK;
}


ioresult CreateFileHandle(register DEVICE *dev, register const char *name,
			  u_int32 firstCluster, u_int32 bytes,
			  u_int32 firstFreeCluster) {
  FILE *fp = NULL;
  FatFileInfo *fi = NULL;
  FatDeviceInfo *di = (FatDeviceInfo *)dev->deviceInfo;
  ioresult retVal = S_ERROR;
  u_int32 firstBlock = di->dataStart + firstCluster*di->fatSectorsPerCluster;
  u_int32 t32;

  /* Fix length data to first block. */
#if 1
  printf("Fix length to RIFF WAV headers, cluster %ld, block %ld, "
	 "aSC %ld -> %ld\n",
	 firstCluster, firstBlock, di->allocationStartCluster,
	 firstFreeCluster);
#endif
  dev->BlockRead(dev, firstBlock, 1, outBuf);
  SetLE32(outBuf, RIFF_FILE_SIZE_OFFSET, bytes- 8);
  SetLE32(outBuf, RIFF_DATA_SIZE_OFFSET, bytes-44);
  dev->BlockWrite(dev, firstBlock, 1, outBuf);

  di->allocationStartCluster = firstFreeCluster;
  /* Create file handle. */
#if 1
  printf("Create file handle, name %s, firstCluster %ld, bytes %ld\n",
	 name, firstCluster, bytes);
#endif
  fp = fopen(name, "wb");
#if 0
  printf("#1 FOPEN %p\n", fp);
#endif
  if (!fp) {
    printf("E: Can't open %s\n", name);
    goto finally;
  }
  fi = (FatFileInfo *)fp->fileInfo;

  fi->startCluster = firstCluster;
  fi->currentFragment.startSector = firstBlock;
  fp->fileSize = bytes;
  fp->pos = fp->fileSize;
  fi->currentFragment.sizeBytes = (fp->pos | 0xffffff) + 1;
  fi->currentFragment.startByteOffset = 0;
  fp->flags &= ~(__MASK_DIRTY|__MASK_UNGETC);
#if 1
  printf("File size should be %ld bytes\n", fp->fileSize);

  printf("Fixing directory entry pointer\n");
#endif
  {
    DirectoryEntry *dirEnt = (void*)&fp->sectorBuffer[fi->directoryEntryNumber*16];
    FatImageSectorReadNoDirty(fp, fi->directoryEntrySector);
    dirEnt->firstClusHi=Swap16((u_int16)(fi->startCluster>>16));
    dirEnt->firstClusLo=Swap16((u_int16)fi->startCluster);
  }

#if 1
  printf("Closing file\n");
#endif
#if 0
  printf("#1 FCLOSE %p\n", fp);
#endif


  fclose(fp);
  fp = NULL;

  retVal = S_OK;
 finally:
  return retVal;
}

/*

000c:85F0  FD A9 09 09  FF BF 3F 3F  FF BF 3F 3F  3F BF 3F 3F

002A:7FF0  7D 28 08 08  7F 3F 3F 3F  7F 3F 3F 3F  BF 3F 3F 3F

ok:
0024:C430  8D B9 07 00  8E B9 07 00  FF FF FF 0F  90 B9 07 00

*/


#if 0
ioresult PrintResults(register DEVICE *dev) {
  FatDeviceInfo *di = (FatDeviceInfo *)dev->deviceInfo;
  SD_HWINFO *hwi = (SD_HWINFO *)dev->hardwareInfo;
  u_int32 fatSectors=0;
  u_int32 currEmptyClusters = 0;
  volatile u_int32 largestEmptyClusters = 0, largestEmptyAddr = 0;
  s_int32 i32;
  int i;

  printf("DevSdSdHardwareInfo:\n");
  printf("flags 0x%04x, rca 0x%04x, size 0x%08lx, nextBlock 0x%08lx\n",
	 hwi->flags, hwi->rca, hwi->size, hwi->nextBlock);

  printf("FatDeviceInfo:\n");
  printf("rootEntCnt %d, numFats %d\n",
	 di->rootEntCnt, di->numFats);
  printf("totalClusters %ld, fatSectorsPerCluster %d, fatBits %d\n",
	 di->totalClusters, di->fatSectorsPerCluster, di->fatBits);
  printf("fatStart %ld, dataStart %ld\n",
	 di->fatStart, di->dataStart);
  printf("rootStart %ld, allocationStartCluster %ld\n",
	 di->rootStart, di->allocationStartCluster);

  if (di->fatBits != 32) {
    printf("FAT BITS MUST BE 32!\n");
    return S_ERROR;
  }

  fatSectors = di->totalClusters/128; /* Not exact: truncates end of FAT */
  printf("fatSectors %ld\n", fatSectors);

  for (i32=0; i32<fatSectors; i32++) {
    u_int32 *bp = (u_int32 *)outBuf;
    int j;
    dev->BlockRead(dev, di->fatStart+i32, 1, outBuf);
#if 1
    if (i32 < 1) {
      int i;
      printf("%ld:\n", di->fatStart+i32);
      for (i=0; i<128; i+=8) {
	int j;
	for (j=0; j<8; j++) {
	  printf("%08lx ", Swap32Mix(bp[i+j]));
	}
	printf("\n");
      }
    }
#endif
    for (j=0; j<128; j++) {
      if (*bp++) {
	if (currEmptyClusters) {
	  u_int32 startAddr = i32*128+j-currEmptyClusters;
	  printf("  Empty from %9ld, %8ld clusters = %8.1f MiB\n",
		 startAddr, currEmptyClusters,
		 (double)
		 (currEmptyClusters*di->fatSectorsPerCluster)*(0.5/1024.0));
	  if (currEmptyClusters > largestEmptyClusters) {
	    largestEmptyClusters = currEmptyClusters;
	    largestEmptyAddr = startAddr;
	  }
	  currEmptyClusters = 0;
	}
      } else {
	currEmptyClusters++;
      }
    }
  }
  if (currEmptyClusters) {
    u_int32 startAddr = i32*128-currEmptyClusters;
    printf("  Empty from %9ld, %8ld clusters = %8.1f MiB\n",
	   startAddr, currEmptyClusters,
	   (double)
	   (currEmptyClusters*di->fatSectorsPerCluster)*(0.5/1024.0));

    if (currEmptyClusters > largestEmptyClusters) {
      largestEmptyClusters = currEmptyClusters;
      largestEmptyAddr = startAddr;
    }
    currEmptyClusters = 0;
  }

  printf("Largest from %9ld, %8ld clusters = %8.1f MiB\n",
	 largestEmptyAddr, largestEmptyClusters,
	 (double)
	 (largestEmptyClusters*di->fatSectorsPerCluster)*(0.5/1024.0));


  return S_OK;
}
#endif


s_int16 BuildFileEntries(register DEVICE *dev, register s_int16 rescueMode) {
  u_int32 firstCluster = GetRtcStartCluster();
  u_int32 audioBlocks = GetRtcBlocks();
  FatDeviceInfo *di = (FatDeviceInfo *)dev->deviceInfo;
  u_int32 t32, audioBlocksInThisFile;
  s_int16 fileNumber;
  u_int16 t, padBlocks;
  s_int16 entries, entry=0;
  u_int32 startTime = ReadTimeCount();

#if 0
  /* Test code to see how fast finalizing is. */
  firstCluster = 66984;
  audioBlocks = 67093824;
#endif

#ifdef USE_LCD
  UpdateLcd(1);
#endif

  t = (u_int16)(MAX_BLOCKS_PER_FILE & (di->fatSectorsPerCluster - 1));
  if (t) {
    padBlocks = di->fatSectorsPerCluster-t;
  }


#if 0
  printf("audioBlocks %ld\n", audioBlocks);
#endif
  if (!firstCluster || !audioBlocks) {
    if (!rescueMode) {
      printf("No file entries to build.\n");
    }
    return 0;
  }

  if (rescueMode) {
    printf("WARNING! Found unfinished file entries from previous run!\n"
	   "Activating RESCUE MODE!\n");
  } else {
    printf("\nFinalizing file entries.\n");
  }
  printf("MAKE SURE THE DEVICE IS NOT POWERED OFF WHILE\n"
	 "FILE ENTRIES ARE BEING BUILT!!!\n");

  t32 = 0;
  fileNumber = GetRtcFileNumber();
#if 0
  printf("fileNumber@1 = %d\n", fileNumber);
#endif
  di->allocationStartCluster = firstCluster
    + audioBlocks/di->fatSectorsPerCluster + 1;
#if 0
  printf("###### firstClus %ld, allocStartClus %ld\n",
	 firstCluster, di->allocationStartCluster);
#endif
  while (t32 < audioBlocks) {
    u_int16 addExtra = 0;
    audioBlocksInThisFile = MAX_BLOCKS_PER_FILE;
    if (audioBlocksInThisFile > audioBlocks-t32) {
      audioBlocksInThisFile = audioBlocks-t32;
      audioBlocksInThisFile -= (audioBlocksInThisFile-1)%6;
      addExtra = 6;
    }
    if (audioBlocksInThisFile > 0) {
      static char name[22], oneLine[23];
      double seconds = (audioBlocksInThisFile*512L-44L)*
	(1.0/(double)(miscData.sampleRate*(s_int32)((miscData.bitsPerSample>>3)*miscData.audioChannels)));
      sprintf(name, "D:AUDIO/AUD%05u.WAV", fileNumber);
      printf("Creating file entry for %s, length %7.2f seconds\n",
	     name, seconds);
#ifdef USE_LCD
      /*                      1234567890123456789012*/
      LcdTextOutXY(  0,  48, "Creating file entry:");
      LcdTextOutXY(  0,  56, name);
      sprintf(oneLine, "%2.2f seconds", seconds);
      LcdTextOutXY(  0,  64, oneLine);
#endif

      CreateFileHandle(dev, name,
		       firstCluster + t32/di->fatSectorsPerCluster,
		       audioBlocksInThisFile*512,
		       firstCluster + audioBlocks/di->fatSectorsPerCluster + 1);

      t32 += audioBlocksInThisFile + padBlocks + addExtra;
      fileNumber++;
      entry++;
    }
  }

  SetRtcFileNumber(fileNumber);
  SetRtcStartCluster(0);
  entries = entry;

  printf("File %u, entries %d\n", fileNumber, entries);

#ifdef USE_BWF_FILE_LINKING
  {
    u_int32 id = time(NULL) | BitReverse32(ReadTimeCount());
    fileNumber -= entries;
    entry = 0;
    for (entry = 0; entry < entries; entry++) {
      FILE *fp;
      static char name[22];

      sprintf(name, "D:AUDIO/AUD%05u.WAV", fileNumber+entry);

      if (fp = fopen(name, "ab")) {
	int i;
	u_int32 bytes;
	u_int32 linkPos = ftell(fp);
	u_int16 size[2];
	printf("Add BWF data for %s\n", name);

#ifdef USE_LCD
	/*                      1234567890123456789012*/
	LcdTextOutXY(  0,  48, "Update BWF data:");
	LcdTextOutXY(  0,  56, name);
	LcdTextOutXY(  0,  64, "              ");
#endif

	fprintf(fp, "linkXXXX<LINK>\n");

	for (i=0; i<entries; i++) {
	  sprintf(name, "AUD%05u.WAV", fileNumber+i);
	  fprintf(fp, linkFileString, (i==entry) ? "actual" : "other",
		  i+1, name);
	}
	fprintf(fp, "  <ID>%010lu</ID>\n</LINK>\n", id);
	if (ftell(fp) & 1) fputc('\n', fp);
	bytes = ftell(fp)-linkPos;
	SetLE32(size, 0, bytes-8);
	fseek(fp, linkPos+4, SEEK_SET);
	fwrite(size, 2, 1, fp);
	fseek(fp, RIFF_FILE_SIZE_OFFSET, SEEK_SET);
	fread(size, 2, 1, fp);
	bytes += GetLE32(size, 0);
	SetLE32(size, 0, bytes);
	fseek(fp, RIFF_FILE_SIZE_OFFSET, SEEK_SET);
	fwrite(size, 2, 1, fp);

	fclose(fp);
      }

    }
  }
#endif /* USE_BWF_FILE_NUMBERING */

  printf("Finalizing took %3.1f seconds.\n",
	 0.001*(double)(ReadTimeCount()-startTime));

  /* Force flushing by reading one block. */
  dev->BlockRead(dev, 0, 1, outBuf);

#ifdef USE_LCD
  LcdFilledRectangle(0,48,lcd0.width-1,48-1+24,NULL,lcd0.backgroundColor);
#endif


  return entries;
}






s_int16 SetRateAndBits()  {
  s_int32 sampleRateAndBits = miscData.sampleRate * ((miscData.bitsPerSample > 16) ? -1 : 1);
  static u_int32 bpsTab[2][3] = {
    { 96000, 192000, 384000},
    {144000, 288000, 576000}
  };

  miscData.bytesPerSec =
    bpsTab[miscData.bitsPerSample > 16][(miscData.sampleRate > 24000) + (miscData.sampleRate > 48000)];

  SetLE32(riffWavHeader, RIFF_SAMPLE_RATE_OFFSET,     miscData.sampleRate);
  SetLE32(riffWavHeader, RIFF_BYTES_PER_SEC_OFFSET,   miscData.sampleRate*(s_int32)(miscData.bitsPerSample>>3));
  SetLE16(riffWavHeader, RIFF_ALIGN_OFFSET,           (miscData.bitsPerSample>>2));
  SetLE16(riffWavHeader, RIFF_BITS_PER_SAMPLE_OFFSET, miscData.bitsPerSample);

  if (!stdaudioin ||
      ioctl(stdaudioin, IOCTL_AUDIO_SET_RATE_AND_BITS,
	    (void *)(&sampleRateAndBits)) ||
      ioctl(stdaudioout, IOCTL_AUDIO_SET_RATE_AND_BITS,
	    (void *)(&sampleRateAndBits))) {
    printf("Couldn't set audio\n");
    return -1;
  }
  return 0;
}





#ifdef USE_EXT1_BOARD_KEYS

#define ROTARY_UP       0x10
#define ROTARY_RIGHT    0x08
#define ROTARY_DOWN     0x04
#define ROTARY_LEFT     0x02
#define ROTARY_CENTER	0x01

int rotaryPos = 0, newRotaryButtons = 0;

extern struct CyclicNode checkRotaryNode;

void CheckRotary(register struct CyclicNode *cyclicNode) {
  static int history[2] = {-1,-1}, i = 0;
#if 0
  static int oldPos = 0;
#endif
  static int but = 0, oldBut = 0;
  static int phase = 0;
#if 0
  int aFill = ioctl(stdaudioin, IOCTL_AUDIO_GET_INPUT_BUFFER_FILL, NULL);
  int bSize = ioctl(stdaudioin, IOCTL_AUDIO_GET_INPUT_BUFFER_SIZE, NULL);

  if (aFill > bSize/2) {
    /* Lighten load from cyclic if audio buffer is too full. */
    printf("#CYC#\n");
    cyclicNode->interval = TICKS_PER_SEC/20;
  } else {
    cyclicNode->interval = TICKS_PER_SEC/200;
  }
#endif

#if 1
  if (!AttemptHwLocksBIP(HLB_NONE, HLIO_JTAG_B, HLP_NONE)) {
    int in;
    PERIP(GPIO2_MODE) &= ~(3<<3);
    PERIP(GPIO2_DDR) &= ~(3<<3);
    DelayMicroSec(10);
    in = (PERIP(GPIO2_IDATA) >> 3) & 3;
    ReleaseHwLocksBIP(HLB_NONE, HLIO_JTAG_B, HLP_NONE);
    if (in != history[0]) {
#if 0
      printf("%3d %d %d %d\n", i, history[1], history[0], in);
#endif
      if ((history[1] == 3 && history[0] == 2 && in == 0) ||
	  (history[1] == 0 && history[0] == 1 && in == 3)) {
#if 0
	printf("  RIGHT\n");
#endif
	rotaryPos++;
      }
      if ((history[1] == 0 && history[0] == 2 && in == 3) ||
	  (history[1] == 3 && history[0] == 1 && in == 0)) {
#if 0
	printf("  LEFT\n");
#endif
	rotaryPos--;
      }
#if 0
      if (rotaryPos != oldPos) {
	printf("pos %2d\n", rotaryPos);
	oldPos = rotaryPos;
      }
#endif
      history[1] = history[0];
      history[0] = in;
    } /* if (in != history[0]) */
  } /* if (!AttemptHwLocksBIP(...)) */
#endif
#if 1
  if (!AttemptHwLocksBIP(HLB_NONE, HLIO_CS_SELECT|HLIO_NF, HLP_NONE)) {
    int but;
    u_int16 origMode, origDDR;
    Forbid();
    origDDR = PERIP(GPIO0_DDR);
    origMode = PERIP(GPIO0_MODE);
    PERIP(GPIO0_MODE) &= ~0x1f;
    PERIP(GPIO0_DDR) &= ~0x1f;
    SELECT_IO_CHANNEL(IOCHAN_DEV_EXT4);
    DelayMicroSec(10);
    /* bits: 4:Left, 3:Up, 2:Right, 1:Down, 0:Center */
    but = (PERIP(GPIO0_IDATA) & 0x1f) ^ 0x1f;
    newRotaryButtons |= but & ~oldBut;
    SELECT_IO_CHANNEL(IOCHAN_DEV_IDLE);
    PERIP(GPIO0_MODE) = origMode;
    PERIP(GPIO0_DDR) = origDDR;
    Permit();
    ReleaseHwLocksBIP(HLB_NONE, HLIO_CS_SELECT|HLIO_NF, HLP_NONE);

#if 0
    if (but != oldBut) {
      printf("\n%05b %ld %ld %ld\n",
	     but,
	     checkRotaryNode.interval,
	     checkRotaryNode.nextActivation,
	     ReadTimeCount());
    }
#endif
    oldBut = but;
  } /* if (!AttemptHwLocksBIP(...)) */
#endif
}

struct CyclicNode checkRotaryNode = {{0}, CheckRotary};
#endif


enum MenuType {
  mtEnd,
  mtAction,
  mtFunction,
  mtText,
  mtToggle,
  mtSlider,
};

struct MenuItem {
  enum MenuType type;
  char *label;
  void *unitOrDest;
  s_int16 value;
  s_int16 modified;
  s_int16 digits, min, step, max;
  s_int16 x;
};

struct Menu {
  char *title;
  struct MenuItem *items;
};

struct MenuItem qualityMenuItems[] = {
  /* type    label            unit/dest  val md dg   min step  max */
  {mtAction, "<-- Back" },
  {mtText  , "Sample rate"  },
  {mtToggle, "  96 kHz"  , &qualityMenuItems[3],   0, 0, 0,    0,   1,   1},
  {mtToggle, "  48 kHz"  , &qualityMenuItems[4],   0, 0, 0,    0,   1,   1},
  {mtToggle, "  24 kHz"  , &qualityMenuItems[2],   0, 0, 0,    0,   1,   1},
  {mtText  , "Sample resolution"},
  {mtToggle, "  24 bits" , &qualityMenuItems[7],   0, 0, 0,    0,   1,   1},
  {mtToggle, "  16 bits" , &qualityMenuItems[6],   0, 0, 0,    0,   1,   1},
  {mtEnd}
};

struct Menu qualityMenu = {
  "Quality",
  qualityMenuItems
};

struct MenuItem versionMenuItems[] = {
  /* type    label            unit/dest  val md dg   min step  max */
  {mtAction, "<-- Back" },
  {mtText  , VERSION1   },
  {mtText  , VERSION2   },
  {mtEnd}
};

struct Menu versionMenu = {
  "Version",
  versionMenuItems
};

s_int16 Format(void) {
  static char param[17];
  strcpy(param, "-y -lHIRESREC D:");

#ifdef USE_LCD
  LcdFilledRectangle(4,4,lcd0.width-1-4,lcd0.height-1-4,NULL,lcd0.backgroundColor);

  LcdTextOutXY(45, 45, "FORMATTING");
  LcdTextOutXY(42, 65, "PLEASE WAIT");
#endif
  printf("FORMATTING... PLEASE WAIT\n");

  RunProgram("FORMAT", param);

  GetLargestFragment(sdDev, &freeFrag, 0);

  return -1;
}

struct MenuItem formatMenuItems[] = {
  /* type    label            unit/dest  val md dg   min step  max */
  {mtAction  , "<-- Back" },
  {mtText    , "" },
  {mtText    , "Format SD Card?" },
  {mtText    , "WARNING:" },
  {mtText    , "All data will be" },
  {mtText    , "permanently lost!" },
  {mtText    , "" },
  {mtFunction, "  Yes" , Format},
  {mtAction,   "  No"},
  {mtEnd}
};

struct Menu formatMenu = {
  "Format SD Card",
  formatMenuItems
};

struct MenuItem mainMenuItems[] = {
  /* type    label            unit  dest val md dg   min step  max */
  {mtAction, "<-- Exit" },
  {mtAction, "Quality -->"  ,  &qualityMenu},
  {mtSlider, "Monitor"      ,      "dB", -20, 0, 3,  -40,   2,   0},
  {mtAction, "Format  -->"  ,  &formatMenu},
  {mtAction, "Version -->"  ,  &versionMenu},
#if 0
  {mtToggle, "On/Off 1"       ,    NULL,   0},
  {mtToggle, "On/Off 2"       ,    NULL,   1},
#endif
  {mtEnd}
};

struct Menu mainMenu = {
  "Main Menu",
  mainMenuItems
};


struct MenuItem format2MenuItems[] = {
  /* type    label            unit/dest  val md dg   min step  max */
  {mtAction  , "<-- Exit" },
  {mtText    , "" },
  {mtText    , "Card is not FAT32!" },
  {mtText    , "Format SD Card?" },
  {mtText    , "WARNING:" },
  {mtText    , "All data will be" },
  {mtText    , "permanently lost!" },
  {mtText    , "" },
  {mtFunction, "  Yes" , Format},
  {mtAction,   "  No"},
  {mtEnd}
};

struct Menu format2Menu = {
  "Format Menu",
  format2MenuItems
};

#define PIXELS_PER_LINE 10
#define MENU_LINES 11
s_int16 UpdateMenu(register struct Menu *mn, register u_int16 firstItem, register u_int16 activeItem, register s_int16 force) {
  int item=0;
  struct MenuItem *mi = mn->items;

#ifdef USE_LCD
  if (force) {
    LcdTextOutXY((lcd0.width-strlen(mn->title)*7)/2, 5, mn->title);
  }
#endif
  printf("@t@%s@%x@@\n", mn->title, mn);

  while (mi->type != mtEnd) {
    int mustRender = 0;
    if (item >= firstItem && item < firstItem+MENU_LINES) {
      if (force < 0 ||
	  (force & 0xFF) == item || ((force>>8) & 0xFF) == item) {
	mustRender = 1;
      }
    }
    if (mustRender) {
#ifdef USE_LCD
      int x = 5;
      int y = 4+(item-firstItem+1)*PIXELS_PER_LINE;
#endif
      u_int16 t;
      static char s[16];
#ifdef USE_LCD
      if (item == activeItem) {
	t = lcd0.backgroundColor;
	lcd0.backgroundColor = lcd0.textColor;
	lcd0.textColor = t;
      }
      LcdTextOutXY(x, y+1, mi->label);
#endif
      printf("@i%02d%c@%s", item, item==activeItem?'+':'-', mi->label);
#ifdef USE_LCD
      x += (strlen(mi->label))*7;
#endif
      if (mi->digits) {
	int l;
	char *p = s+8;
	memset(s, ' ', sizeof(s));
	sprintf(p, " %d", mi->value);
#ifdef USE_LCD
	LcdTextOutXY(x, y+1, p+strlen(p)-mi->digits-1);
	x += (mi->digits+1)*7;
#endif
	printf("@v%d:%d:%d", mi->min,mi->value,mi->max);
	if (mi->unitOrDest) {
#ifdef USE_LCD
	  LcdTextOutXY(x, y+1, " ");
	  x += 4;
	  LcdTextOutXY(x, y+1, mi->unitOrDest);
	  x += (strlen(mi->unitOrDest))*7;
#endif
	}
      }
#ifdef USE_LCD
      if (item == activeItem) {
	t = lcd0.backgroundColor;
	lcd0.backgroundColor = lcd0.textColor;
	lcd0.textColor = t;
      }
#endif
      if (mi->type != mtSlider) {
#ifdef USE_LCD
	LcdFilledRectangle(x,y+1,lcd0.width-1-5-((mi->type==mtToggle)?15:0),y+8,NULL,
			   (item==activeItem) ?
			   COLOR_WHITE : lcd0.backgroundColor);
#endif
	if (mi->type == mtToggle) {
#ifdef USE_LCD
	  LcdFilledRectangle(lcd0.width-5-15,y+1,lcd0.width-1-5,y+8,NULL,
			     lcd0.backgroundColor);

#endif
	}
      }
      if (mi->type != mtToggle) {
#ifdef USE_LCD
	LcdFilledRectangle(x,y,lcd0.width-1-5,y,NULL,lcd0.backgroundColor);
#endif
      }
      if (mi->type == mtToggle) {
#ifdef USE_LCD
	LcdFilledRectangle(lcd0.width-10-7, y, lcd0.width-10, y+7,icons_recsym[4+mi->value],0);
#endif
	printf("@t%d", mi->value);
      } else if (mi->type == mtSlider) {
#ifdef USE_LCD
	int xEnd = lcd0.width-1-6;
	int dataEnd;
	int fieldWidth;
	x += 4;
	if (force < 0) LcdFilledRectangle(x, y+1, xEnd, y+8, NULL, 0xFFFF);
	x++;
	xEnd--;
	dataEnd = x+(int)((mi->value-mi->min)/(double)(mi->max-mi->min)*(1+xEnd-x)+0.5);
	if (dataEnd > x) {
	  LcdFilledRectangle(x, y+2, dataEnd-1, y+7, NULL, __RGB565RGB(160, 255, 30));
	}
	if (dataEnd <= xEnd) {
	  LcdFilledRectangle(dataEnd, y+2, xEnd, y+7, NULL, lcd0.backgroundColor);

	}

	fieldWidth = xEnd-x-1;
#endif
      }
      printf("@@\n");
    }
    if (mi->type != mtEnd) {
      mi++;
      item++;
    }
  }
#ifdef USE_LCD
  if (force && item < firstItem+MENU_LINES) {
    int y = 4+(item-firstItem+1)*PIXELS_PER_LINE;

    LcdFilledRectangle(5,y,lcd0.width-1-5,3+MENU_LINES*PIXELS_PER_LINE,NULL,
		       lcd0.backgroundColor);
      
  }
#endif
  printf("@E@@\n");

  return item;
}

s_int16 ShowMenu(register struct Menu *mn) {
  int activeItem = 0;
  int firstItem = 0;
#ifdef USE_EXT1_BOARD_KEYS
  int oldRotaryPos=rotaryPos;
#endif
  int menuItems;
  int quit=0, retVal=0;
  u_int32 newKeysPushed = 0;

  printf("@M@%04x@@\n", mn);

#ifdef USE_LCD
  lcd0.textColor = COLOR_WHITE;

  LcdFilledRectangle(3,3,lcd0.width-1-3,lcd0.height-1-3,NULL,COLOR_WHITE);
  LcdFilledRectangle(4,4,lcd0.width-1-4,lcd0.height-1-4,NULL,lcd0.backgroundColor);
#endif

  menuItems = UpdateMenu(mn, firstItem, activeItem, -1);
  while (!quit) {
    struct MenuItem *mi = mn->items+activeItem;
#ifdef USE_EXT1_BOARD_KEYS
    int newRotaryPos = rotaryPos;
#endif
    int update = 0, fullUpdate = 0;
    int oldActiveItem = activeItem;
    int itemDirection = 0;

#ifdef USE_UART_KEYS
    newKeysPushed |= GetUartKeysPushed();
#endif

    fread(inBuf, 1, IN_BUF_SIZE_32BIT, stdaudioin);
    fwrite(inBuf, 1, IN_BUF_SIZE_32BIT, stdaudioout);

    if (mainMenuItems[2].modified) {
      miscData.volume = -2*mainMenuItems[2].value;
      mainMenuItems[2].modified = 0;
      ioctl(stdaudioout, IOCTL_AUDIO_SET_VOLUME, (void *)(miscData.volume+256));
      printf("~%04x=%d\n", UIMSG_S16_SET_VOLUME, miscData.volume);
    }

#ifdef USE_EXT1_BOARD_KEYS
    if (newRotaryButtons & ROTARY_UP || newRotaryPos < oldRotaryPos) {
      newRotaryButtons &= ~ROTARY_UP;
      oldRotaryPos = newRotaryPos;
      newKeysPushed |= KEY_MASK_UP;
    }
    if (newRotaryButtons & ROTARY_DOWN || newRotaryPos > oldRotaryPos) {
      newRotaryButtons &= ~ROTARY_DOWN;
      oldRotaryPos = newRotaryPos;
      newKeysPushed |= KEY_MASK_DOWN;
    }
    if (newRotaryButtons & ROTARY_LEFT) {
      newRotaryButtons &= ~ROTARY_LEFT;
      newKeysPushed |= KEY_MASK_LEFT;
    }
    if (newRotaryButtons & ROTARY_RIGHT) {
      newRotaryButtons &= ~ROTARY_RIGHT;
      newKeysPushed |= KEY_MASK_RIGHT;
    }
    if (newRotaryButtons & ROTARY_CENTER) {
      newRotaryButtons &= ~ROTARY_CENTER;
      newKeysPushed = KEY_MASK_SELECT;
    }
#endif

    if (newKeysPushed & (KEY_MASK_QUIT | KEY_MASK_MENU)) {
      quit = 1;
      retVal |= (newKeysPushed & KEY_MASK_QUIT) ? 1 : 0;
    } else if (newKeysPushed & KEY_MASK_UP) {
      newKeysPushed &= ~KEY_MASK_UP;
      update = 1;
      itemDirection = -1;
    } else if (newKeysPushed & KEY_MASK_DOWN) {
      newKeysPushed &= ~KEY_MASK_DOWN;
      update = 1;
      itemDirection = 1;
    } else if (newKeysPushed & KEY_MASK_SELECT) {
      newKeysPushed &= ~KEY_MASK_SELECT;
      if (mi->type == mtToggle) {
	newKeysPushed |= KEY_MASK_RIGHT;
      } else if (mi->type == mtAction) {
	if (mi->unitOrDest) {
	  if (ShowMenu(mi->unitOrDest)) {
	    newKeysPushed |= KEY_MASK_QUIT;
	  }
#ifdef USE_EXT1_BOARD_KEYS
	  oldRotaryPos = rotaryPos;
#endif
	  newKeysPushed &= KEY_MASK_QUIT;
	  UpdateMenu(mn, firstItem, activeItem, -1);
	} else {
	  quit = 1;
	}
      } else if (mi->type == mtFunction) {
	if (mi->unitOrDest) {
	  s_int16 (*func)(void) = mi->unitOrDest;
	  s_int16 res = func();
	  if (!res) {
	    /* res = 0 -> continue at same menu level */
#ifdef USE_EXT1_BOARD_KEYS
	    oldRotaryPos = rotaryPos;
#endif
	    UpdateMenu(mn, firstItem, activeItem, -1);
	  } else {
	    /* res != 0 -> go up one level */
	    quit = 1;
	  }
	}
      }
    } else if (newKeysPushed & KEY_MASK_LEFT) {
      newKeysPushed &= ~KEY_MASK_LEFT;
      if (mi->type == mtSlider) {
	update = -1;
	if ((mi->value -= mi->step) < mi->min) {
	  mi->value = mi->min;
	}
	mi->modified = 1;
      } else {
	quit = 1;
      }
    } else if (newKeysPushed & KEY_MASK_RIGHT) {
      newKeysPushed &= ~KEY_MASK_RIGHT;
      if (mi->type == mtAction || mi->type == mtFunction) {
	newKeysPushed |= KEY_MASK_SELECT;
      } else if (mi->type == mtToggle || mi->type == mtSlider) {
	update = -1;
	if (mi->type == mtToggle) {
	  if (mi->unitOrDest) {
	    struct MenuItem *mi2 = mi->unitOrDest;
	    mi->value = 1;
	    while (mi2 != mi) {
	      mi2->value = 0;
	      mi2->modified = 1;
	      mi2 = mi2->unitOrDest;
	    }
	    fullUpdate = 1;
	  } else {
	    mi->value = !mi->value;
	  }
	} else if ((mi->value += mi->step) > mi->max) {
	  mi->value = mi->max;
	}
	mi->modified = 1;
      }
    }

    if (itemDirection < 0) {
      if (activeItem > 0) {
	do {
	  activeItem--;
	  mi--;
	} while (activeItem > 0 && mi->type == mtText);
      }
    }
    if (itemDirection > 0) {
      if (activeItem < menuItems-1) {
	do {
	  activeItem++;
	  mi++;
	} while (activeItem < menuItems-1 && mi->type == mtText);
      }
    }
	  
    if (activeItem == oldActiveItem && update > 0) {
      update = 0;
    }

    if (update) {
      s_int16 force = -1;
      if (update > 0) {
	force = activeItem | (oldActiveItem<<8);
      } else {
	force = activeItem*0x101;
      }
      if (activeItem < firstItem) {
	firstItem = activeItem;
	force = -1;
      } else if (activeItem >= firstItem + MENU_LINES) {
	firstItem = activeItem-(MENU_LINES-1);
	force = -1;
      } else if (fullUpdate) {
	force = -1;
      }
      menuItems = UpdateMenu(mn, firstItem, activeItem, force);
      update = 0;
    }
  }
#ifdef USE_EXT1_BOARD_KEYS
  newRotaryButtons = 0;
#endif

  printf("@E@%04x@%x@@\n", mn, retVal);

  return retVal;
}


#define NAMELENGTH 80
#define MAX_PLAY_FILES 2048
char *errorString = "";
int eCode = 0;
s_int16 wordsPerSample = 1;
u_int16 firstOutput = 0;

void PlayerThread(void) {
  eCode = DecodeAudio(decoderLibrary, audioDecoder, &errorString);
}

int CompareUInt16(const void *p1, const void *p2) {
  u_int16 d1 = *((u_int16 *)p1);
  u_int16 d2 = *((u_int16 *)p2);
  if (d1 < d2) return -1;
  return (d1 > d2);
}

s_int16 (*originalOutput)(struct CodecServices *cs, s_int16 *data, s_int16 n);

s_int16 MyOutput(struct CodecServices *cs, s_int16 *data, s_int16 n) {
  if (n<0) {
    miscData.bitsPerSample = 24;
    FindAudioLevels((s_int32 *)data, -n);
  } else if (n > 0) {
    miscData.bitsPerSample = 16;
    FindAudioLevels((s_int32 *)data, n);
  }
  if (n && firstOutput) {
    firstOutput = 0;
    miscData.sampleRate = cs->sampleRate;
    SetRateAndBits();
  }
  originalOutput(cs, data, n);
}

void ModifyVolume(register u_int32 *newKeysPushed
#ifdef USE_EXT1_BOARD_KEYS
		  , register s_int16 *oldRotaryPos
#endif
		  ) {
  int addVol = 0;

#ifdef USE_EXT1_BOARD_KEYS
  s_int16 newRotaryPos = rotaryPos;

  if (newRotaryPos != *oldRotaryPos) {
    addVol = (*oldRotaryPos-newRotaryPos)*4;
    *oldRotaryPos = newRotaryPos;
  }
#endif
  if (*newKeysPushed & KEY_MASK_VOLUME_DOWN) {
    *newKeysPushed &= ~KEY_MASK_VOLUME_DOWN;
    addVol = 4;
  }
  if (*newKeysPushed & KEY_MASK_VOLUME_UP) {
    *newKeysPushed &= ~KEY_MASK_VOLUME_UP;
    addVol = -4;
  }

  if (addVol) {
    miscData.volume += addVol;
    if (miscData.volume < 0) {
      miscData.volume = 0;
    } else if (miscData.volume > 40*2) {
      miscData.volume = 40*2;
    }
    ioctl(stdaudioout, IOCTL_AUDIO_SET_VOLUME, (void *)(miscData.volume+256));
    printf("~%04x=%d\n", UIMSG_S16_SET_VOLUME, miscData.volume);
    SetRtcParameters();
  }
}

u_int16 audioFiles = 0;
static u_int16 audioFileNumber[MAX_PLAY_FILES];

void FindAudioFiles(void) {
  FILE *fp = fopen("D:AUDIO/.", "s");
  static char fileName[NAMELENGTH];
  audioFiles = 0;

  if (!fp) return;

  if (FatFindFirst(fp, "AUDIO/.", fileName, NAMELENGTH) == S_OK) {
    do {
      fileName[NAMELENGTH-1] = '\0';
      if (!memcmp(fileName, "AUD", 3) && !strcmp(fileName+8, ".WAV")) {
	audioFileNumber[audioFiles++] = (u_int16)strtol(fileName+3, NULL, 10);
      }
    } while (audioFiles < MAX_PLAY_FILES &&
	     FatFindNext(fp, fileName, NAMELENGTH) == S_OK);
  }
  fclose(fp);
  fp = NULL;

  /* Sort the file numbers */
  qsort(audioFileNumber, audioFiles, sizeof(audioFileNumber[0]), CompareUInt16);
}

void Playback(register u_int16 playFileNumber) {
  int quit = 0;
  FatDeviceInfo *di = NULL;
  FILE *fp;
  int i;
  int currFileIdx = 0;
  s_int32 lastSeconds = -1, newSeconds;
  s_int16 originalBitsPerSample = miscData.bitsPerSample;
  s_int32 originalSampleRate = miscData.sampleRate;
  u_int32 newKeysPushed = 0;
#ifdef USE_EXT1_BOARD_KEYS
  s_int16 oldRotaryPos = rotaryPos;
#endif
  struct CodecServices *cs = NULL;
  int clear = 1;

  miscData.recMode = rmPlayback;
  errorString = "";


  di = (FatDeviceInfo *)sdDev->deviceInfo;

  FindAudioFiles();
  if (!audioFiles) {
    goto finally;
  }

  /* The last one will be our current file. */
  currFileIdx = audioFiles-1;

  decoderLibrary = LoadLibrary("audiodec");
  if (!decoderLibrary) {
    printf("!decLib\n");
    goto finally;
  }

#ifdef USE_EXT1_BOARD_KEYS
  newRotaryButtons = 0;
#endif

  while (!quit) {
    s_int16 lastPerCent = -1;
    sprintf(miscData.fileName, "AUD%05u.WAV", audioFileNumber[currFileIdx]);
    sprintf(tStr, "D:AUDIO/%s", miscData.fileName);

    miscData.recordingTime = -1;
    miscData.playTimeTotal = 999999;
#ifdef USE_LCD
    UpdateLcd(clear);
#endif
    clear = -1;

    printf("~%04x=%d\n", UIMSG_S16_SONG_NUMBER, currFileIdx+1);
    printf("~%04x'%s\n", UIMSG_TEXT_SHORT_FILE_NAME, miscData.fileName);

    fp = fopen(tStr, "rb");
    if (!fp) {
      printf("!openFile\n");
      goto finally;
    }

    audioDecoder =
      CreateAudioDecoder(decoderLibrary, fp, stdaudioout, NULL, auDecFGuess);

    if (!audioDecoder) {
      printf("!createAuDec\n");
      goto finally;
    }

    cs = &audioDecoder->cs;
    originalOutput = cs->Output;
    cs->Output = MyOutput;
    firstOutput = 1;

    /*
      audioDecoder->pause = pause;
      audioDecoder->cs.fastForward = playSpeed;
    */

    StartTask(TASK_DECODER, PlayerThread);
    Delay(TICKS_PER_SEC/10);
    miscData.sampleRate = cs->sampleRate;
    miscData.playTimeTotal = cs->playTimeTotal;
    printf("~%04x=%ld\n", UIMSG_U32_TOTAL_PLAY_TIME_SECONDS, cs->playTimeTotal);
    printf("~%04x=%d\n", UIMSG_S16_PLAYING, UIM_PLAYING_PLAYING);
    miscData.recordingTime = 0;

#ifdef USE_LCD
    UpdateLcd(-1);
#endif

    while (pSysTasks[TASK_DECODER].task.tc_State &&
	   pSysTasks[TASK_DECODER].task.tc_State != TS_REMOVED) {
      int hadAnythingToDo = 0;

#ifdef USE_LCD
      UpdateLcd(0);
#endif

#ifdef USE_DEVBOARD_KEYS
      newKeysPushed |= GetDevBoardKeysPushed();
#endif
#ifdef USE_UART_KEYS
      newKeysPushed |= GetUartKeysPushed();
#endif
#if 0
      if (newKeysPushed) {
	printf("nkp: %04x\n", newKeysPushed);
      }
#endif

      newSeconds = cs->playTimeSeconds;

#ifdef USE_EXT1_BOARD_KEYS
      ModifyVolume(&newKeysPushed, &oldRotaryPos);
#else
      ModifyVolume(&newKeysPushed);
#endif


#ifdef USE_EXT1_BOARD_KEYS
     if (newRotaryButtons & ROTARY_UP) {
       newRotaryButtons &= ~ROTARY_UP;
       newKeysPushed |= KEY_MASK_PREVIOUS;
     }
     if (newRotaryButtons & ROTARY_DOWN) {
       newRotaryButtons &= ~ROTARY_DOWN;
       newKeysPushed |= KEY_MASK_NEXT;
     }
     if (newRotaryButtons & ROTARY_LEFT) {
       newRotaryButtons &= ~ROTARY_LEFT;
       newKeysPushed |= KEY_MASK_FAST_REVERSE;
     }
     if (newRotaryButtons & ROTARY_RIGHT) {
       newRotaryButtons &= ~ROTARY_RIGHT;
       newKeysPushed |= KEY_MASK_FAST_FORWARD;
     }
     if (newRotaryButtons & ROTARY_CENTER) {
       newRotaryButtons &= ~ROTARY_CENTER;
       newKeysPushed |= KEY_MASK_PAUSE;
     }
#endif

      if (newKeysPushed & KEY_MASK_PREVIOUS) {
	newKeysPushed &= ~KEY_MASK_PREVIOUS;
	audioDecoder->pause = 0;
	miscData.recMode = rmPlayback;	
	if (newSeconds >= 5 || currFileIdx < 1) {
	  cs->goTo = 0;
	} else {
	  cs->cancel = 1;
	  currFileIdx-=2;
	}
      } else if (newKeysPushed & KEY_MASK_NEXT) {
	newKeysPushed &= ~KEY_MASK_NEXT;
	audioDecoder->pause = 0;
	miscData.recMode = rmPlayback;	
	cs->cancel = 1;
      } else if (newKeysPushed & KEY_MASK_FAST_REVERSE) {
	newKeysPushed &= ~KEY_MASK_FAST_REVERSE;
	if (cs->goTo == 0xFFFFU) {
	  s_int32 t=cs->playTimeSeconds;
	  if ((t-=60) < 0) {
	    t = 0;
	  }
	  cs->goTo = (u_int16)t;
	}
      } else if (newKeysPushed & KEY_MASK_FAST_FORWARD) {
	newKeysPushed &= ~KEY_MASK_FAST_FORWARD;
	if (cs->goTo == 0xFFFFU) {
	  cs->goTo = (u_int16)(cs->playTimeSeconds) + 60;
	}
      } else if (newKeysPushed & KEY_MASK_PAUSE) {
	newKeysPushed &= ~KEY_MASK_PAUSE;
	audioDecoder->pause = !audioDecoder->pause;
	printf("~%04x=%d\n", UIMSG_S16_PLAYING, 2-audioDecoder->pause);
	miscData.recMode = audioDecoder->pause ? rmPause : rmPlayback;	
      }

      if (newSeconds != lastSeconds && newSeconds >= 0) {
	s_int16 newPerCent =
	  (s_int16)(cs->Tell(cs)/(cs->fileSize/100));
	hadAnythingToDo = 1;
	lastSeconds = newSeconds;
	miscData.recordingTime = newSeconds;
	if (newPerCent != lastPerCent) {
	  lastPerCent = newPerCent;
	  printf("~%04x=%d\n", UIMSG_U32_PLAY_FILE_PERCENT, newPerCent);
	}
	printf("~%04x=%ld\n", UIMSG_U32_PLAY_TIME_SECONDS, lastSeconds);
      }

      if ((newKeysPushed & (/*KEY_MASK_PLAYBACK|*/KEY_MASK_STOP|KEY_MASK_QUIT)) || (appFlags & APP_FLAG_QUIT)) {
	if (newKeysPushed & KEY_MASK_QUIT) {
	  appFlags |= APP_FLAG_QUIT;
	}
	newKeysPushed &= ~(/*KEY_MASK_PLAYBACK|*/KEY_MASK_STOP|KEY_MASK_QUIT);
	cs->cancel = 1;
	audioDecoder->pause = 0;
	quit = 1;
      }
    }
    cs->Output = originalOutput;
    DeleteAudioDecoder(decoderLibrary, audioDecoder);
    audioDecoder = NULL;

    fclose(fp);
    fp = NULL;

    if (++currFileIdx >= audioFiles) {
      quit = 1;
    }
  } /* while (!quit) */

 finally:
  printf("~%04x=%d\n", UIMSG_S16_PLAYING, UIM_PLAYING_STOPPED);

  if (decoderLibrary) {
    if (audioDecoder) {
      cs->Output = originalOutput;
      DeleteAudioDecoder(decoderLibrary, audioDecoder);
      audioDecoder = NULL;
    }
    DropLibrary(decoderLibrary);
    decoderLibrary = NULL;
  }

  /* Restore states / LCD for the recorder */
  miscData.bitsPerSample = originalBitsPerSample;
  miscData.sampleRate = originalSampleRate;
  SetRateAndBits();
  miscData.recMode = rmStop;
  miscData.playTimeTotal = 0;
#ifdef USE_LCD
  UpdateLcd(1);
#endif
}

#define FILE_SYSTEM_CHECK_INTERVAL (2*TICKS_PER_SEC)

#define MAX_CMD_LENGTH 255
char cmd[MAX_CMD_LENGTH+1];


ioresult main(char *parameters) {
  int nParam, i;
  char *p = parameters;
  ioresult retVal = S_ERROR;
  FatFileInfo *fi = NULL;
  FatDeviceInfo *di = NULL;
  u_int16 fileNumber = 0;
  s_int16 t;
  s_int32 audioBlocks = 0, audioBlocksInThisFile = 0;
  s_int16 outBufWP = 0, inBufRP = IN_BUF_SIZE_24BIT;
  u_int32 lastRecordingTime = -1;
  int padBlocks = 0;
  u_int32 firstOverflows = 0;
  u_int32 newKeysPushed = 0/*KEY_MASK_RECORD*/;
  int verbose = 0;
#ifdef USE_EXT1_BOARD_KEYS
  int oldRotaryPos = rotaryPos;
#endif
  u_int32 lastFileSystemCheckTimeCount;
  ioresult writeErr = S_OK;
  s_int16 currCmdPos = -1;
  s_int16 skipAlign = 0;
  s_int16 noGoodFileSystem = 0;

  miscData.sampleRate = 96000;
  miscData.bitsPerSample = 24;
  miscData.audioChannels = 2;
  miscData.volume = (ioctl(stdaudioout, IOCTL_AUDIO_GET_VOLUME, NULL)-256) & ~3;

  vs23MaxFill = 0;

#if 0
  if (!PERIP(0) || PERIP(0)) {
    static u_int16 buf[4][16] = {
      {0x0101, 0x1111, 0x2222, 0x3333, 0x4444, 0x5555, 0x6666, 0x7777,
       0x8888, 0x9999, 0xaaaa, 0xbbbb, 0xcccc, 0xdddd, 0xeeee, 0xffff},
      {0x0101, 0x1111, 0x2222, 0x3333, 0x4444, 0x5555, 0x6666, 0x7777,
       0x8888, 0x9999, 0xaaaa, 0xbbbb, 0xcccc, 0xdddd, 0xeeee, 0xffff},
      "\pPacked string\0Packd string",
      "Unpacked string"
    };
    int j;
    printf("PERIP(0) = 0x%04x\n", PERIP(0));
    strcpypack(&buf[0][2], &buf[3][1]);
    printf("%d %d\n", strlenpacked(&buf[2][0]), strlenpacked(&buf[2][7]));
    strcpyunpack(&buf[1][1], &buf[2][7]);
    for (i=0; i<4; i++) {
      for (j=0; j<16; j++) {
	printf("%04x ", buf[i][j]);
      }
      printf("\n");
    }

    return 0;
  }
#endif

#ifdef USE_EXT1_BOARD_KEYS
  AddCyclic(&checkRotaryNode, TICKS_PER_SEC/200, TICKS_PER_SEC/200);
#endif

  printf("\n%s\n%s\n\n", VERSION1, VERSION2);
#ifdef USE_LCD
  LcdFilledRectangle(0,0,lcd0.width-1,lcd0.height-1,NULL,lcd0.backgroundColor);
  PERIP(PWM_PULSELEN) = 128;
#endif

  GetRtcParameters();
  ioctl(stdaudioout, IOCTL_AUDIO_SET_VOLUME, (void *)(miscData.volume+256));
  printf("~%04x=%d\n", UIMSG_S16_SET_VOLUME, miscData.volume);

  nParam = RunProgram("ParamSpl", (u_int16)parameters);
  for (i=0; i<nParam; i++) {
    if (!strcmp(p, "-h")) {
      printf("Usage: HiResRec [-p|+p|"
	     "-v|+v|-nx|rate|bits|-h]\n"
	     "-p/+p\tPause / Don't pause at start\n"
	     "-nx\tSet file number counter to x (1-65535)\n"
	     "-v/+v\tVerbose on/off\n"
	     "rate\t96000 / 48000 / 24000\n"
	     "bits\t24 / 16\n"
	     "-h\tShow this help\n");
      retVal = S_OK;
      goto finally;
    } else if (!strcmp(p, "-p")) {
      newKeysPushed = 0;
    } else if (!strcmp(p, "+p")) {
      newKeysPushed = KEY_MASK_RECORD;
    } else if (!strncmp(p, "-n", 2)) {
      s_int32 forceFileCounter = strtol(p+2, NULL, 0);
      if (forceFileCounter < 1 || forceFileCounter > 65535) {
	printf("E: Value outside of range in option \"%s\"\n", p);
	goto finally;
      } else {
	SetRtcFileNumber((u_int16)forceFileCounter);
      }
    } else if (!strcmp(p, "-v")) {
      verbose = 1;
    } else if (!strcmp(p, "+v")) {
      verbose = -1;
    } else {
      s_int32 t32 = strtol(p, NULL, 0);
      if (t32 == 24000 || t32 == 48000 || t32 == 96000) {
	miscData.sampleRate = t32;
      } else if (t32 == 16 || t32 == 24) {
	miscData.bitsPerSample = (s_int16)t32;
      } else {
	printf("E: Unknown parameter \"%s\"\n", p);
	goto finally;
      }
    }
    p += strlen(p)+1;
  }

  if (miscData.sampleRate == 96000) {
    qualityMenuItems[2].value = 1;
  } else if (miscData.sampleRate == 48000) {
    qualityMenuItems[3].value = 1;
  } else {
    qualityMenuItems[4].value = 1;
  }

  if (miscData.bitsPerSample == 24) {
    qualityMenuItems[6].value = 1;
  } else {
    qualityMenuItems[7].value = 1;
  }

  lastFileSystemCheckTimeCount = ReadTimeCount() - FILE_SYSTEM_CHECK_INTERVAL;

#ifdef USE_LCD
  UpdateLcd(1);
#endif

  miscData.peak[0] = miscData.peak[1] = 2000;

#if 0
  printf("#2 FOPEN %p\n", fp);
#endif

  SetRtcParameters();

  if (!(sdDev = VODEV('D'))) {
#ifdef USE_LCD
    LcdFilledRectangle(0,0,lcd0.width-1,lcd0.height-1,NULL,lcd0.backgroundColor);
    LcdTextOutXY(54, 30, "FATAL");
    LcdTextOutXY(54, 40, "ERROR");
    LcdTextOutXY(54, 60, "CAN'T");
    LcdTextOutXY(57, 70, "FIND");
    LcdTextOutXY(40, 80, "SD DRIVER");
#endif

    printf("E: Can't find device D:\n");
    Delay(2*TICKS_PER_SEC);
    goto finally;
  }

  ioctl(sdDev, IOCTL_RESTART, NULL);
  mkdir("D:AUDIO");
  {
    FILE *fp = fopen("D:AUDIO/.", "s");
    if (fp) {
      fclose(fp);
    } else {
      noGoodFileSystem = 1;
    }
  }

  di = (FatDeviceInfo *)sdDev->deviceInfo;

  BuildFileEntries(sdDev, 1);

  if (SetRateAndBits()) {
    goto finally;
  }

  GetLargestFragment(sdDev, &freeFrag, 0);

  for (i=0; i<16; i++) {
    fread(inBuf, 1, IN_BUF_SIZE_32BIT, stdaudioin);
  }

#if 1
  fileNumber = GetRtcFileNumber();
  sprintf(miscData.fileName, "AUD%05u.WAV", fileNumber);

  while (!(appFlags & APP_FLAG_QUIT) && !(newKeysPushed & KEY_MASK_QUIT)) {
    u_int32 currTime = ReadTimeCount();

#ifdef USE_DEVBOARD_KEYS
    newKeysPushed |= GetDevBoardKeysPushed();
#endif
#ifdef USE_UART_KEYS
    if (currCmdPos < 0) {
      newKeysPushed |= GetUartKeysPushed();
    } else {
      newKeysPushed = 0;
      while (ioctl(stdin, IOCTL_TEST, NULL) > 0) {
	int ch = fgetc(stdin);
	if (ch == 8) {
	  printf("\nCMD Cancel!\n");
	  currCmdPos = -1;
	} else if (ch == '\n' || ch == '\r') {
	  char *p;
	  ioresult st;
	  cmd[currCmdPos] = '\0';
	  p = strchr(cmd, ' ');
	  while (*p == ' ') {
	    *p++ = '\0';
	  }
	  printf("\n");
	  st = RunProgram(cmd, p);
	  printf("CMD result: %d\n", st);
	  currCmdPos = -1;
	} else {
	  if (currCmdPos <= MAX_CMD_LENGTH) {
	    fputc(ch, stdout);
	    cmd[currCmdPos++] = ch;
	  }
	}
      }
    }
#endif
    if (miscData.recMode == rmRecord) {
      if (newKeysPushed & KEY_MASK_PAUSE) {
	newKeysPushed &= ~KEY_MASK_PAUSE;
	miscData.recPause = !miscData.recPause;
	printf("~%04x=%d\n", UIMSG_S16_PLAYING, 2-miscData.recPause);
      }
      if (newKeysPushed & KEY_MASK_RECORD) {
	miscData.recPause = 0;
      }
    }
    if (miscData.recMode == rmRecord && writeErr) {
      FILE *fp;
      newKeysPushed = KEY_MASK_RECORD;
#ifdef USE_LCD
      LcdFilledRectangle(0,30,lcd0.width-1,lcd0.height-1,NULL,lcd0.backgroundColor);
      lcd0.textColor = __RGB565RGB(255, 255, 0);
      LcdTextOutXY(20, 50, "YOU MUST REINSERT");
      LcdTextOutXY(55, 60, "SD CARD");
      LcdTextOutXY(55, 70, "OR DATA");
      LcdTextOutXY(27, 80, "WILL BE LOST!!!");
#endif
      printf("W: You must reinsert SD Card or data will be lost!\n");
      while (!(fp = fopen("D:AUDIO", "rb"))) {
	printf("E: Can't open D:AUDIO/\n");
	Delay(TICKS_PER_SEC);
	ioctl(sdDev, IOCTL_RESTART, NULL);
	mkdir("D:AUDIO");
      }
      writeErr = 0;
#ifdef USE_LCD
      UpdateLcd(1);
#endif
#ifdef USE_EXT1_BOARD_KEYS
      newRotaryButtons = 0;
#endif
    }

    if (currTime - lastFileSystemCheckTimeCount >= FILE_SYSTEM_CHECK_INTERVAL &&
	miscData.recMode == rmStop) {
      static u_int16 tmpBuf[256];
      lastFileSystemCheckTimeCount = currTime;
#if 0
      printf("#1 fp %p, buf %p, read()\n", fp, fp->sectorBuffer,
	     sdDev->BlockRead());
#endif
      if (noGoodFileSystem || sdDev->BlockRead(sdDev, 0, 1, tmpBuf)) {
	FILE *fp;
	noGoodFileSystem = 0;
	while (!(fp = fopen("D:AUDIO", "rb"))) {
	  printf("E: Can't open D:AUDIO/\n");
	  if (!(sdDev->BlockRead(sdDev, 0, 1, tmpBuf))) {
	    /* We have some device but the file format is unknown.
	       So let's suggest formatting. */
	    if (ShowMenu(&format2Menu)) {
	      newKeysPushed |= KEY_MASK_QUIT;
	    }
#ifdef USE_EXT1_BOARD_KEYS
	    oldRotaryPos = rotaryPos;
	    newRotaryButtons = 0;
#endif
	    newKeysPushed &= KEY_MASK_QUIT;

#ifdef USE_LCD
	    UpdateLcd(1);
#endif
	  } else {
#ifdef USE_LCD
	    LcdFilledRectangle(0,30,lcd0.width-1,lcd0.height-1,NULL,lcd0.backgroundColor);
	    lcd0.textColor = __RGB565RGB(255, 255, 0);
	    LcdTextOutXY(47, 50, "PLEASE");
	    LcdTextOutXY(47, 60, "INSERT");
	    LcdTextOutXY(40, 70, "SD CARD!");
#endif
	    printf("W: Please insert SD Card!\n");
	
	  }
	  Delay(TICKS_PER_SEC);
	  ioctl(sdDev, IOCTL_RESTART, NULL);
	  mkdir("D:AUDIO");
	}
	fclose(fp);
	printf("W: New SD Card found!\n");
	    LcdTextOutXY(47, 50, "PLEASE");
	    LcdTextOutXY(47, 60, " WAIT ");
	    LcdTextOutXY(40, 70, "   ...  ");
	GetLargestFragment(sdDev, &freeFrag, 0);
#ifdef USE_LCD
	UpdateLcd(1);
#endif
#ifdef USE_EXT1_BOARD_KEYS
	newRotaryButtons = 0;
#endif
      }
    }

    if (miscData.fatBits != 32) {
      if (ShowMenu(&format2Menu)) {
	newKeysPushed |= KEY_MASK_QUIT;
      }
#ifdef USE_EXT1_BOARD_KEYS
      oldRotaryPos = rotaryPos;
      newRotaryButtons = 0;
#endif
      newKeysPushed &= KEY_MASK_QUIT;

#ifdef USE_LCD
      UpdateLcd(1);
#endif
      GetLargestFragment(sdDev, &freeFrag, 1);
    }

    if (newKeysPushed & KEY_MASK_COMMAND && currCmdPos < 0) {
      newKeysPushed &= KEY_MASK_QUIT;
      if (miscData.recMode == rmStop) {
	currCmdPos = 0;
	printf("CMD:");
      }
    } else if (newKeysPushed & KEY_MASK_VUMETER) {
      newKeysPushed &= ~KEY_MASK_VUMETER;
      printf("VU %5.1f %5.1f dB\n",
	     -0.1*miscData.peak[2], -0.1*miscData.peak[3]);
      miscData.peak[2] = miscData.peak[3] = 2000;
      miscData.level[2] = miscData.level[3] = 2000;
    }

    miscData.recordingTime =
      (s_int32)(512.0*audioBlocks/miscData.bytesPerSec+0.5);

    if (miscData.recordingTime != lastRecordingTime) {
      char fileName[14];
      u_int32 t32;
      u_int16 maxFill;

      lastRecordingTime = miscData.recordingTime;
      ioctl(stdaudioin, IOCTL_AUDIO_GET_OVERFLOWS, (void *)(&t32));

      miscData.spaceInBlocks = freeFrag.blocks - audioBlocks;

      if (!miscData.recordingTime) {
	firstOverflows = t32;
      }
      SetRtcBlocks(audioBlocks);

      Forbid();
      maxFill = vs23MaxFill;
      vs23MaxFill = 0;
      Permit();

      printf("D:AUDIO/%s: %4lds, %4ld samples lost, "
	     "%3d/%d KiB buffer used\n",
	     miscData.fileName,
	     miscData.recordingTime,
	     t32-firstOverflows, maxFill/2, vs23Size/2);
    }
    if (inBufRP >= IN_BUF_SIZE_24BIT || miscData.recMode == rmStop ||
	miscData.recMode == rmPause) {
      if (miscData.bitsPerSample > 16) {
	fread(inBuf, 1, IN_BUF_SIZE_32BIT, stdaudioin);
	if (ioctl(stdaudioout, IOCTL_AUDIO_GET_OUTPUT_BUFFER_FREE, NULL) >
	    IN_BUF_SIZE_32BIT) {
	  fwrite(inBuf, 1, IN_BUF_SIZE_32BIT, stdaudioout);
	}
	FindAudioLevels((s_int32 *)inBuf, IN_BUF_SIZE_32BIT/4);
	Convert32BitTo24Bit(inBuf, IN_BUF_SIZE_32BIT/4);
      } else {
	fread(inBuf, 1, IN_BUF_SIZE_24BIT, stdaudioin);
	if (ioctl(stdaudioout, IOCTL_AUDIO_GET_OUTPUT_BUFFER_FREE, NULL) >
	    IN_BUF_SIZE_24BIT) {
	  fwrite(inBuf, 1, IN_BUF_SIZE_24BIT, stdaudioout);
	}
	FindAudioLevels((s_int32 *)inBuf, IN_BUF_SIZE_24BIT/2);
	Convert16BitTo16Bit(inBuf, IN_BUF_SIZE_24BIT/2);
      }
      inBufRP = 0;
    }

    if (currCmdPos >= 0) {
      newKeysPushed &= KEY_MASK_QUIT;
    }

#ifdef USE_EXT1_BOARD_KEYS
    ModifyVolume(&newKeysPushed, &oldRotaryPos);
#else
    ModifyVolume(&newKeysPushed);
#endif

    if (miscData.recMode == rmRecord || miscData.recMode == rmPause) {
      if (miscData.recMode == rmRecord) {
	int toCopy = 256-outBufWP;
	if (toCopy > IN_BUF_SIZE_24BIT-inBufRP) {
	  toCopy = IN_BUF_SIZE_24BIT-inBufRP;
	}
	memcpy(outBuf+outBufWP, inBuf+inBufRP, toCopy);
	inBufRP += toCopy;
	outBufWP += toCopy;
      }
      if (outBufWP >= 256) {
	if (!miscData.recPause && !skipAlign) {
	  writeErr = sdDev->BlockWrite(sdDev, freeFrag.firstBlock+audioBlocks,
				       1, outBuf);
	  audioBlocks++;
	  audioBlocksInThisFile++;
	} else {
	  /* skipAlign will make it sure that we pause always a number of
	     blocks that is a multiple of 3. Thism, in order, will make it sure
	     that 24-bit files keep aligned even after pausing recording. */
	  if (++skipAlign == 3) {
	    skipAlign = 0;
	  }
	  writeErr = 0;
	}

	outBufWP = 0;

	if (audioBlocksInThisFile >= MAX_BLOCKS_PER_FILE) {
	  u_int32 t32;
	  u_int16 maxFill;
	  ioctl(stdaudioin, IOCTL_AUDIO_GET_OVERFLOWS, (void *)(&t32));

	  Forbid();
	  maxFill = vs23MaxFill;
	  vs23MaxFill = 0;
	  Permit();

	  miscData.recordingTime =
	    (s_int32)(512.0*audioBlocks/miscData.bytesPerSec+0.5);

	  printf("D:AUDIO/%s: %4lds, %4ld samples lost, "
		 "%3d/%d KiB buffer used\n",
		 miscData.fileName, miscData.recordingTime,
		 t32-firstOverflows, maxFill/2, vs23Size/2);
	  vs23MaxFill = 0;
	  audioBlocksInThisFile = 0;
	  memset(outBuf, 0, sizeof(outBuf));
	  for (i=0; i<padBlocks; i++) {
	    sdDev->BlockWrite(sdDev, freeFrag.firstBlock+audioBlocks, 1, outBuf);
	    audioBlocks++;
	  }
	  memcpy(outBuf, riffWavHeader, sizeof(riffWavHeader));
	  outBufWP = sizeof(riffWavHeader);
	  fileNumber++;
	  sprintf(miscData.fileName, "AUD%05u.WAV", fileNumber);
	}
      }
      if (audioBlocks >= 1125) { /* 1 second 96kHz/24b, 3 seconds 48kHz/16b. */
#ifdef USE_EXT1_BOARD_KEYS
	if (newRotaryButtons & ROTARY_CENTER) {
	  newRotaryButtons = 0;
	  miscData.recMode = (miscData.recMode == rmPause) ? rmRecord : rmPause;
	}
#endif
	if (newKeysPushed & KEY_MASK_RECORD || audioBlocks >= freeFrag.blocks) {
	  /* If stopped because continuous space had ended, simulate a
	     KEY_MASK_RECORD event. */
	  if (audioBlocks >= freeFrag.blocks) {
	    printf("E: Disk or largest fragment full\n");
	  }
	  newKeysPushed ^= KEY_MASK_RECORD;
#ifdef USE_EXT1_BOARD_KEYS
	  newRotaryButtons = 0;
#endif
	  miscData.recMode = rmStop;
	  miscData.recordingTime = 0;
	  memset(miscData.level, 2000, 4);
	  memset(miscData.peak, 2000, 4);
	  SetRtcBlocks(audioBlocks);
	  BuildFileEntries(sdDev, 0);
	  audioBlocks = 0;
	  audioBlocksInThisFile = 0;
	  outBufWP = 0, inBufRP = IN_BUF_SIZE_24BIT;
	  fileNumber = GetRtcFileNumber();
	  sprintf(miscData.fileName, "AUD%05u.WAV", fileNumber);
	  GetLargestFragment(sdDev, &freeFrag, 0);
	  miscData.recPause = 0;
	  vs23MaxFill = 0;
	}
      }
      newKeysPushed &= KEY_MASK_QUIT;
    } else if (miscData.recMode == rmStop) {
#ifdef USE_EXT1_BOARD_KEYS
      if (newRotaryButtons & ROTARY_CENTER) {
	newRotaryButtons &= ~ROTARY_CENTER;
	newKeysPushed |= KEY_MASK_MENU;
      }
#endif
      if (newKeysPushed & KEY_MASK_MENU) {
	newKeysPushed &= ~KEY_MASK_MENU;

	mainMenuItems[2].value = -miscData.volume/2 & ~1;

	if (ShowMenu(&mainMenu)) {
	  newKeysPushed |= KEY_MASK_QUIT;
	}
#ifdef USE_EXT1_BOARD_KEYS
	oldRotaryPos = rotaryPos;
	newRotaryButtons = 0;
#endif
	newKeysPushed &= KEY_MASK_QUIT;
	memset(inBuf, 0, sizeof(inBuf));
	memset(outBuf, 0, sizeof(outBuf));

	if (qualityMenuItems[2].modified) {
	  qualityMenuItems[2].modified = 0;
	  qualityMenuItems[3].modified = 0;
	  qualityMenuItems[4].modified = 0;
	  if (qualityMenuItems[2].value) {
	    miscData.sampleRate = 96000;
	  } else if (qualityMenuItems[3].value) {
	    miscData.sampleRate = 48000;
	  } else {
	    miscData.sampleRate = 24000;
	  }
	  SetRtcParameters();
	}

	if (qualityMenuItems[6].modified) {
	  qualityMenuItems[6].modified = 0;
	  qualityMenuItems[7].modified = 0;
	  if (qualityMenuItems[6].value) {
	    miscData.bitsPerSample = 24;
	  } else {
	    miscData.bitsPerSample = 16;
	  }
	  SetRtcParameters();
	}

	if (SetRateAndBits()) {
	  goto finally;
	}

#if 0
	miscData.volume = -2*mainMenuItems[2].value;
	ioctl(stdaudioout, IOCTL_AUDIO_SET_VOLUME, (void *)(miscData.volume+256));
#endif

#ifdef USE_LCD
	UpdateLcd(1);
#endif
	GetLargestFragment(sdDev, &freeFrag, 1);
      }
      if (newKeysPushed & KEY_MASK_PLAYBACK) {
	newKeysPushed &= KEY_MASK_PLAYBACK;
	/* Open file, go to playback program... */
	Playback(fileNumber-1);
	sprintf(miscData.fileName, "AUD%05u.WAV", fileNumber);
#ifdef USE_EXT1_BOARD_KEYS
	oldRotaryPos = rotaryPos;
#endif
	newKeysPushed &= ~(KEY_MASK_QUIT|KEY_MASK_PLAYBACK);
      } else if (newKeysPushed & KEY_MASK_RECORD) {

#if 0
	printf("##0\n");
	printf("Cluster start %ld=0x%lx, size %ld\n",
	       freeFrag.firstCluster, freeFrag.firstCluster, freeFrag.clusters);
	printf("Block   start %ld=0x%lx, size %ld\n",
	       freeFrag.firstBlock, freeFrag.firstBlock, freeFrag.blocks);
#endif
	if (freeFrag.blocks < 1125) {
	  printf("E: Fragment full\n");
	} else {
	  miscData.recMode = rmRecord;
	  miscData.recPause = 0;
	  vs23MaxFill = 0;
	  
	  SetRtcStartCluster(freeFrag.firstCluster);

	  t = (u_int16)(MAX_BLOCKS_PER_FILE & (di->fatSectorsPerCluster - 1));
	  if (t) {
	    padBlocks = di->fatSectorsPerCluster-t;
	  }

	  memcpy(outBuf, riffWavHeader, sizeof(riffWavHeader));
	  outBufWP = sizeof(riffWavHeader);
	  vs23MaxFill = 0;
	  lastRecordingTime = -1;
	}
      }
      newKeysPushed &= KEY_MASK_QUIT;
    } else if (miscData.recMode == rmPlayback) {
      if (newKeysPushed & KEY_MASK_STOP) {
	miscData.recMode = rmStop;
#if 0
	printf("#3 FCLOSE %p\n", fp);
#endif
      } else {
	
      }
      newKeysPushed &= KEY_MASK_QUIT;
    } /* if (recMode == ...) */

#ifdef USE_LCD
    UpdateLcd(0);
#endif
  } /* while(!quit) */
  SetRtcBlocks(audioBlocks);

  BuildFileEntries(sdDev, 0);
#endif
  
  retVal = S_OK;
 finally:
  printf("Exiting.\n");
  SetRtcParameters();

#ifdef USE_EXT1_BOARD_KEYS
  DropCyclic(&checkRotaryNode);
#endif

#ifdef USE_LCD
  i = 128;
  while (i) {
    Delay(20);
    i=i*120/128;
    PERIP(PWM_PULSELEN) = i;
  }
  LcdFilledRectangle(0,0,lcd0.width-1,lcd0.height-1,NULL,lcd0.backgroundColor);
#endif

  return retVal;
}
