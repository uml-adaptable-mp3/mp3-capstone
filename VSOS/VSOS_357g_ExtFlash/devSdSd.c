/// \file devSdSd.c VsOS Device driver for SD card in native SD mode
/// \author Timo Solla, Henrik Herranen, VLSI Solution Oy
/*
  History:
  2014-11-20 HH - Corrected bug that caused write errors on some SD cards
  2014-11-21 HH - Corrected MMC card size reporting + closing Multiple Block Wr
  2014-11-24 HH - Added optional write CRC checking code, released new driver.
 */

#include <stdlib.h>
#include <string.h>
#include <vo_stdio.h>
#include <timers.h>
#include <clockspeed.h>
#include <audio.h>
#include <power.h>

#ifndef NO_VSIDE_SETUP
#include "vsos.h"
#include "devSdSd.h"
#include "vo_fat.h"   
#include "vs1005g.h"
#include "mmcCommands.h"
//#include "forbid_stdout.h"
#include "fifoRdWr.h"
#include "hwLocks.h"
#endif

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





// devSdSd.hwInfo.flags : params/flags
// devSdSd.hwInfo.rca : RCA
// devSdSd.hwInfo.sizeHigh : size high
// devSdSd.hwInfo.sizeLow : size low
// devSdSd.hwInfo.statusHigh : status high / arg high
// devSdSd.hwInfo.statusLow : status low  / arg low


const SD_DEVICE __y devSdSdDefaults = {   
  0, // u_int16 flags; //< present, block/char
  DevSdSdIdentify, //char*    (*Identify)(void *obj, char *buf, u_int16 bufsize);   
  DevSdSdCreate, //ioresult (*Create)(DEVICE *dev, char *name, struct filesystem_descriptor *fs);
  DevSdSdDelete, //ioresult (*Delete)(DEVICE *dev);
  DevSdSdIoctl, //ioresult (*Ioctl)(DEVICE *dev, s_int16 request, s_int32 arg); //Start, Stop, Flush, Check Media
  // Stream operations
  0, //u_int16  (*Read) (register __i0 DEVICE *self, void *buf, u_int16 destinationIndex, u_int16 bytes); ///< Read bytes to *buf, at byte index
  0, //u_int16  (*Write)(register __i0 DEVICE *self, void *buf, u_int16 sourceIndex, u_int16 bytes); ///< Write bytes from *buf at byte index
  // Block device operations
  DevSdSdBlockRead, //ioresult (*BlockRead)(DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data);
  DevSdSdBlockWrite, //ioresult (*BlockWrite)(DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data);
  // Stream operations
  //DevSdSdInput, //ioresult (*Input)(DEVICE *dev, u_int16 port, void *buf, u_int16 bytes);
  //DevSdSdOutput, //ioresult (*Output)(DEVICE *dev, u_int16 port, void *buf, u_int16 bytes);
  //FILESYSTEM *fs;
  //DIRECTORY *root; //Zero if the device cannot hold directories
  //u_int16  deviceInstance;
  //u_int16  hardwareInfo[6]; ///< Device driver's info of this hardware
  //u_int16  deviceInfo[10]; // For filesystem use, size TBD   
};


// void DelayL (volatile u_int32 i) {


auto void WaitSdReady(void) {
  while (PERIP(SD_CF) & SD_CF_ENA)          // wait for ready
    ;
}



// ====================================================================
// ====================================================================
// ====================================================================
// Read from nf mem to data buffer
#ifndef XP_FIFO_FUNC
#define XP_FIFO_FUNC

//with LCC use register: LCC_REGISTER is register with LCC, nothing with VCC
auto void xp_fiford(LCC_REGISTER u_int16 *dbuf, register u_int16 addr,
		    register u_int16 wcnt) { 
  PERIP(XP_CF) |= XP_CF_RDBUF_ENA;     // read fifo
  PERIP(XP_ADDR) = addr;               // addr ->
  PERIP(XP_IDATA);             // 1st dummy read (for pipeline)
  PERIP(XP_IDATA);             // 2nd dummy read (for pipeline)

  dbuf = XP_FifoRd8(dbuf, wcnt>>3);
  wcnt &= 7;
  while (wcnt--) {
    *dbuf++ = PERIP(XP_IDATA);           // read from fifo
  }
  PERIP(XP_CF)  &= ~XP_CF_RDBUF_ENA;     // release dsp if (reset do op)
  return;
}

// Write data buffer to nf mem
auto void xp_fifowr(LCC_REGISTER const u_int16 *dbuf, register u_int16 addr,
		    register u_int16 wcnt) { 
  PERIP(XP_CF) |= XP_CF_WRBUF_ENA;    // write fifo
  PERIP(XP_ADDR) = addr;              // addr ->
  dbuf = XP_FifoWr8(dbuf, wcnt>>3);
  wcnt &= 7;
  while (wcnt--) {
    PERIP(XP_ODATA) = *dbuf++;           // write to fifo
  }
  PERIP(XP_CF)  &= ~XP_CF_WRBUF_ENA;     // release dsp if (reset do op)
  return;
}

#endif




// vs1005 SD pins:
//    cmd  : gpio2(10)
//    dat3 : gpio2(9)
//    dat2 : gpio2(8)
//    dat1 : gpio2(7)
//    dat0 : gpio2(6)
//    clk  : gpio2(5)
// Using gpmode to switch between drive '1' (gp mode) and
// 'Z'(perip mode) states   (clk is always driven)
#define SD_SETUP_GPIO()  {PERIP(GPIO2_ODATA)|=0x07C0; PERIP(GPIO2_DDR)|=0x07E0; PERIP(GPIO2_MODE)&=~0x07E0;}
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
#define SDgpioSetup()     {PERIP(GPIO2_ODATA)|=0x07C0; PERIP(GPIO2_DDR)|=0x07E0; PERIP(GPIO2_MODE)&=~0x07E0;}
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
#define ACMD6   (0x4000  | ( 6<<8) | RESP_R1  | 3)  // SET_BUS_WIDTH
#define CMD7    (0x8000  | ( 7<<8) | RESP_R1  | 3)  // SELECT(/DESELECT)_CARD
#define CMD7_0  (0x0000  | ( 7<<8) |      0   | 0)  // (SELECT/)DESELECT_CARD
#define CMD8    (0x4000  | ( 8<<8) | RESP_R7  | 3)  // SEND_ICOND, R7
#define CMD9    (0x8000  | ( 9<<8) | RESP_R2  | 3)  // SEND_CSD
#define CMD10   (0x8000  | (10<<8) | RESP_R2  | 3)  // SEND_CID
#define CMD12   (0x0000  | (12<<8) | RESP_R1B | 3)  // STOP_TRANSMISSION
#define CMD13   (0x8000  | (13<<8) | RESP_R1  | 3)  // SEND_STATUS
#define ACMD13  (0x0000  | (13<<8) | RESP_R1  | 3)  // SD_STATUS
#define CMD15   (0x8000  | (15<<8) |      0   | 0)  // GO_INACTIVE_STATE
#define CMD16   (0x4000  | (16<<8) | RESP_R1  | 3)  // SET_BLOCKLEN, in bytes
#define CMD17   (0x4000  | (17<<8) |      0   | 3)  // READ_SINGLE_BLOCK, R1
#define CMD18   (0x4000  | (18<<8) |      0   | 3)  // READ_Multiple_BLOCK, R1
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
    t = ReadTimeCount();
    SD_DAT3_1HIGH();                      // data bus to gpio mode, drive '1'
    // SD_DAT0_Z;                           // dat0 still input
    while (1) {
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
      USEY(SD_LEN) = 1;                  // number of bytes to read
      PERIP(SD_CF) = SD_CF_ENA|SD_CF_READSEL|SD_CF_DATASEL|SD_CF_NOSTOPB;
      WaitSdReady();
      xp_fiford(tmpdata, SDSD_PERIP_ADDR, 1);        // *dbuf, addr, wcnt
      if (needResp) {
	int bits = 8;
	wordStorage |= tmpdata[0]>>8; /* Put new bits to 8 LSb's */
	while (bits-- && needResp) {
	  wordStorage <<= 1;
	  if (!(wordStorage & 0x8000U)) { /* When MSb = 0, there is a reply */
	    /* 0x2xxx = crc ok, 0x5xxx = crc fail, others general fail */
	    needResp = 0;
	    if ((wordStorage & 0xF000U) != 0x2000) {
	      fprintf(stderr, "#%04x\n", wordStorage);
	    }
	  }
	}
      } else { /* !needResp */
	if (PERIP(SD_ST) & SD_ST_DAT0) {              /* DAT0 HIGH */
	  /* Now there are guaranteed 4 extra clocks, but spec requires 8.
	     So offer a few more (32 is overkill, but certainly enough.) */
	  SDClock32();
	  //	fprintf(stderr, "\n");
	  break;
	}
      } /* !needResp */
#else /* !CHECK_WRITE_CRC_ERROR */
      USEY(SD_PTR) = 1536;               // above memory array
      USEY(SD_LEN) = 4;                  // number of bytes to read
      PERIP(SD_CF) = SD_CF_ENA|SD_CF_READSEL|SD_CF_DATASEL|SD_CF_NOSTOPB;
      WaitSdReady();
      if (PERIP(SD_ST) & SD_ST_DAT0) {              // DAT0 HIGH
	SDClock32(); /* Needs at least 8 clock cycles after DAT0 = high,
			send 32. 141120 Henrik */
	break;
      }
#endif /* !CHECK_WRITE_CRC_ERROR */
      if (ReadTimeCount()-t > 1*TICKS_PER_SEC) {    // Enough?, Some cards are very slow
	return 2;                         // give up, data not coming
      }
    }
  }
  SD_DAT_HIGH();                              // data bus to gpio mode, drive '1'
  return 0;                               // OK
}






auto u_int16 SDDataRX1(register SD_DEVICE *dev) {
  SD_HWINFO *hwi = &(dev->hwInfo);
  PERIP(SD_PTR) = SDSD_PERIP_ADDR;              // start address for read data
  PERIP(SD_LEN) = 512;           // number of bytes to transfer
  PERIP(SD_CF) = (hwi->flags&SDSD_F_4BIT) |
    (SD_CF_READSEL) |                  // SD_CF[6]:  RD = 1, WR = 0
    (SD_CF_CRC16|SD_CF_POLL);
  SD_DAT_Z();                                 // data line(s) to perip mode
  PERIP(SD_CF) |= SD_CF_ENA;               // ena HIGH
  return 0;
}







// 
// vs1005G Command write/read function for SD
// 
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


  //========================= READ RESPONSE =========================
  if (sd_cmd & RESP_BUSY) {
    SD_DAT0_Z();                                  // POLL DAT0 FOR BUSY
  }
  // TBD: CMD17, CMD18 should be here ???     // Data reads
  if ((sd_cmd == ACMD51) || (sd_cmd == ACMD13)) {   // Skipping response
    SD_CMD_Z();
    // cmd line to perip mode
    {
      u_int16 res;
      res = SDDataRXTX(dev, (sd_cmd == ACMD51) ? 8 : 64, 1);    // 64 bytes, SD_STATUS
      if (res) {                              // cmd break, no crc
	return res;                           // ERROR, no point to continue
      } else {
	SDClock32();
	//SD_CMD_HIGH();                        // gpio mode, drive '1'
	return 0;                             // OK
      }
    }
  }
	
  PERIP(SD_PTR) = SDSD_PERIP_ADDR;                    // start address for read data
  PERIP(SD_LEN) = 6;                           // number of bytes to read
  if (
#ifdef SUPPORT_UNUSED_CMDS
      (sd_cmd == CMD17) ||
#endif
      (sd_cmd == CMD18)) {
    // read until first byte received
    PERIP(SD_CF) = SD_CF_READSEL|SD_CF_CMDSEL|SD_CF_POLL;  // rd+cmd, pollstart
  } else {
    /**** Onko &3 oikein, vai pitäisikö olla &2? */
    PERIP(SD_CF) = SD_CF_READSEL|SD_CF_CMDSEL|(sd_cmd&0x3);     // rd+cmd, crc7, pollstart
  }
  if (sd_cmd & (RESP_R2)) {                    // R2: 1 + 16 bytes
    PERIP(SD_LEN) = 1;                         // number of bytes to read
    PERIP(SD_CF) = SD_CF_READSEL|SD_CF_CMDSEL|(sd_cmd&0x1); // rd+cmd, pollstart
  }
  SD_CMD_Z();                                     // perip mode



#define lazy_user_or_not_possible_to_weld_a_pull_up_resistor
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
      (sd_cmd == CMD18)) {
    return 0; // go reading data, skip rest of RESP
    // NOTE: LEAVING CMD FLOATING
  }

  if (!((PERIP(SD_ST)&(SD_ST_ANY_ERR|SD_ST_DAT0|SD_ST_CMDBRK)) == SD_ST_DAT0)
      && !(sd_cmd & RESP_BUSY)) {
    return 2;                                 // ERROR, no point to continue
  }

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
  }
  return 0;  // ok
}

	




// 
// Configure GPIOs to SD usage, check if there is a card
// in slot (50k pull up in dat3 line)
// Set up card to SD-mode (CMD0 + DAT3=HIGH, DAT3 = SPI_CS)
// Card is in idle state after this function
//    return 0 : everything OK
//           non-0 : card not there (no pull-up in dat3 found)
//           Not used: pull-up found but CMD0 failed
// 
// This detect is only for SD-cards, not for MMC
//   (MMC reguires clock < 400kHz, MMC requires cmd55+...)
auto u_int16 DetectSDCard(register SD_DEVICE *dev) {
  u_int16 dat3_state = 0;
  // NOTE: Current vs1005x Lab board does not have switch for SD power so
  //       this function is not that usable (in case card is stucked etc.)
  // 
  // Target IO voltage 3.0V, Cvdd 1.85, avdd 3.0
  // Henrik 130430: VS1005 devBoard SD regulator (200 mA) is 3.3 V
  // SetReguVoltage(0x7, 0x10, 0x17, 0x18);  // vsel, avdd, iovdd, cvdd)

#if 0
  //    14-10 :  regu_avdd_ctrl_rs
  //     9-5  :  regu_iovdd_ctrl_rs
  //     4-0  :  regu_cvdd_ctrl_rs
  PERIP(REGU_VOLT) &= ~(0x001F<<REGU_VOLT_IOVDD_B);
  PERIP(REGU_VOLT) |=  (0x17<<REGU_VOLT_IOVDD_B);
  PERIP(REGU_CF) &=  ~0x0008;
  PERIP(REGU_CF) |=   0x0008;  // toggle clock
  PERIP(REGU_CF) &=  ~0x0008;
#endif

#if 1
  /* This will require at least VSOS 3.19b to work (2014-11-27) */
  ResetSdCard();
#endif

  // Init SD device regs
  // default for SD perip mode pins is:
  //    data and cmd : inputs ('Z')
  //    clk          : out '0'
  PERIP(CLK_CF) &= ~CLK_CF_NFOFF;  // Make sure SD perip clock is enabled
  PERIP(SD_CF) = 0x0000;    // SD control     (0xFC7E)
  // TBD: Clock spec is like 100kHz - 400kHz, SD clk is core clock dependent
  PERIP(SD_ST) = 0x1F*SD_ST_WAITSTATES; // Max waitstates
  PERIP(SD_PTR) = SDSD_PERIP_ADDR;  // SD datapointer (0xFC7C)  perip mem addr
  PERIP(SD_LEN) = 0;         // SD data length (0xFC7D)
  PERIP(XP_ST) |= XP_ST_SD_INT;    // SD int         (0xFC6C)  reset int flag if any
  // 
  // 
  // ========================================================== 
  // ===================== SD card Detect ===================== 
  // ========================================================== 
  SD_SETUP_GPIO(); // all SD pins to gpio mode, drive HIGH except clk LOW
  // First dat3 low then float and check if it gets HIGH
  PERIP(GPIO2_MODE) &= ~0x0200;
  PERIP(GPIO2_ODATA) &= ~0x0200; // i.e. dat3 '0'
  SD_DAT3_Z();                   // i.e. dat3 'Z'
  PERIP(GPIO2_ODATA) |= 0x0200;  // restore dat3 drive back to'HIGH'
  DelayL(10000);                 // maybe about 10000*3*80ns = ~2.5ms @ 1x clk
  dat3_state = (PERIP(GPIO2_IDATA)>>9) & 1; // 50k pull-up in dat3 line 
  SD_CMD_DAT_HIGH(); // gpmode

  if (!dat3_state) {
    PERIP(GPIO2_DDR) &= ~0x0020; // turn clock pin back to gp input if no card
    return -1;         // Error
  }

  PERIP(GPIO2_MODE) |=  0x0020; // turn clock pin to SD perip mode

  // ========================================================== 
  // ===================== SD card Reset ====================== 
  // ========================================================== 
  //    clock with 0x1f wait states is : xtal*pll_mult / ((0x1F+1)*2)
  //    E.g. 12.288e6*1 /   ((0x1F+1)*2) = 192000Hz
  //    12.288e6*1      /   ((0x10+1)*2) = 361410Hz

  ///// CMD0 : reset
  /////    select SD mode  => CMD0 + CS <= '1'  (CS = dat3 line)
  /////    select SPI mode => CMD0 + CS <= '0'  (CS = dat3 line)
  /////    CMD0 + cs = '0' => go to spi mode (SD MODE NEEDS PULL UP IN DAT3)
  /////    CMD0 : w40 w00 w00 w00 w00 w95
  // NOTE: sending reset cmd just to be sure card gets to SD mode
  //in SDCmd// SDClock32();       // some clocks, dat and cmd HIGH
#if 0
  // Commented out because similar operations performed in IDSDCard();
  SDClock32();      // some 80+ clocks, dat and cmd HIGH
  SDClock32();
  SDClock32();
  SDCmd(dev, CMD0);    // No response to CMD0
  SDClock32();      // some clocks, dat and cmd HIGH
  //__NO CLOCKS AFTER CMD__ except CMD0 => no response//
#endif
#if 0
  if ( ! (PERIP(SD_ST)&0x0010)) { // This can be removed
    dat3_state = 2; // would mean something is driving against our dat3 line
  }
#endif
  return 0;         // OK: return 0 if card found, MMC has dat3 line?
}







// 
// Identify SD/MMC card
//    From Idle to Standby stat
// Return zero if all OK
// 
auto u_int16 IDSDCard(register SD_DEVICE *dev) {  // init card, interface and sw variables
  SD_HWINFO *hwi = &(dev->hwInfo);
  u_int16 lcnt = 0;

  PERIP(CLK_CF) &= ~(CLK_CF_NFOFF);  // Make sure SD perip clock is enabled
  PERIP(XP_CF) = 0; // disable
  PERIP(XP_ST) |= XP_ST_SD_INT | XP_ST_INT_ENA;
  PERIP(SD_CF) = 0x0000;    // SD control     (0xFC7E)
  PERIP(SD_ST) = 0x1F*SD_ST_WAITSTATES; // SD status    (0xFC7F)  waitstates
#if 0
  // Not needed: done in SDCmd();
  PERIP(SD_PTR) = SDSD_PERIP_ADDR;  // SD datapointer (0xFC7C)  perip mem addr
  PERIP(SD_LEN) = 0;         // SD data length (0xFC7D)
#endif
  // SD int reset int flag if any and enable xperip interrupt

  memset(hwi, 0, sizeof(*hwi));

  ///// 
  ///// CMD0 : reset
  ///// 
  SDClock32();       // some clocks, dat and cmd HIGH
  SDClock32();       // some clocks, dat and cmd HIGH
  SDCmd(dev, CMD0, 0);     // (no response)
  SDClock32();       // some clocks, dat and cmd HIGH
  // NOTE:  SDClock32() leaves SDDatHigh and SDCmdHigh

  DelayL(1000);

  /////
  /////  New spec command  (0 = IDLE state)
  ///// 
  ///// CMD8 : SEND_IF_COND    (resp R7)
  ///// SD State : Idle -> Idle
  /////            Idle -> Idle when NO success (no response)
  // voltage 2.7V - 3.6V, check pattern 0x1AA
  // TBD: do something with the result?
  if (SDCmd(dev, CMD8, SDSD_ARG_CMD8)) {        // Test for SD version 2
    //Just for dbg// printf("   CMD8 ");
    hwi->flags |= SDSD_F_V2;
  }

  // Now try to get the SD card's operating condition
  //   ACMD41: SD_APP_SEND_OP_COND, R3
  // or check if this is MMC card
  //   CMD1:   SEND_OP_COND, R3
  // mmc_send_op_cond(mmc);
  // 
  ///// 
  ///// ACMD41 : SEND_OP_COND    (resp R1 + R3)
  ///// SD State : Idle -> Ready when success
  /////            Idle -> Inactive when NO success
  /////   response: Operation Condition Register OCR[31:0]
  /////   Poll OCR[31] until '1' => power up ok, power setting ok
  /////   When timeout or no response => bad voltages
  /////   card ready when returns ocr_busy = 1 or error (no response)
  ///// 
  ///// CARD STATE: IDLE
  /////    One card's OCR was : 0x 80ff 8000
  /////                   HCSD: 3f c0ff 8000 (ready and SDSD_OCR_HC high)
  ///// 
  /////  Polling clock should be 100kHz - 400kHz and CONTINOUS
  /////  ACMD41 polling interval should be < 50ms
  ///// 


  while (1) {               // poll while ready or timeout
    SDClock32();            // some clocks, dat and cmd HIGH
    if (lcnt & 0x0008) {    // 8-15, 24-31, 40-47
      //TBD  if (SDCmd(MMCCMD1))  { return 200;}
      SDCmd(dev, MMCCMD1, SDSD_ARG_OCR_3V3|SDSD_OCR_HC);     // SEND_OP_COND
    } else {                // 0-7, 16-23, 32-39
      SDCmd(dev, CMD55, 0);       // APP cmd next, response R1
      SDCmd(dev, ACMD41, SDSD_ARG_OCR_3V3|SDSD_OCR_HC);      // SD_APP_OP_COND, no crc7, response R3 : OCR
    }
    DelayL(10000);
    // Timeout
    if (hwi->flags & SDSD_F_OCR31) {  // status high / arg high
      if (lcnt & 0x0008) {
	hwi->flags |= SDSD_F_MMC;
	hwi->rca = SDSD_DEF_MMCRCA; // NOTE: MMC CMD3 IS SET_RCA_ADDR
      } else {
	hwi->flags |= SDSD_F_SD;
      }
      break;
    }
    if (++lcnt > 2*128) {     // One card seemed to take two loops
      // No SD card nor MMC card
      return 200;           // ERROR, no point to continue (0=IDLE / INACTIVE)
    }
  }




  ///// 
  ///// CMD2 : ALL_SEND_CID    (resp R2 i.e. 136 bits, INTERNAL CRC7)
  ///// SD State : Ready -> Ident when success
  /////            Ready -> Ready (or ?) when NO success
  /////   response: Card Identification Register CID[127:0]
  /////              (manufacturer stuff)
  ///// 
  ///// CARD STATE: READY
  /////    One card responded CID:  353 4453 4432 3536 8000 7362 1100 62cb
  ///// 
  // ALL_SEND_CID, response R2, 136 bits
  if (SDCmd(dev, CMD2, 0)) {
    return 201;  // ERROR, no point to continue (1=READY)
  }




  ///// 
  ///// CMD3 :  SEND_RELATIVE_ADDRESS, response = R6 (48 bits)
  ///// SD State : Ident -> Standby when success
  /////            Ident -> Ident (or ?) when NO success
  /////   response: Card Relative Address RCA[15:0]
  /////   NOTE: RCA CHANGES EACH TIME CMD3 IS CALLED (RCA++ seemed to be
  /////         the case)
  ///// 
  ///// CARD STATE: IDENT
  /////    One card responded RCA: e624
  ///// 
  // SEND_RELATIVE_ADDR, response R6, 48 bits
  if (hwi->flags & SDSD_F_MMC) {
    if (SDCmd(dev, MMCCMD3, 0)) {
      return 202; //ERROR, no point to continue (2=IDENT)
    }
  } else { /* SDSD_F_SD */
    if (SDCmd(dev, CMD3, 0)) {
      return 203;  // ERROR, no point to continue (2 = IDENT)
    }
    // Do not accept RCA = 0x0000, call again CMD3 to increment RCA
    if (!hwi->rca) {
      if (SDCmd(dev, CMD3, 0)) {
	return 204;  // ERROR, no point to continue (2 = IDENT)
      }
    }
  }





  // 
  // End of Initialization and Identification
  // 

  ///// 
  ///// CMD9: SEND_CSD, response = R2 (i.e. 136 bits, 17 bytes)
  ///// SD State : Standby -> Standby when success
  /////            Standby -> Standby when NO success
  /////   response: Card Spesific Data CSD[127:0]
  ///// 
  ///// CARD STATE: STANDBY
  /////    One card responded CSD:  26 32 5f59 83c4 fefa cfff 9240 40df
  ///// 
  ///// 
  ///// CMD9: SEND_CSD  (Card-specific data) (response is R2 i.e. 136 bits)
  /////                 (127bits: 120 of data + 7 crc, , NOTE: INTERNAL CRC)) 
  // SEND_CSD, response R2, 136 bits
  if (SDCmd(dev, CMD9, 0)) {
    return 205;  // ERROR, no point to continue (3=STANDBY)
  }
  // Data read acces time TAAC: CSD[119:112]
  // Max data transfer rate TRAN_SPEED: CSD[103:96] (should be 0x32 = 25MHz)
  //    One 256M card CSD:  0026 0032 5f59 83c4 fefa cfff 9240 40df
  //                             ^^^^
  // Adjusting wait state accordingly
  // with 100MHz clock this would be 25M (clk / (2*(1+1)))
  PERIP(SD_ST) &= ~(0x1F*SD_ST_WAITSTATES) | (1*SD_ST_WAITSTATES);

  ///// 
  ///// CMD10: SEND_CID, response = R2 (i.e. 136 bits, 17 bytes)
  ///// SD State : Standby -> Standby when success
  /////            Standby -> Standby when NO success
  /////   response: Card IDentification data CID[127:0]
  ///// 
  ///// CARD STATE: STANDBY
  /////    One card responded CID:   353 4453 4432 3536 8000 7362 1100 62cb
  ///// 
  ///// 
  ///// CMD10 : SEND_CID (card identification) (response: R2, 136 bits)
  /////
  // SEND_CID, response R2, 136 bits
  if (SDCmd(dev, CMD10, 0)) {
    return 206;  // ERROR, no point to continue (3=STANDBY)
  }


  ///// 
  ///// CMD7: SELECT/DESELECT_CARD, response = R1 (i.e. 48 bits, 6 bytes)
  ///// SD State : Standby  -> Transfer when success
  /////       or   Transfer -> Standby  when success
  /////            Standby -> Standby   when NO success
  /////       or   Transfer -> Transfer when success
  /////   response: Card Status[31:0]
  ///// 
  ///// CARD STATE: STANDBY
  ///// 
  ///// 
  ///// CMD7 : SELECT/DESELECT_CARD STANDBY <-> TRANSFER (response: R1)
  /////
  // SELECT(/DESELECT)_CARD, response R2, 136 bits
  if (SDCmd(dev, CMD7, 0)) {
    return 207; // ERROR, no point to continue (3 = STANDBY)
  }

  return 0;    // So far so good (4 = TRANSFER)
}






// Setup SD/MMC card's data bus (block size, 1/4bit bus for SD)
//    From Idle -> Standby stat
// Return zero if all OK
// 
auto u_int16 SetupSDData(register SD_DEVICE *dev) {  // init card DATA interface
  SD_HWINFO *hwi = &(dev->hwInfo);
  ///// 
  ///// CARD STATE: TRANSFER
  ///// 
  // This only applies to SD card (Not to MMC card)
  if (hwi->flags & SDSD_F_SD) {         // NOT SD___MMC___bit
    u_int16 tmpdata[4];
    // SEND_SCR[63:0]
    // Read card's SCR register  // One card's SCR:    0  920    0    0
    if (SDCmd(dev, CMD55, 0))  {
      return 301; // APP_CMD cmd next, R1, 48b
    }
    if (SDCmd(dev, ACMD51, 0)) {
      return 302; // SEND_SCR, R1 + 64 bit data
    }

    // NOTE: DATA TRANSFER IS IN CMD FUNCTION:  SDDataRd(8);  // bytes
    // TBD: This could be in SDCmdV2 like other data gathering stuff
    // read all 17 bytes (first three words again)
    xp_fiford(tmpdata, SDSD_PERIP_ADDR, 4);          // *dbuf, addr, wcnt
    // SD.scr[0] = tmpdata[0];
    // SD.scr[1] = tmpdata[1];
    // SD.scr[2] = tmpdata[2];
    // SD.scr[3] = tmpdata[3];
    if (tmpdata[0] & (1<<2)) {                // SCR[51:48]: SD_BUS_WIDTHS
      hwi->flags |= SDSD_F_4BIT;         // enable 4-bit bus
#if 0
      fprintf(stderr, "## 4-bit mode!\n");
    } else {
      fprintf(stderr, "## 1-bit mode!\n");
#endif
    }
    // STOP_TRANSMISSION, R1b
    if (SDCmd(dev, CMD12, 0)) {
      return 303; // ERR, no point to continue
    }
    // SCR[51:48] bit 2 (i.e. [50]) = '1' => 4-bit data bus supported
  }

  // CMD16: SET_BLOCKLEN 512 bytes
  // Set block length to 512 bytes
  // CMD16: SET_BLOCKLEN 
  if (SDCmd(dev, CMD16, 512)) {           // arg,  Block size in bytes
    return 304;  // ERROR, no point to continue
  }
  

  // Set data bus width to 4 bits if possible
  if (hwi->flags & SDSD_F_4BIT) {          // SD params
    // CMD55: APP_CMD, R1
    // ACMD6: SET_BUS_WIDTH 1/4-bit
    if (SDCmd(dev, CMD55, 0) || SDCmd(dev, ACMD6, 0b10)) { // arg: 0b00: 1-bit, 0b10: 4-bit
      return 306; // ERROR, no point to continue
    }
  }


#if 0
  // This can be removed. Data is not used at the moment.
  if (hwi->flags & SDSD_F_SD) {           // SD params, NOT MMC
    if (SDCmd(dev, CMD55, 0))  {
      return 307;  // APP_CMD cmd next, R1, 48b
    }
    if (SDCmd(dev, ACMD13, 0)) {
      return 308;  // SD_STATUS, R1 + 512b data
    }
    //  NOTE: THIS IS IN RDCMD:  SDDataRd(512/8); // bytes
  }
#endif

  return 0;                                       // all OK
}




// ====================================================================
// ====================================================================
// ====================================================================



   
ioresult DevSdSdCreate(register __i0 DEVICE *dev, void *name,
		       u_int16 extraInfo) {
  ObtainHwLocksBIP(HLB_0<<SDSD_PERIP_BUFNO, HLIO_SD, HLP_SD);
  // printf("DevSdSdCreate kutsuttu\n");
  memcpyYX(dev, &devSdSdDefaults, sizeof(DEVICE));
  dev->deviceInstance = __nextDeviceInstance++;
  /* Ignore result of IOCTL: even if there is no SD card inserted at the
     moment, device creation is ok, and if a card is inserted later, it
     can be accessed through this handle. */
  dev->Ioctl(dev, IOCTL_RESTART, 0);
  return S_OK;
}

ioresult DevSdSdDelete(register __i0 DEVICE *dev) {
  if (dev) {
    ObtainHwLocksBIP(HLB_NONE, HLIO_NONE, HLP_XPERIP_IF);
    PERIP(XP_ST) &= ~XP_ST_SD_INT;
    ReleaseHwLocksBIP(HLB_0<<SDSD_PERIP_BUFNO, HLIO_SD, HLP_SD|HLP_XPERIP_IF);
  }
  return S_OK;
}


const char *DevSdSdIdentify(register __i0 void *dev,
			    char *buf, u_int16 bufsize) {
  return "SD/SD Card";
}


auto s_int16 SdIdentify(register __i0 SD_DEVICE *dev) {
  u_int16 errCode;
  char *errStr = NULL;
  SD_HWINFO *hwi = &(dev->hwInfo);

  ObtainHwLocksBIP(HLB_NONE, HLIO_NONE, HLP_XPERIP_IF);

  if ((errCode = DetectSDCard(dev))) {
    errStr = "SD Card not found";
    goto finally;
  }

  if ((errCode = IDSDCard(dev))) {
    errStr = "SD Ident / Card stuck"; 
    goto finally;
  }

  // 512b blocks, 4-bitbus, waits...
  if ((errCode = SetupSDData(dev))) {
    errStr = "SD data bus set up";
    goto finally;
  }

 finally:
  ReleaseHwLocksBIP(HLB_NONE, HLIO_NONE, HLP_XPERIP_IF);
  if (errStr) {
    SysError(errStr);
  }

#if 0
  if (!errCode) {
    fprintf(stderr,
	    " SD identified.\n"
	    "    RCA  : %x\n"
	    "    Size : %ld kB\n"
	    "    Flags: %x\n",
	    hwi->rca, hwi->size, hwi->flags);
  }
#endif

  return errCode;
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

  if (!__F_CAN_SEEK_AND_READ(dev)) {
    hwi->nextBlock = -1;
    return SysError("Seek");
  }

  ObtainHwLocksBIP(HLB_NONE, HLIO_NONE, HLP_XPERIP_IF);

  if (((hwi->flags & SDSD_F_MULTIPLE_BLOCK_READ) &&
       (firstBlock != hwi->nextBlock || !blocks)) ||
      hwi->flags & SDSD_F_MULTIPLE_BLOCK_WRITE) {
    /* Close down earlier multiple block read. */
#if 0
    printf("  Close old R/W, bl %ld, nB %p\n", hwi->nextBlock, &hwi->nextBlock);
#endif
    WaitSdReady();

    hwi->nextBlock = -1;
    SD_DAT_Z();                                // data line(s) to perip mode
    if (SDCmd(dev, CMD12, 0)) {
      errStr = "Data stop fail";  // ERROR, no point to continue
      errCode = S_ERROR;
      goto finally;
    }
    SDClock32();                           // NOTE: some clocking
    //SD_DAT_HIGH();                       // data line(s) to perip mode

    hwi->flags &= ~(SDSD_F_MULTIPLE_BLOCK_WRITE|SDSD_F_MULTIPLE_BLOCK_READ);
  }

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
  while (blocks) {
#if 0
    printf("  Read %ld\n", firstBlock);
#endif

    u_int32 t = ReadTimeCount();

    WaitSdReady();

    while ((PERIP(SD_ST)&SD_ST_ANY_ERR) && ReadTimeCount()-t < TICKS_PER_SEC/2) {
      PERIP(SD_CF) |= SD_CF_ENA;               // ena HIGH
      WaitSdReady();
    }

    if (PERIP(SD_ST) & SD_ST_ANY_ERR) {
      SDCmd(dev, CMD12, 0);                 // try to get back on line
      //return ( (blocks<<4) | 2); }     // ERROR, no point to continue
      hwi->nextBlock = -1;
      errStr = "Data Fail";    // ERROR, no point to continue
      errCode = S_ERROR;
      goto finally;
    }

    SD_CMD_DAT_HIGH();                              // data bus to gpio mode, drive '1'
    // it should be gone now
    xp_fiford(data, SDSD_PERIP_ADDR, 256);       // *dbuf, addr, wcnt
    SDDataRX1(dev);	// Start next transfer
    data += 256;
    hwi->nextBlock++;
    blocks--;
  }
 finally:
  ReleaseHwLocksBIP(HLB_NONE, HLIO_NONE, HLP_XPERIP_IF);
  if (errStr) {
    SysError(errStr);
  }
  return errCode;                             // OK
}




ioresult DevSdSdBlockWrite(register __i0 DEVICE *stdDev,  u_int32 firstBlock,
			   u_int16 blocks, u_int16 *data) {
  SD_DEVICE *dev = (SD_DEVICE *)stdDev;
  SD_HWINFO *hwi = &(dev->hwInfo);
  ioresult errCode = S_OK;
  char *errStr = NULL;
  // u_int16 errorCount=0;
#if 0
  printf("DevSdSdBlockWrite (%ld) @%4x %04x %04x %04x %04x\n",
	 firstBlock, data, data[0], data[1], data[2], data[3]);
#endif
  //  if (!__F_CAN_SEEK_AND_WRITE(dev)) {
  //  return S_ERROR;
  //}
#if 0
  printf("SdSdBlWrite(%ld, %d)\n",firstBlock,blocks);
#endif

  if (!__F_CAN_SEEK_AND_READ(dev)) {
    hwi->nextBlock = -1;
    return SysError("Seek");
  }

  ObtainHwLocksBIP(HLB_NONE, HLIO_NONE, HLP_XPERIP_IF);
  if (((hwi->flags & SDSD_F_MULTIPLE_BLOCK_WRITE) &&
       (firstBlock != hwi->nextBlock || !blocks)) ||
      hwi->flags & SDSD_F_MULTIPLE_BLOCK_READ) {
    /* Close down earlier multiple block write. */
#if 0
    printf("  Close old W/R, bl %ld\n", hwi->nextBlock);
#endif
    WaitSdReady();
    hwi->nextBlock = -1;
    SD_DAT_Z();                                // data line(s) to perip mode
    if (SDCmd(dev, CMD12, 0)) {
      errStr = "Data stop fail";  // ERROR, no point to continue
      errCode = S_ERROR;
      goto finally;
    }
    SDClock32();                           // NOTE: some clocking
    //SD_DAT_HIGH();                       // data line(s) to perip mode

    hwi->flags &= ~(SDSD_F_MULTIPLE_BLOCK_WRITE|SDSD_F_MULTIPLE_BLOCK_READ);
  }

  if (blocks && !(hwi->flags & SDSD_F_MULTIPLE_BLOCK_WRITE)) {
    // WRITE_MULTIPLE_BLOCK
    if (SDCmd(dev, CMD25,
	      (hwi->flags & SDSD_F_HC) ? firstBlock : firstBlock<<9)) {
      // ERROR, no point to continue
      errCode = S_ERROR;
      errStr = "Multiple block write";
      goto finally;
    }
    // NOTE: IT SEEMED THAT CMD24/CMD25 COMMAND REQUIRED SOME CLOCKS AFTERWARDS.
    //       THEY ARE NOW FED IN SDCmd() -FUNCTION
    hwi->flags |= SDSD_F_MULTIPLE_BLOCK_WRITE;
    hwi->nextBlock = firstBlock;
  }

  while (blocks) {
    xp_fifowr(data, SDSD_PERIP_ADDR, 256);      // *dbuf, mem addr, wcnt
    if (SDDataRXTX(dev, 512, 0)) {       // Write = TX
      SDCmd(dev, CMD12, 0);                    // try to get back on line
      errCode = (blocks<<4) | 2;            // ERROR, no point to continue
      goto finally;
    }
    data += 256;
    hwi->nextBlock++;
    blocks--;      
  }

 finally:
  ReleaseHwLocksBIP(HLB_NONE, HLIO_NONE, HLP_XPERIP_IF);
  if (errStr) {
    SysError(errStr);
  }
  return errCode;
}




IOCTL_RESULT DevSdSdIoctl(register __i0 DEVICE *stdDev, s_int16 request, IOCTL_ARGUMENT arg) {
  SD_DEVICE *dev = (SD_DEVICE *)stdDev;
  ioresult errCode = S_OK;
  //  SD_HWINFO *hwi = &(dev->hwInfo);
  u_int16 i, n, cmd;

#if 0
  printf("DevSdSdIoCtl(0x%04p, %6d, 0x04x) called\n", stdDev, request, arg);
#endif

  switch (request) {
  case IOCTL_RESTART:
    //    fprintf(stderr, "SD IOCTL RESTART\n");
    if (!SdIdentify(dev)) {
      // Could indentify. Now mark as good, then find filesystem
      dev->flags = __MASK_PRESENT | __MASK_OPEN |
	__MASK_SEEKABLE | __MASK_READABLE | __MASK_WRITABLE |
	__MASK_OVERLAPPED;
      // Find a filesystem which understands this device
      dev->fs = StartFileSystem(stdDev, "0");
      if (!dev->fs) {
	// Couldn't find the filesystem; remove __MASK_OPEN flag
	dev->flags = __MASK_PRESENT |
	  __MASK_SEEKABLE | __MASK_READABLE | __MASK_WRITABLE |
	  __MASK_OVERLAPPED;
        errCode = S_ERROR;
      }
    } else {
      // Could not identify, remove __MASK_OPEN flag
      dev->flags = __MASK_PRESENT |
	__MASK_SEEKABLE | __MASK_READABLE | __MASK_WRITABLE |
	__MASK_OVERLAPPED;
      errCode = S_ERROR;
    }
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
