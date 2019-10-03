/// \file devSdSdBase.c VsOS Device driver for SD card in native SD mode
/// \author Timo Solla, Henrik Herranen, VLSI Solution Oy

/* NOTE!
   devSdSdBase.c, devSdSdBaseMonolithic.c, decSdSdBaseReadOnly.c
   are aliases to the same file. They should be exactly the same! */

#include <stdlib.h>
#include <string.h>
#include <vo_stdio.h>
#include <timers.h>
#include <clockspeed.h>
#include <audio.h>
#include <power.h>
#include <apploader.h>
#include <taskandstack.h>
#include <extSymbols.h>

#ifndef NO_VSIDE_SETUP
#include "vsos.h"
#include <devSdSd.h>
#include "vo_fat.h"
#include "vs1005h.h"
#include "mmcCommands.h"
//#include "forbid_stdout.h"
#include <fifoRdWr.h>
#include "hwLocks.h"
#include "devByteBus.h"
#include "vs23s010_io.h"
#endif

/* Restarting a Multiple Block Read is an expensive operation.
   MULTIPLE_BLOCK_READ_SKIP_LIMIT tells how many blocks must be skipped
   before a new multiple block read operation is done. */
#define MULTIPLE_BLOCK_READ_SKIP_LIMIT 6

#if 0
/* If not defined, commands not used by the driver itself will
   not be supported */
#  define SUPPORT_UNUSED_CMDS
#endif

#if 0
/* If defined, will check CRC error message from card when writing a
   data block. This is powerful for debugging, but usually just makes
   the driver unnecessarily alightly larger with no benefits. */
#  define CHECK_WRITE_CRC_ERROR
#endif

DEVICE byteBus;
s_int16 vs23MaxFill = 0, vs23Size = 0, vs23CurrFill = 0;

ioresult thereHaveBeenWriteErrors = S_OK;

// devSdSd.hwInfo.flags : params/flags
// devSdSd.hwInfo.rca : RCA
// devSdSd.hwInfo.sizeHigh : size high
// devSdSd.hwInfo.sizeLow : size low
// devSdSd.hwInfo.statusHigh : status high / arg high
// devSdSd.hwInfo.statusLow : status low  / arg low

#ifdef SDSDMONO
int IoctlRestart(SD_DEVICE *dev);
#endif /* SDSDMONO */

const SD_DEVICE __mem_y devSdSdDefaults = {   
  0, // u_int16 flags; //< present, block/char
  DevSdSdIdentify, //char*    (*Identify)(void *obj, char *buf, u_int16 bufsize);   
  DevSdSdCreate, //ioresult (*Create)(DEVICE *dev, char *name, struct filesystem_descriptor *fs);
  DevSdSdDelete, //ioresult (*Delete)(DEVICE *dev);
  DevSdSdIoctl, //ioresult (*Ioctl)(DEVICE *dev, s_int16 request, s_int32 arg); //Start, Stop, Flush, Check Media
  // Stream operations
  NULL, //u_int16  (*Read) (register __i0 DEVICE *self, void *buf, u_int16 destinationIndex, u_int16 bytes); ///< Read bytes to *buf, at byte index
  NULL, //u_int16  (*Write)(register __i0 DEVICE *self, void *buf, u_int16 sourceIndex, u_int16 bytes); ///< Write bytes from *buf at byte index
  // Block device operations
  DevSdSdBlockRead, //ioresult (*BlockRead)(DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data);
  DevSdSdBlockWrite, //ioresult (*BlockWrite)(DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data);
  // Stream operations
  NULL, //FILESYSTEM *fs;
  0, //u_int16  deviceInstance;
  {SDSD_F_TRY_4BIT,0,0,0},//SD_HWINFO  hwInfo; ///< Device driver's info of this hardware, 6 words
  {0}//u_int16  deviceInfo[10]; // For filesystem use, size TBD   
};


#ifdef SDSDMONO
auto void WaitSdReady(void);
#else /* !SDSDMONO */
auto void WaitSdReady(void) {
  while (PERIP(SD_CF) & SD_CF_ENA)          // wait for ready
    ;
}
#endif /* !SDSDMONO */


int track23 = -1;



// vs1005 SD pins:
//    cmd  : gpio2(10)
//    dat3 : gpio2(9)
//    dat2 : gpio2(8)
//    dat1 : gpio2(7)
//    dat0 : gpio2(6)
//    clk  : gpio2(5)
// Using gpmode to switch between drive '1' (gp mode) and
// 'Z'(perip mode) states   (clk is always driven)
#define SD_SETUP_GPIO()  {PERIP(GPIO2_SET_MASK)=0x07C0; PERIP(GPIO2_DDR)|=0x07E0; PERIP(GPIO2_MODE)&=~0x07E0;}
#define SD_DAT_HIGH()     {PERIP(GPIO2_MODE)  &= ~0x03C0;}
#define SD_DAT3_HIGH()    {PERIP(GPIO2_MODE)  &= ~0x0200;}
#define SD_DAT3_1HIGH()   {PERIP(GPIO2_MODE)  &= ~0x0380;}
#define SD_DAT0_HIGH()    {PERIP(GPIO2_MODE)  &= ~0x0040;}
#define SD_CMD_HIGH()     {PERIP(GPIO2_MODE)  &= ~0x0400;}
#define SD_CMD_DAT_HIGH() {PERIP(GPIO2_MODE)  &= ~0x07C0;}
#define SD_DAT_Z()        {PERIP(GPIO2_MODE)  |=  0x03C0;} // perip mode => input
#define SD_DAT3_Z()       {PERIP(GPIO2_MODE)  |=  0x0200;}
#define SD_DAT0_Z()       {PERIP(GPIO2_MODE)  |=  0x0040;}
#define SD_CMD_Z()        {PERIP(GPIO2_MODE)  |=  0x0400;}
#define SDgpioSetup()     {PERIP(GPIO2_SET_MASK)=0x07C0; PERIP(GPIO2_DDR)|=0x07E0; PERIP(GPIO2_MODE)&=~0x07E0;}
#define SDdatHigh()    {PERIP(GPIO2_MODE)  &= ~0x03C0;}
#define SDdat3High()   {PERIP(GPIO2_MODE)  &= ~0x0200;}
#define SDdat3_1High() {PERIP(GPIO2_MODE)  &= ~0x0380;}
#define SDdat0High()   {PERIP(GPIO2_MODE)  &= ~0x0040;}
#define SDcmdHigh()    {PERIP(GPIO2_MODE)  &= ~0x0400;}
#define SDdatZ()       {PERIP(GPIO2_MODE)  |=  0x03C0;} // perip mode => input
#define SDdat3Z()      {PERIP(GPIO2_MODE)  |=  0x0200;}
#define SDdat0Z()      {PERIP(GPIO2_MODE)  |=  0x0040;}
#define SDcmdZ()       {PERIP(GPIO2_MODE)  |=  0x0400;}


// CMD Responses are
//       R1  : 6 bytes : 
//             "00" & [45:40]=cmdid, [39:8]=status, crc7 & '1'
//       R1b : 6 bytes : 
//             "00" & [45:40]=cmdid, [39:8]=status, crc7 & '1'
//             + possible busy in data line
//       R2 : 136b = 17B  (CMD2):
//             "00" & "111111" & [127:1]=CID/CSD & '1'
//       R3 : 6 bytes : 
//             "00" & "111111", [39:8]=OCR, "1111111" & '1'
//       R6 : 6 bytes :
//             "0x03" & RCA[15:0] & CardStatus[some 16 bits] & crc7 & '1'
// sd_cmd:
//     [15]  : RCA as argument
//     [14]  : SD.cmdarg as argument
//     [13:8]:  CMDx (0-51)
//     [7]   : R1 // 48bit   (6B)
//     [6]   : R2 // 135bit  (17B, "00111111" + 16B)
//     [5]   : R3 // 48bit   (6B)           reserved, cmd_datax => always '1'
//     [4]   : R6 // 48bit   (6B)           reserved, noDstart  => always '0'
//     [3]   : R1b-busy signaling in data   reserved, noDstop   => always '0'
//     [2]:                                 reserved, crc16     => always '0'
//     [1]:    crc7 (in read, write has always crc)
//     [0]:    poll (in read)

#define RESP_R1   (1<<7)
#define RESP_R2   (1<<6)
#define RESP_R3   (1<<5)
#define RESP_R6   (1<<4)
#define RESP_R7   (1<<7) // using R1 for this one
#define RESP_BUSY (1<<3)
#define RESP_R1B  (RESP_R1 | RESP_BUSY)

#define CMD0    (0x0000  |      0  |      0   | 0)  // RESET
#define MMCCMD1 (0x0000  | ( 1<<8) | RESP_R3  | 1)  // MMC SEND_OP_COND, R3
#define CMD2    (0x0000  | ( 2<<8) | RESP_R2  | 3)  // ALL_SEND_CID, R2
#define CMD3    (0x0000  | ( 3<<8) | RESP_R6  | 3)  // SEND_RCA, R6
#define MMCCMD3 (0x8000  | ( 3<<8) | RESP_R1  | 3)  // SET_RCA, R1 <<<< MMC
#define MMCCMD6 (0x4000  | ( 6<<8) | RESP_R1B | 3)  // SWITCH
#define ACMD6   (0x4000  | ( 6<<8) | RESP_R1  | 3)  // SET_BUS_WIDTH
#define CMD7    (0x8000  | ( 7<<8) | RESP_R1  | 3)  // SELECT(/DESELECT)_CARD
#define CMD7_0  (0x0000  | ( 7<<8) |      0   | 0)  // (SELECT/)DESELECT_CARD
#define CMD8    (0x4000  | ( 8<<8) | RESP_R7  | 3)  // SEND_IF_COND, R7
#define MMCCMD8 (0x4000  | ( 8<<8) |       0  | 3)  // SEND_EXTENDED_CSD
#define CMD9    (0x8000  | ( 9<<8) | RESP_R2  | 3)  // SEND_CSD
#define CMD10   (0x8000  | (10<<8) | RESP_R2  | 3)  // SEND_CID
#define CMD12   (0x0000  | (12<<8) | RESP_R1B | 3)  // STOP_TRANSMISSION
#define CMD13   (0x8000  | (13<<8) | RESP_R1  | 3)  // SEND_STATUS
#define ACMD13  (0x0000  | (13<<8) | RESP_R1  | 3)  // SD_STATUS
#define CMD15   (0x8000  | (15<<8) |      0   | 0)  // GO_INACTIVE_STATE
#define CMD16   (0x4000  | (16<<8) | RESP_R1  | 3)  // SET_BLOCKLEN, in bytes
#define CMD17   (0x4000  | (17<<8) |      0   | 3)  // READ_SINGLE_BLOCK, R1
#define CMD18   (0x4000  | (18<<8) |      0   | 3)  // READ_Multiple_BLOCK, R1
#define ACMD23  (0x4000  | (23<<8) | RESP_R1  | 3)  // SET_WR_BLK_ERASE_COUNT
#define CMD24   (0x4000  | (24<<8) | RESP_R1  | 3)  // WRITE_BLOCK, R1
#define CMD25   (0x4000  | (25<<8) | RESP_R1  | 3)  // WRITE_MULTIPLE_BLOCK, R1
#define CMD32   (0x4000  | (32<<8) | RESP_R1  | 3)  // ERASE_WR_BLOCK_START, R1
#define CMD33   (0x4000  | (33<<8) | RESP_R1  | 3)  // ERASE_WR_BLOCK_END, R1
#define CMD38   (0x0000  | (38<<8) | RESP_R1  | 3)  // ERASE, R1b
#define ACMD41  (0x0000  | (41<<8) | RESP_R3  | 1)  // SD_APP_OP_COND, no crc7
#define ACMD51  (0x0000  | (51<<8) | RESP_R1  | 3)  // SEND_SCR
#define CMD55   (0x8000  | (55<<8) | RESP_R1  | 3)  // APP_CMD, if RCA use it





// 
// vs1005 Generate SD clocks (32) with cmd&data high
// 
auto void SDClock32(void) {
  WaitSdReady();
  SD_CMD_DAT_HIGH();  // gpio mode, drive '1'
  // leaves memory intact
  PERIP(SD_PTR) = 1536;                // above memory array
  PERIP(SD_LEN) = 4;                   // number of bytes to read
  PERIP(SD_CF)  = SD_CF_ENA|SD_CF_READSEL|SD_CF_CMDSEL;// ena,rd,cmd, NO crc7+poll
  WaitSdReady();
}






// 
// vs1005G Data read/write function for SD
// ALWAYS 4-BIT BUS IF POSSIBLE
// Input parameter is byte count bcnt where bcnt[15] has special meaning
//    if bcnt[15] == 1 :  Data Read
//    if bcnt[15] == 0 :  Data Write
// NOTE: Control bits  (continued data TX/RX op)
//         12 : SD_NO_CRCSEND   -- crc not sent after data in tx/rx
//         11 : SD_NO_CRCRST    -- crc not reset at op start in tx/rx
//          4 : SD_SKIPSTART    -- no DATA start bit in tx/rx
//          3 : SD_NODSTOP      -- no data stop bit in tx/rx
//       can not be modified through this function
// 
// NOTE: bcnt does not include crc16 (2 bytes)
// 
#ifndef SDSDR
u_int16 SDDataRXTX(register SD_DEVICE *dev, register u_int16 bcnt,
		   register u_int16 isRead) {
  SD_HWINFO *hwi = &(dev->hwInfo);
  u_int32 t = ReadTimeCount();

  PERIP(SD_PTR)   = SDSD_PERIP_ADDR;              // start address for read data
  PERIP(SD_LEN)   = bcnt;           // number of bytes to transfer
  //               [4bit], [rd/wr]+dat, crc16+poll
  // PERIP(SD_CF) = (hw->flags&SDSD_F_4BIT) |  // [4bit], [rd/wr]+dat, crc16+poll
  PERIP(SD_CF)    = (hwi->flags&SDSD_F_4BIT) |
    (isRead ? SD_CF_READSEL : 0) |                  // SD_CF[6]:  RD = 1, WR = 0
    (SD_CF_CRC16|SD_CF_POLL);             // should not matter if POLL in tx
  SD_DAT_Z();                                 // data line(s) to perip mode
  do {
    PERIP(SD_CF) |= SD_CF_ENA;               // ena HIGH
    WaitSdReady();
    // Timeout 0.5s, should be enough for everyone
  } while ((PERIP(SD_ST)&SD_ST_ANY_ERR) && ReadTimeCount()-t < TICKS_PER_SEC/2);
  if (PERIP(SD_ST)&SD_ST_ANY_ERR) {
    return 1;
  }
  // NOT Checking DAT0, NOT CHECKING CMD BREAK any more
  // No clocks here, could be MULTIPLE read
  // TX = write op, wait for busy signaling in dat0
  if (!isRead) {               // i.e. WRITE
#ifdef CHECK_WRITE_CRC_ERROR
    int wordStorage = 0xff00;
    int needResp = 1;
#endif /* CHECK_WRITE_CRC_ERROR */
    u_int16 ffMask = 0xFFFFU;

    t = ReadTimeCount();
    SD_DAT3_1HIGH();                      // data bus to gpio mode, drive '1'
    // SD_DAT0_Z;                           // dat0 still input
    while (1) {
      u_int16 timeSpent;
#ifdef CHECK_WRITE_CRC_ERROR
      /* A fully error-checking routine which reads and analyzes the CRC reply
	 token that is read from DAT0 as 1-bit data.
	                       busy  CRC 0=busy     1=ready
	Reply is in format: ...1111 0xxx1000 ... 0001111111
	xxx = CRC = 010 = CRC ok
	xxx = CRC = 101 = CRC error
	xxx = CRC = 111 = write failed
      */
      u_int16 tmpdata[1];
      PERIP(SD_PTR) = SDSD_PERIP_ADDR; // start address for read data
      PERIP(SD_LEN) = 1;                  // number of bytes to read
      PERIP(SD_CF) = SD_CF_ENA|SD_CF_READSEL|SD_CF_DATASEL|SD_CF_NOSTOPB;
      WaitSdReady();
      xp_fiford(tmpdata, SDSD_PERIP_ADDR, 1);        // *dbuf, addr, wcnt
#if 0
      printf("%02x", tmpdata[0]>>8);
#endif
      if (needResp) {
	int bits = 8;
	wordStorage |= tmpdata[0]>>8; /* Put new bits to 8 LSb's */
	while (bits-- && needResp) {
	  wordStorage <<= 1;
	  if (!(wordStorage & 0x8000U)) { /* When MSb = 0, there is a reply */
	    /* 0x2xxx = crc ok, 0x5xxx = crc fail, others general fail */
	    needResp = 0;
	    if ((wordStorage & 0xF000U) != 0x2000) {
	      thereHaveBeenWriteErrors = S_ERROR;
#if 0
	      fprintf(stderr, "#%04x\n", wordStorage);
#endif
	    }
	  }
	}
      } else { /* !needResp */
	if (PERIP(SD_ST) & SD_ST_DAT0) {              /* DAT0 HIGH */
	  /* Now there are guaranteed 4 extra clocks, but spec requires 8.
	     So offer a few more (32 is overkill, but certainly enough.) */
	  SDClock32();
	  //	fprintf(stderr, "\n");
	  printf("\n");
	  break;
	}
      } /* !needResp */
#else /* !CHECK_WRITE_CRC_ERROR */
      u_int16 tmpdata[1];
      PERIP(SD_PTR) = SDSD_PERIP_ADDR/*1536*/;               // above memory array
      PERIP(SD_LEN) = 2;                  // number of bytes to read
      PERIP(SD_CF) = SD_CF_ENA|SD_CF_READSEL|SD_CF_DATASEL|SD_CF_NOSTOPB;
      WaitSdReady();
      xp_fiford(tmpdata, SDSD_PERIP_ADDR, 1);        // *dbuf, addr, wcnt
#if 0
      printf("%04x=%04x:%04x ", PERIP(SD_ST), tmpdata[0], tmpdata[1]);
#endif
      ffMask &= tmpdata[0];
      if (ffMask != 0xFFFFU && (tmpdata[0] & 0x1ff) == 0x1ff) {
	/* 0x1ff forces us to send at least 8 clock cycles after DAT0 = high */
#if 0
	printf("\n");
#endif
	break;
      }
#endif /* !CHECK_WRITE_CRC_ERROR */
      timeSpent = (u_int16)(ReadTimeCount()-t);
      if (timeSpent > 1*TICKS_PER_SEC) {    // Enough?, Some cards are very slow
	return 2;                         // give up, data not coming
      }
      /* We didn't get out? Then save some CPU for the other processes! */
      ReleaseHwLocksBIP(HLB_NONE, HLIO_NONE, HLP_XPERIP_IF);
      if (timeSpent > 1) {
	Delay((timeSpent > 5) ? 5 : timeSpent-1);
      }
      ObtainHwLocksBIP(HLB_NONE, HLIO_NONE, HLP_XPERIP_IF);
    } /* while (1) */
  } /* if (!isRead) */

  SD_DAT_HIGH();                              // data bus to gpio mode, drive '1'
  return 0;                               // OK
}
#endif /* !SDSDR */






auto void SDDataRX1(register SD_DEVICE *dev) {
  SD_HWINFO *hwi = &(dev->hwInfo);
  PERIP(SD_PTR) = SDSD_PERIP_ADDR;              // start address for read data
  PERIP(SD_LEN) = 512;           // number of bytes to transfer
  PERIP(SD_CF) = (hwi->flags&SDSD_F_4BIT) |
    (SD_CF_READSEL) |                  // SD_CF[6]:  RD = 1, WR = 0
    (SD_CF_CRC16|SD_CF_POLL);
  SD_DAT_Z();                                 // data line(s) to perip mode
  PERIP(SD_CF) |= SD_CF_ENA;               // ena HIGH
}







// 
// vs1005G Command write/read function for SD
// 
#ifdef SDSDMONO
auto u_int16 SDCmd(register SD_DEVICE *dev, register u_int16 sd_cmd,
		     register u_int32 arg);
#else /* "!SDSDMONO */
#include "lcd.h"
auto u_int16 SDCmd(register SD_DEVICE *dev, register u_int16 sd_cmd,
		     register u_int32 arg) {
  SD_HWINFO *hwi = &(dev->hwInfo);
  {
    u_int16 tmpdata[3];
    //========================= WRITE CMD =========================
    if (sd_cmd&0x8000U) { // use RCA in argument (13, 9, 10, 51, 55, 7)
      arg = (u_int32)hwi->rca << 16;   // RCA
    }
    tmpdata[0] = 0x4000|(0x7F00&sd_cmd)| (u_int16)(arg>>24);  // bytes 0,1
    tmpdata[1] = (u_int16)(arg>>8);         // bytes 2,3
    tmpdata[2] = (u_int16)(arg<<8);// byte 5 be replaced w/ crc7+stop by HW

    xp_fifowr(tmpdata, SDSD_PERIP_ADDR, 3);     // *dbuf, mem addr, wcnt
  }
  SDClock32();                        // NOTE: do some clocking before wr
  PERIP(SD_PTR) = SDSD_PERIP_ADDR;            // start address for read data
  PERIP(SD_LEN) = 6;                   // number of bytes to write
  SD_CMD_Z();                             // cmd line to perip mode
  PERIP(SD_CF) = SD_CF_ENA|SD_CF_WRITESEL|SD_CF_CMDSEL|SD_CF_CRC7;    // ena+rd+cmd, crc7
  WaitSdReady();
  SD_CMD_HIGH(); // gpio mode, drive '1'

#if 0
  // This part is not necessary here, for convenience,
  // no resp to CMD0, 15
  if ( 
#ifdef SUPPORT_UNUSED_CMDS
      (sd_cmd == CMD15) ||
#endif
      (sd_cmd == CMD0)) {
    // RESET or GO_INACTIVE_STATE
    hwi->rca = 0; // RCA
    return 0;                           // ok
  }
#endif

  //========================= READ RESPONSE =========================
  if (sd_cmd & RESP_BUSY) {
    SD_DAT0_Z();                                  // POLL DAT0 FOR BUSY
  }
#if 0
 // TBD: CMD17, CMD18 should be here ???     // Data reads
  if ((sd_cmd == ACMD51) || (sd_cmd == ACMD13)) {   // Skipping response
    u_int16 res;
    SD_CMD_Z();
    // cmd line to perip mode
    res = SDDataRXTX(dev, (sd_cmd == ACMD51) ? 8 : 64, 1);    // 64 bytes, SD_STATUS
    if (res) {                              // cmd break, no crc
      return res;                           // ERROR, no point to continue
    } else {
      SDClock32();
      //SD_CMD_HIGH();                        // gpio mode, drive '1'
      return 0;                             // OK
    }
  }
#endif
	
  PERIP(SD_PTR) = SDSD_PERIP_ADDR;                    // start address for read data
  PERIP(SD_LEN) = 6;                           // number of bytes to read
  if (
#ifdef SUPPORT_UNUSED_CMDS
      (sd_cmd == CMD17) ||
#endif
      (sd_cmd == CMD18) || (sd_cmd == MMCCMD8)) {
    // read until first byte received
    PERIP(SD_CF) = SD_CF_READSEL|SD_CF_CMDSEL|SD_CF_POLL;  // rd+cmd, pollstart
  } else {
    /**** Onko &3 oikein, vai pitäisikö olla &2? */
    PERIP(SD_CF) = SD_CF_READSEL|SD_CF_CMDSEL|(sd_cmd&0x3);     // rd+cmd, crc7, pollstart
  }
#if 0
  if (sd_cmd & (RESP_R2)) {                    // R2: 1 + 16 bytes
    PERIP(SD_LEN) = 1;                         // number of bytes to read
    PERIP(SD_CF) = SD_CF_READSEL|SD_CF_CMDSEL|(sd_cmd&0x1); // rd+cmd, pollstart
  }
#endif
  SD_CMD_Z();                                     // perip mode



#if 0
#define lazy_user_or_not_possible_to_weld_a_pull_up_resistor
#endif
#ifdef  lazy_user_or_not_possible_to_weld_a_pull_up_resistor
  // 
  // If this code is active, then MMC card driver drives against
  // VS1005 pad!!!!!!
  // 
  // NOTE! NOTE! NOTE! NOTE! NOTE! NOTE! NOTE! 
  // Remove this when pull-up in cmd line
  //   (MMC card is in open-collector mode when in identification.
  //    After standby the drivers are in pushd-pull mode)
  // This issue doesn't exist with SD cards.
  // 
  if ((sd_cmd == MMCCMD1) ||
      ((hwi->flags & SDSD_F_MMC) && (sd_cmd == CMD2 || sd_cmd == CMD3))) {
    SD_CMD_HIGH();  // MMC card still in push pull mode
  }
#endif

  PERIP(SD_CF) |= SD_CF_ENA;                  // ena (+ params)
  WaitSdReady();
  if (
#ifdef SUPPORT_UNUSED_CMDS
      (sd_cmd == CMD17) ||
#endif
      (sd_cmd == CMD18) || (sd_cmd == MMCCMD8)) {
    return 0; // go reading data, skip rest of RESP
    // NOTE: LEAVING CMD FLOATING
  }

  if (!((PERIP(SD_ST)&(SD_ST_ANY_ERR|SD_ST_DAT0|SD_ST_CMDBRK)) == SD_ST_DAT0)
      && !(sd_cmd & RESP_BUSY)) {
    return 2;                                 // ERROR, no point to continue
  }

#if 0
  if (sd_cmd & (RESP_R2)) {                    // R2: 1 + 16 bytes
    // continue reading
    if (!(PERIP(SD_ST)&(SD_ST_ANY_ERR|SD_ST_CMDBRK))) {         // no errors
      // now turn crc7 ON for CID's/CSD's internal crc check
      PERIP(SD_LEN) = 16;                      // bytes to read = 128 bits
      PERIP(SD_CF) = SD_CF_ENA|SD_CF_READSEL|SD_CF_CMDSEL|(sd_cmd&SD_CF_CRC7); // ena+rd+cmd, crc7, NOpollstart
      // Pointer is now at SDSD_PERIP_ADDR+1 (one byte op done)
      WaitSdReady();
      if ((PERIP(SD_ST)&(SD_ST_ANY_ERR|SD_ST_DAT0|SD_ST_CMDBRK)) != SD_ST_DAT0) { // No dat0 busy possible
	return 3;                             // ERROR, no point to continue
      }
    }
  }
#endif
  SD_CMD_HIGH();                                  // gpio mode, drive '1'
  if (sd_cmd & RESP_BUSY) {
    do {
      PERIP(SD_PTR)   = 1536;                  // above memory array
      PERIP(SD_LEN)   = 32;                    // number of bytes to write
      PERIP(SD_CF)   =  SD_CF_ENA|SD_CF_WRITESEL|SD_CF_CMDSEL;  // ena+rd+dat
      WaitSdReady();
    } while (!(PERIP(SD_ST) & SD_ST_DAT0));
    SD_DAT0_HIGH();
  }
  SDClock32();                                // NOTE: some clocking
	
	
  //========================= GATHER DATAS =========================
  // MUST BE IF THIS FAR: if ( (PERIP(SD_ST)&0x005F) == 0) {      // no errors
  {
    u_int16 tmpdata[9];
    // read all 17 bytes (first three words again)
    xp_fiford(tmpdata, SDSD_PERIP_ADDR, 9);        // *dbuf, addr, wcnt
    // first byte ([135:128]) should be 0x3F. It is NOT checked.
    // Status is returned in RESP 1 at least.
    // Storing it here for all RESPONSES
    //   devSdSd.hwInfo.statusHigh          // status high / arg high
    //   devSdSd.hwInfo.statusLow           // status low  / arg low
#if 0
    // Result is not used anywhere, so field removed.
    MemCopyPackedBigEndian((void*)(&(hwi->status)),0,tmpdata, 1, 4);
#endif
    //
#if 0
    if (sd_cmd == CMD9 ) {                  // SEND_CSD = CMD9, RESP_R2
      // CSD = Card Spesific Data
      // Card size
      // Ver 1.0 CSD:
      //       READ_BL_LEN : [83:80],  4 bits
      //       C_SIZE      : [73:62], 12 bits
      //       C_SIZE_MULT : [49:47],  3 bits
      //       SD card size =  (READ_BL_LEN<<9) * (C_SIZE+1) *
      //                       2^(c_size_mult+2)
      //
      //    Ver 2.0 CSD:
      //       C_SIZE      : [69:48], 22 bits
      //       SD card size =  (c_size+1) * 512kB
      //
      // Some card's CSD:
      //               1        2       3     4     5     6     7      8
      //               127:112 111:96  95:80 79:64 63:48 47:32 31:16  15:0
      //  256M CSD:   005d     0132    1359  80e3  76d9  cfff  1640   004f
      //  4G   CSD:   400e     0032    5b59  0000  1e5c  7f80  0a40   40df
      //
      //  4G MMC:3f00 905e     002a    1f59  83d3  edb6  03ef  9640   0045 


      u_int16 csd_struct  = (tmpdata[1]>>14);          // two MSBs
      u_int32 sd_size = 0;
      if ( (csd_struct == 0) ||                       // CSD ver 1.0
	   (csd_struct > 0 && (hwi->flags&SDSD_F_MMC)) ) {      // MMC CSD ver 1.2
	u_int16 read_bl_len = tmpdata[3] & 0x000F;
	u_int16 c_size      = ((tmpdata[4]<<2)|(tmpdata[5]>>14))&0x0FFF;
	u_int16 c_size_mult = ((tmpdata[5]<<1)|(tmpdata[6]>>15))&0x0007;
	sd_size = (u_int32)((read_bl_len&0x7)<<9) * (c_size+1);    // 2^11 + 2^10 + 2^9
	sd_size <<= c_size_mult+2;  // Bytes
	sd_size >>= 10;                               // KILO Bytes
      } else if (csd_struct == 1) {                   // CSD ver 2.0
	// c_size = CSD[69:48]
	sd_size = ((((u_int32)(tmpdata[4]&0x3F)<<16) | tmpdata[5]) + 1) * 512;
      }
      hwi->size = sd_size;   // Store SD card size in KiB
    }
    // if ( sd_cmd == ACMD41 ) {       //  = SD_APP_OP_COND = ACMD41, RESP_R3
    if (sd_cmd & RESP_R3) {            // ACMD41 or (MMC)CMD1
      // R3: OCR reg in [39:8]
      // first byte ([47:40]) should be 0x3F. It is NOT checked.
      if (tmpdata[0] & 0x80) {   // power ok 0x80, SDHC bit is 0x40
	hwi->flags |=
	  (((tmpdata[0]>>6)&1) << SDSD_F_HC_B) | (1<<SDSD_F_OCR31_B);
      }
    }
    if (sd_cmd == CMD3) {      // SEND_RELATIVE_ADDRR6: RCA reg in [39:24]
      // NOTE: CMD3 (SEND_RCA), NOT MMCCMD3 (SET_RCA)
      // first byte ([47:40]) should be 0x03. It is NOT checked.
      hwi->rca  = (tmpdata[0]<<8) | (tmpdata[1]>>8);  // RCA is in bits [39:24]
      // some status bits in [23:8] skipped
      // hwi->statusHigh = (tmpdata[1]<<8) | (tmpdata[2]>>8);
    }
#endif
  }
  return 0;  // ok
}
#endif /* !SDSDMONO */
	








#define VS23S010_BLOCKS_IN_ONE_CHIP (2*128)

struct VS23S010 {
  u_int16 writeRP, writeWP;
  u_int32 writeBlockR, writeBlockW;
  DEVICE *dev;
} vs23s;

ioresult DevSdSdBlockWriteRaw(register __i0 DEVICE *stdDev, u_int32 firstBlock,
			      u_int16 blocks, u_int16 *data);

struct TaskAndStack *sdTask = NULL;
u_int16 quitSdTask = 0;
u_int16 sdTaskBuf[256];

void SdTask(void) {
  u_int32 startTime = 0;
  u_int16 continuousTime = 0;
#if 0
  u_int32 nothingToDoTimes = 0;
#endif
  while (!quitSdTask) {
    Forbid();
    if (vs23s.writeRP != vs23s.writeWP) {
      Permit();
#if 0
      nothingToDoTimes = 0;
#endif
      if (!startTime) {
	startTime = ReadTimeCount();
#if 0
	printf("$");
#endif
      }
      continuousTime = (u_int16)(ReadTimeCount()-startTime);
      if (continuousTime < 2) {
	u_int16 newWriteRP;
	s_int16 aFill, fill;

#if 0
	printf("%03x:", aFill);
#endif
	ReadVS23S010(&byteBus, sdTaskBuf, 512*(s_int32)vs23s.writeRP, 512);
	DevSdSdBlockWriteRaw(vs23s.dev, vs23s.writeBlockR, 1, sdTaskBuf);

	/* update pointers */
	newWriteRP = vs23s.writeRP+1;
	if (newWriteRP >= vs23Size) {
	  newWriteRP = 0;
	}
	vs23s.writeRP = newWriteRP;
	vs23s.writeBlockR++;
	fill = vs23s.writeWP - vs23s.writeRP;
	if (fill < 0) {
	  fill += vs23Size;
	}
	vs23CurrFill = fill;

#if 0
	printf("(%d:", continuousTime);
#endif
      } else {
#if 0
	printf("%%");
#endif
	Delay(2); /* Share 2 ms for every 2 ms used */
	startTime = 0;
      }
    } else {
#if 0
      printf(".");
#endif
      /* Nothing to write to SD: wait 5 ms */
      vs23s.writeBlockR = vs23s.writeBlockW;
      Permit();
      startTime = 0;
      Delay(5);
    }
  }
}

// ====================================================================
// ====================================================================
// ====================================================================

ioresult DevSdSdCreate(register __i0 DEVICE *dev, void *name,
		       u_int16 extraInfo) {
  int i;

  if (sdTask) {
    return S_ERROR;
  }

  quitSdTask = 0;
  memset(&vs23s, 0, sizeof(vs23s));
  AddSymbol("_vs23MaxFill", (void *)0x13f9, (int)(&vs23MaxFill));
  AddSymbol("_vs23CurrFill", (void *)0x13f9, (int)(&vs23CurrFill));
  AddSymbol("_vs23Size", (void *)0x13f9, (int)(&vs23Size));

  {
    static const devByteBusHwInfo myHwInfo = {
      (void __mem_y *)NF_CF,	/* Base register */
#if 0
      0x21,	/* csPin = TDI */
#else
      0x20,	/* csPin = TMS */
#endif
      0,	/* ioChannel (0 since csPin != 0xFFFF) */
      15000,	/* maxClockKHz */
    };
    DevByteBusCreate(&byteBus, &myHwInfo, 0);
  }

  ObtainHwLocksBIP(HLB_0<<SDSD_PERIP_BUFNO, HLIO_SD, HLP_SD);
  // printf("DevSdSdCreate kutsuttu\n");
  memcpyYX(dev, &devSdSdDefaults, sizeof(DEVICE));
  dev->deviceInstance = __nextDeviceInstance++;

  /* Ignore result of IOCTL: even if there is no SD card inserted at the
     moment, device creation is ok, and if a card is inserted later, it
     can be accessed through this handle. */
  dev->Ioctl(dev, IOCTL_RESTART, 0);

  for (i=4; i>0; i--) {
    sdTaskBuf[0] = i;
    sdTaskBuf[1] = 0xf4de;
    sdTaskBuf[2] = 0x17f3;
    WriteVS23S010(&byteBus, sdTaskBuf, i*(128L*1024)-512, 512);
  }
  vs23Size = 0;
  for (i=1; i<=4; i++) {
    ReadVS23S010(&byteBus, sdTaskBuf, i*(128L*1024)-512, 512);
    if (sdTaskBuf[0] == i && *((u_int32 *)(sdTaskBuf+1)) == 0x17f3f4de) {
      vs23Size = i*256;
    }
  }

  if (!vs23Size) {
    SysReport("W: no VS23S0x0, no buffering");
    dev->BlockWrite = DevSdSdBlockWriteRaw;
  } else if (!(sdTask = CreateTaskAndStack(SdTask, "SDSD23", 512, 20))) {
    return S_ERROR;
  }

  return S_OK;
}

ioresult DevSdSdDelete(register __i0 DEVICE *dev) {
  if (dev) {
    ObtainHwLocksBIP(HLB_NONE, HLIO_NONE, HLP_XPERIP_IF);
    PERIP(XP_ST) &= ~XP_ST_SD_INT;
    ReleaseHwLocksBIP(HLB_0<<SDSD_PERIP_BUFNO, HLIO_SD, HLP_SD|HLP_XPERIP_IF);
  }
  if (sdTask) {
    quitSdTask = 1;
    while (vs23s.writeRP != vs23s.writeWP ||
	   (sdTask->task.tc_State && sdTask->task.tc_State != TS_REMOVED)) {
      Delay(1);
    }
  }
  return S_OK;
}


const char *DevSdSdIdentify(register __i0 void *dev,
			    char *buf, u_int16 bufsize) {
  static char identString[] = "SD + VS23S0x0";
  identString[11] = '0'+vs23Size/256;
  return identString;
}


auto ioresult DataStop(register __i2 SD_DEVICE *dev) {
  SD_HWINFO *hwi = &(dev->hwInfo);

  WaitSdReady();
  hwi->nextBlock = -1;
  SD_DAT_Z();                                // data line(s) to perip mode
  if (SDCmd(dev, CMD12, 0)) {
    SysError("Data stop fail");  // ERROR, no point to continue
    return S_ERROR;
  }
  SDClock32();                           // NOTE: some clocking

  hwi->flags &= ~(SDSD_F_MULTIPLE_BLOCK_WRITE|SDSD_F_MULTIPLE_BLOCK_READ);

  return S_OK;
}



ioresult DevSdSdBlockRead(register __i0 DEVICE *stdDev, u_int32 firstBlock,
			  u_int16 blocks, u_int16 *data) {
  SD_DEVICE *dev = (SD_DEVICE *)stdDev;
  SD_HWINFO *hwi = &(dev->hwInfo);
  ioresult errCode = S_OK;
  char *errStr = NULL;
#if 0
  u_int16 *oData = data;
  printf("DevSdSdBlockRead (%ld) @%4x ", firstBlock, oData);
#endif
  s_int16 skipBlocks;
#if 0
  if (PERIP(1) == 12345) {
    printf("DevSdSdBlockRead (%ld), flags %04x\n",
	   firstBlock, dev->flags);
  }
#endif

  if (!__F_CAN_SEEK_AND_READ(dev)) {
    hwi->nextBlock = -1;
    //    fprintf(stderr, "SEEK ERROR\n");
    return SysError("Seek");
  }

//  if (firstBlock < 39000) printf("#R1\n");
  ObtainHwLocksBIP(HLB_USER_3, HLIO_NONE, HLP_XPERIP_IF);
//  if (firstBlock < 39000) printf("#R2\n");

  /* Make sure nothing is unwritten before attempting a read. Otherwise
     we could accidentally read something that is still waiting in the
     write queue. */
  while (vs23s.writeRP != vs23s.writeWP) {
    ReleaseHwLocksBIP(HLB_USER_3, HLIO_NONE, HLP_XPERIP_IF);
    Delay(5);
    ObtainHwLocksBIP(HLB_USER_3, HLIO_NONE, HLP_XPERIP_IF);
  }

  if ((s_int32)hwi->nextBlock == -1 ||
      firstBlock - hwi->nextBlock > MULTIPLE_BLOCK_READ_SKIP_LIMIT) {
    skipBlocks = -1;
  } else {
    skipBlocks = (s_int16)firstBlock - (s_int16)hwi->nextBlock;
  }

  if (((hwi->flags & SDSD_F_MULTIPLE_BLOCK_READ) &&
       (skipBlocks < 0 || !blocks)) ||
      (hwi->flags & SDSD_F_MULTIPLE_BLOCK_WRITE)) {
    /* Close down earlier multiple block read. */
#if 0
    printf("  Close old R/W, bl %ld\n", hwi->nextBlock);
#endif
    skipBlocks = 0;
    if (errCode = DataStop(dev)) {
      goto finally;
    }

    /* Keep SD speed always at 25 MHz or less. */
    PERIP(SD_ST) = PERIP(SD_ST) & ~(SD_ST_WAITSTATES_MASK) |
      ((u_int16)(clockSpeed.cpuClkHz/50000000)*SD_ST_WAITSTATES);
  }

  //  if (firstBlock < 39000) printf("#R3\n");

  if (blocks && !(hwi->flags & SDSD_F_MULTIPLE_BLOCK_READ)) {
#if 0
    printf("  Init new R, bl %ld\n", firstBlock);
#endif
    if (SDCmd(dev, CMD18,
		(hwi->flags & SDSD_F_HC) ? firstBlock : firstBlock<<9)) {
      hwi->nextBlock = -1;
      errStr = "Read Start Fail";  // ERROR, no point to continue
      errCode = S_ERROR;
      goto finally;
    }
    SD_CMD_Z();    // There is a response coming (cmd break) 
    hwi->flags |= SDSD_F_MULTIPLE_BLOCK_READ;
    hwi->nextBlock = firstBlock;
    SDDataRX1(dev);
  }

  // reading blocks of data
  if (skipBlocks > 0) {
    blocks += skipBlocks;
  }
  while (blocks) {
#if 0
    printf("  Read %ld\n", firstBlock);
#endif


    u_int32 t = ReadTimeCount();

    WaitSdReady();

    while ((PERIP(SD_ST)&SD_ST_ANY_ERR) && ReadTimeCount()-t < TICKS_PER_SEC/2){
      PERIP(SD_CF) |= SD_CF_ENA;               // ena HIGH
      WaitSdReady();
    }

    if (PERIP(SD_ST) & SD_ST_ANY_ERR) {
      SDCmd(dev, CMD12, 0);		// try to get back on line
      //return ( (blocks<<4) | 2); }	// ERROR, no point to continue
      hwi->nextBlock = -1;
      errStr = "Data Fail";	// ERROR, no point to continue
      errCode = S_ERROR;
      goto finally;
    }

    SD_CMD_DAT_HIGH();	// data bus to gpio mode, drive '1'
    // it should be gone now
    xp_fiford(data, SDSD_PERIP_ADDR, 256);       // *dbuf, addr, wcnt
    SDDataRX1(dev);	// Start next transfer
    if (--skipBlocks < 0) {
      data += 256;
    }
    hwi->nextBlock++;
    blocks--;
  }

#if 0
  if (hwi->nextBlock == 1) {
    int i;
    printf("### FIRSTBLOCK=0\n");
    for (i=0; i<256; i++) {
      printf(" %04x", data[i-256]);
      if ((i&15) == 15) {
	printf("\n");
      }
    }
  }
#endif

 finally:
  //  if (firstBlock < 39000) printf("#R4\n");
  ReleaseHwLocksBIP(HLB_USER_3, HLIO_NONE, HLP_XPERIP_IF);
  if (errStr) {
    SysError(errStr);
  }
  //  fprintf(stderr, " %d, dev[3] = %04x\n", errCode, USEX(0x88f));
  return errCode;                             // OK
}



#ifdef SDSDR
ioresult DevSdSdBlockWriteRaw(register __i0 DEVICE *stdDev, u_int32 firstBlock,
			      u_int16 blocks, u_int16 *data) {
  SysError("Read-only file system");
  return S_ERROR;
}
#else /* !SDSDR */
ioresult DevSdSdBlockWriteRaw(register __i0 DEVICE *stdDev,  u_int32 firstBlock,
			      u_int16 blocks, u_int16 *data) {
  SD_DEVICE *dev = (SD_DEVICE *)stdDev;
  SD_HWINFO *hwi = &(dev->hwInfo);
  ioresult errCode = S_ERROR;
  char *errStr = NULL;
  static int firstTime = 1;
  // u_int16 errorCount=0;

#if 0
  if (firstBlock < 39000)
  printf("DevSdSdBlockWriteRaw (%ld) @%4x %04x %04x %04x %04x %04x %04x %04x %04x\n",
	 firstBlock, data, data[0], data[1], data[2], data[3],
	 data[4], data[5], data[6], data[7]);
#endif
#if 0
  printf("SdSdBlWrite(%ld, %d)\n",firstBlock,blocks);
#endif

  //  printf("#W1\n");
  ObtainHwLocksBIP(HLB_USER_3, HLIO_NONE, HLP_XPERIP_IF);
  //  printf("#W2\n");

  if (!__F_CAN_SEEK_AND_READ(dev)) {
    hwi->nextBlock = -1;
    errStr = "Seek";
    goto finally;
  }

  if (thereHaveBeenWriteErrors) {
    errStr = "Write";
    goto finally;
  }

  if (((hwi->flags & SDSD_F_MULTIPLE_BLOCK_WRITE) &&
       (firstBlock != hwi->nextBlock || !blocks)) ||
      (hwi->flags & SDSD_F_MULTIPLE_BLOCK_READ)) {
    /* Close down earlier multiple block write. */
#if 0
    printf("  Close old W/R, bl %ld\n", hwi->nextBlock);
#endif
    if (errCode = DataStop(dev)) {
      goto finally;
    }
  }

  //  printf("#W3\n");

  if (blocks && !(hwi->flags & SDSD_F_MULTIPLE_BLOCK_WRITE)) {
    // WRITE_MULTIPLE_BLOCK
#if 0
    if (firstTime) {
      firstTime = 0;
#if 0
      /* Give a hint for how many blocks have been invalidated, and are
	 thus free to be erased. Doesn't seem to make any difference with
	 VLSI's 4 GB Kingston SD cards (2017-02-08) */
      SDCmd(dev, CMD55, 0);
      SDCmd(dev, ACMD23, 2048L*1024); /* 1 GiB */
#else
      if (SDCmd(dev, CMD55, 0) || SDCmd(dev, ACMD23, 2048L*1024)) {
	printf("### FAILED\n");
      } else {
	printf("### SUCCESS\n");
      }
#endif
    }
#endif

#if 0
    printf("  Init new W, bl %ld\n", firstBlock);
#endif
    if (SDCmd(dev, CMD25,
	      (hwi->flags & SDSD_F_HC) ? firstBlock : firstBlock<<9)) {
      // ERROR, no point to continue
      errStr = "Multiple block write";
      goto finally;
    }
    // NOTE: IT SEEMED THAT CMD24/CMD25 COMMAND REQUIRED SOME CLOCKS AFTERWARDS.
    //       THEY ARE NOW FED IN SDCmd() -FUNCTION
    hwi->flags |= SDSD_F_MULTIPLE_BLOCK_WRITE;
    hwi->nextBlock = firstBlock;
  }

  //  printf("#W4\n");

  while (blocks) {
    /* Short evaluation */
    if (PERIP(SD_CF) & SD_CF_ENA) {
      goto finally;
    }

    //    printf("#W5\n");

    xp_fifowr(data, SDSD_PERIP_ADDR, 256);      // *dbuf, mem addr, wcnt

    //    printf("#W6\n");
  
    if (SDDataRXTX(dev, 512, 0)) {       // Write = TX
      SDCmd(dev, CMD12, 0);                    // try to get back on line
      errCode = (blocks<<4) | 2;            // ERROR, no point to continue
      goto finally;
    }

    //   printf("#W7\n");

    data += 256;
    hwi->nextBlock++;
    blocks--;      
  }

  errCode = S_OK;
 finally:
  //  printf("#W9\n");
  ReleaseHwLocksBIP(HLB_USER_3, HLIO_NONE, HLP_XPERIP_IF);
  if (errStr) {
    SysError(errStr);
  }
  if (errCode) {
    thereHaveBeenWriteErrors = S_ERROR;
  }
  return errCode;
}
#endif /* !SDSDR */


ioresult DevSdSdBlockWrite(register __i0 DEVICE *stdDev,  u_int32 firstBlock,
			   u_int16 blocks, u_int16 *data) {
  vs23s.dev = stdDev;

#if 0
  if (firstBlock < 39000)
    printf("DevSdSdBlockWrite    (%ld) @%4x %04x %04x %04x %04x %04x %04x %04x %04x\n",
	   firstBlock, data, data[0], data[1], data[2], data[3],
	   data[4], data[5], data[6], data[7]);
#endif

  //  if (firstBlock < 39000) printf("#w1\n");

  while (blocks--) {
    u_int16 newWriteWP = vs23s.writeWP+1;
    s_int16 fill;
    if (newWriteWP >= vs23Size) {
      newWriteWP = 0;
    }
    fill = vs23s.writeWP - vs23s.writeRP;
    if (fill < 0) {
      fill += vs23Size;
    }
    if (++fill > vs23MaxFill) {
      vs23MaxFill = fill;
    }
    vs23CurrFill = fill;
    while ((vs23s.writeBlockW != firstBlock && vs23s.writeRP != vs23s.writeWP)
	   || newWriteWP == vs23s.writeRP) {
#if 0
      printf("#%d%d%d;%lx,%lx,%x,%x,%x;\n",
	     (int)(vs23s.writeBlockW != firstBlock),
	     vs23s.writeRP != vs23s.writeWP,
	     newWriteWP == vs23s.writeRP,
	     vs23s.writeBlockW, firstBlock,
	     vs23s.writeRP, vs23s.writeWP, newWriteWP);
#endif
#if 0
      printf("#");
#endif
      Delay(1);
    }
    if (vs23s.writeRP == vs23s.writeWP) {
#if 0
      printf("2\n");
#endif
      vs23s.writeBlockR = vs23s.writeBlockW = firstBlock;
#if 0
    } else {
      printf("3\n");
#endif
    }
    WriteVS23S010(&byteBus, data, 512*(s_int32)vs23s.writeWP, 512);

    vs23s.writeBlockW++;
    vs23s.writeWP = newWriteWP;
    firstBlock++;
    data += 256;
  }

  //  if (firstBlock < 39000) printf("#w2\n");

  return thereHaveBeenWriteErrors;
}



IOCTL_RESULT DevSdSdIoctl(register __i0 DEVICE *stdDev, s_int16 request, IOCTL_ARGUMENT arg) {
  SD_DEVICE *dev = (SD_DEVICE *)stdDev;
  ioresult errCode = S_OK;
  //  SD_HWINFO *hwi = &(dev->hwInfo);
  u_int16 i, n, cmd;

#if 0
  fprintf(stderr, "DevSdSdIoctl(0x%04p, %6d, 0x%04x) called\n",
	  stdDev, request, arg);
#endif

  switch (request) {
  case IOCTL_RESTART:
#if 0
    printf("SD IOCTL RESTART\n");
#endif
    /* Wait until potential write buffer has cleared. */
    while (vs23CurrFill && !thereHaveBeenWriteErrors) {
      Delay(TICKS_PER_SEC/100);
    }
#ifdef SDSDR
    dev->flags = 0;
#else /* !SDSDR */
    dev->flags = __MASK_WRITABLE;
#endif /* !SDSDR */
    dev->hwInfo.flags = SDSD_F_TRY_4BIT;
#ifdef SDSDMONO
    if (errCode = IoctlRestart(dev)) {
      dev->hwInfo.flags = 0;
      SysError("Retry with 1-bit mode");
      errCode = IoctlRestart(dev);
    }
#else /* !SDSDMONO */
    if (errCode = RunLibraryFunction("SDSDX23", ENTRY_1, (int)stdDev)) {
      dev->hwInfo.flags = 0;
      SysError("Retry with 1-bit mode");
      errCode = RunLibraryFunction("SDSDX23", ENTRY_1, (int)stdDev);
    }
#endif /* !SDSDMONO */
    thereHaveBeenWriteErrors = S_OK;
    break;
  case IOCTL_GET_GEOMETRY:
    if (arg) {
      DiskGeometry *g = (DiskGeometry *)arg;
      g->sectorsPerBlock = 1;
      /* hwInfo.size is in KiB, IOCTL asks for 512-byte blocks,
	 so multiply by two. */
      g->totalSectors = 2*dev->hwInfo.size;
    }
    break;
  default:
    errCode = S_ERROR;
    break;
  }         
  return errCode;
}
