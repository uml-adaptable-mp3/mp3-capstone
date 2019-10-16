#include <stdio.h>
#include <string.h>
#include <physical.h>
#include "vs1005g.h"
#include <clockspeed.h>
#include "vsnand.h"

#include <audio.h>
#include <mappertiny.h>
//#include <minifat.h>

u_int16 myMinifatBuffer[512];
u_int16 myMallocAreaX[4096];
void puthex(u_int16 a);
void puthex6(u_int32 a);
void fputhex6(u_int32 a, FILE *fp);

#define DEBUG_LEVEL 1
#include "debuglib.h"


extern struct FsNandPhys fsNandPhys;

//#define DUMP_PHYS
//#define CHECK_PHYS
//#define CHECK_MAP
//#define DUMP_FAT
//#define FAT_FORMAT
#define WR_IMAGE

extern struct FsPhysical *ph;
extern struct FsMapper *map;
extern struct Codec *cod;
//extern struct CodecServices cs;

extern s_int16 tmpBuf[2*32];

__y const char hex[] = "0123456789abcdef";
void puthex(u_int16 a) {
  char tmp[8];
  tmp[0] = hex[(a>>12)&15];
  tmp[1] = hex[(a>>8)&15];
  tmp[2] = hex[(a>>4)&15];
  tmp[3] = hex[(a>>0)&15];
  tmp[4] = ' ';
  tmp[5] = '\0';
  fputs(tmp, stdout);
}
void fputhex6(u_int32 a, FILE *fp) {
  char tmp[8];
  tmp[0] = hex[(a>>20)&15];
  tmp[1] = hex[(a>>16)&15];
  tmp[2] = hex[(a>>12)&15];
  tmp[3] = hex[(a>>8)&15];
  tmp[4] = hex[(a>>4)&15];
  tmp[5] = hex[(a>>0)&15];
  tmp[6] = '\0';
  fputs(tmp, fp);
}
void puthex6(u_int32 a) {
    fputhex6(a, stdout);
}
#define NAN 0x80000000L
long GetParam(char *str) {
  static char in[200];
  char *tmp;
  register int i;
  long value;
  if (str) {
    fputs(str, stdout);
    fflush(stdout);
  }
  fgets(in, 200, stdin);
  i = strlen(in);
  while (i && (in[i-1] == '\n' || in[i-1] == '\r'))
    in[--i] = '\0';
  value = strtol(in, &tmp, 0);
  if (!in[0] || (tmp && *tmp && *tmp != '\n' && *tmp != '\r'))
    return NAN;
  return value;
}
void PrintSector(u_int16 *data, u_int16 *spare) {
  register int i;
  if (data) {
    puts("d");
    for (i=0;i<256;i++) {
      puthex(data[i]);
      if ((i&7) == 7)
	putchar('\n');
    }
  }
  if (spare) {
    puts("s");
    for (i=0;i<8;i++)
      puthex(spare[i]);
    putchar('\n');
  }
}

u_int16 NandGetIdent(void){
  u_int16 ident[4];
  NandDeselect();
  NandSelect();
  NandPutCommand(NAND_OP_READ_SIGNATURE);
  NandPutAddressOctet(0);
  NandGetOctets(8,ident);
  NandDeselect();
  fputs("Nand Signature: ",stdout);
  /*
    2cdc 9095 5400 MT29F4G08AAA
    ecd3 14a5 64ec K9G8G08U0A-PCB0   8Gb MLC  type=3 bl=9 sz=21
    ecd5 55a5 68ec K9LAG08U0A-PCB0  16Gb MLC  type=3 bl=9 sz=22 (2 dies)
    add3 14a5 34ad HY27UT088G2A-TPC8 8Gb MLC  type=3 bl=9 sz=21
    add3 14a5 64ad HY27UU08AG5M          MLC  type=3 bl=9 sz=22(21) (2 /CS, 2 R/B)
  */
  /*
    1st byte: maker code
    2nd byte: device code
    3rd & 4th byte: parameters
   */
#if 1
  {
    register int i;
    for (i=0; i<3; i++){
      puthex(ident[i]);
    }
  }
  fputs("\n",stdout);
#endif

#if 1
  puthex(1 << ((ident[1]>>8) & 3));
  puts("dies");
  puthex(((ident[1] >> 10) & 3) + 1);
  puts("bits/cell");
  puthex(1 << ((ident[1] >> 12) & 3));
  puts("sim.prog.pages");
//  puts((ident[1] & 0x4000U) ? "Interleaved program" : "No interleaved program");
//  puts((ident[1] & 0x8000U) ? "Cache program" : "No cache program");
  puthex((ident[1] & 3)+1);
  puts("kB page");
  puthex(64 << ((ident[1] >> 4) & 3));
  puts("kB block");
#endif
  return ident[0];
}


void kernel_main(void) {
  InitClockSpeed(12288, 11520);
  SetClockSpeedLimit(80000000);
#if 0
  SetClockSpeed(SET_CLOCK_USB);
#else
  SetClockSpeed(12288000);
#endif

#if 1
  ph = FsPhNandCreate(1);

  // OPTION 100
  ((struct FsNandPhys *)ph)->nandType = 3;


  // OPTION 120
    {
      /* small page, 32 sectors per block, 128MB flash */
      int blockSizeExp = 5, flashSizeExp = 18, i;
      long wsns;
      __y long tmp;
#if 0
      wsns = GetParam("Waits (ns): ");
#else
      wsns = 70;
#endif
      if (wsns == NAN)
	goto end;

#if 0
      tmp = GetParam("NandType: ");
#else
      tmp = 3;
#endif
      if (tmp == NAN)
	goto end;
      ((struct FsNandPhys *)ph)->nandType = tmp;
      ((struct FsNandPhys *)ph)->waitns = wsns;

#if 0
      tmp = GetParam("blockSizeExpr: ");
#else
      tmp = 8;
#endif
      if (tmp == NAN)
	goto end;
      blockSizeExp = tmp;
      ph->eraseBlockSize = (1 << blockSizeExp);
#if 0
      tmp = GetParam("flashSizeExpr: ");
#else
      tmp = 20;
#endif
      if (tmp == NAN)
	goto end;
      flashSizeExp = tmp;
      ph->eraseBlocks = 1 << (flashSizeExp-blockSizeExp);

      puthex(1<<blockSizeExp);
      puts("=eraseBlockSize");
      puthex(ph->eraseBlocks);
      puts("=eraseBlocks");


      if (i = ph->Erase(ph, 0)) {
	puthex(i);
	puts("Erase failed");
      }
      memset(myMinifatBuffer, -1/*0xffffU*/, 256);
      myMinifatBuffer[0] = ('V' << 8) + 'L';
      myMinifatBuffer[1] = ('S' << 8) + 'I';
      myMinifatBuffer[2] = ((struct FsNandPhys *)ph)->nandType;
      myMinifatBuffer[3] = (blockSizeExp << 8) + flashSizeExp;
      myMinifatBuffer[4] = ((struct FsNandPhys *)ph)->waitns;
      memset(tmpBuf, -1, sizeof(tmpBuf));

      if (ph->Write(ph, 0, 1, myMinifatBuffer, tmpBuf) != 1) {
	puts("Write failed");
      }
      puts("Inited");
    }


  // OPTION 111
    {
      u_int32 sector, sectors = (u_int32)ph->eraseBlockSize * ph->eraseBlocks;
      int i;
      FILE *fp;

      puthex6(ph->eraseBlockSize);
      puts("=eraseBlockSize");
      puthex6(ph->eraseBlocks);
      puts("=eraseBlocks");
      puthex6(sectors);
      puts("=sectors");

      for (sector = 0x0; sector < sectors; sector += ph->eraseBlockSize) {
	if (ph->Erase(ph, sector)) {
	  puthex6(sector);
	  puts("Error");
#if 1 /*mark bad*/
	  memset(tmpBuf, 0xffffU, 8);
	  tmpBuf[0+2] = 0;
	  ph->Write(ph, sector, 1, NULL, tmpBuf);
#endif
	}
      }
      puts("Done");
      fflush(stdout);
    }




    // OPTION 14: Program flash

    {
	FILE *fp;
	int i;

#if 0
	fputs("Filename: ", stdout);
	fflush(stdout);
	fgets(in, 200, stdin);
	i = strlen(in);
	while (i && (in[i-1] == '\n' || in[i-1] == '\r'))
	    in[--i] = '\0';

	fp = fopen(in, "rb");
#else
	fp = fopen("startextflash.nan", "rb");
#endif
	if (fp) {
	    u_int32 sector = 0;
	    puts("Programming");
	    PERIP(INT_ENABLEL0) &= ~INTF_UART_RX;
#define BLOCK_SIZE 2048
//#define BLOCK_SIZE 512
	    memset(myMallocAreaX, -1, BLOCK_SIZE/2);
	    while (fread(myMallocAreaX, 1, BLOCK_SIZE/2, fp)) {
	      if ((sector & (ph->eraseBlockSize-1)) == 0) {
		puthex6(sector);
		puts("Erasing");
		ph->Erase(ph, sector);
	      }
	      memset(tmpBuf, -1, 4*16/2);
	      /*
		"70ns" (82ns) separately 4*512 w+r, 5.5*400us = 2.2ms
		"70ns" (82ns) 2048 w+r, 3.2*400us = 1.28ms
		"25ns" (34ns) 2048 w+r = 0.83ms
	       */
	      if (
#if 1 && BLOCK_SIZE == 2048
		  LargePageWrite(sector, myMallocAreaX, tmpBuf) != 0
#else
		  ph->Write(ph, sector, BLOCK_SIZE/512, myMallocAreaX, tmpBuf) != BLOCK_SIZE/512
		  //MyNandPutPage(sector, myMallocAreaX, tmpBuf) != 0
#endif
		  ) {
		puts("WrError");
	      }
#if 1 /*Compare*/
	      {
		static u_int16 cmp[BLOCK_SIZE/2], spare[64/2];
		if (
#if 1 && BLOCK_SIZE == 2048
		    LargePageRead(sector, cmp, spare) != 0
#else
		    ph->Read(ph, sector, BLOCK_SIZE/512, cmp, spare) != BLOCK_SIZE/512
#endif
		    ) {
		  puthex6(sector);
		  puts("Read Error");
		}
		{
		  if (memcmp(cmp, myMallocAreaX, BLOCK_SIZE/2)) {
		    int i;
		    for (i=0;i<BLOCK_SIZE/2;i++) {
		      if (myMallocAreaX[i] != cmp[i]) {
			printf("%3d %04x %04x\n", i, myMallocAreaX[i], cmp[i]);
			goto end;
		      }
		    }
		    puthex6(sector);
		    puts("Compare Error");
		  } else if (memcmp(spare, tmpBuf, BLOCK_SIZE/32/2)) {
		    int i;
		    for (i=0;i<BLOCK_SIZE/32/2;i++) {
		      if (tmpBuf[i] != spare[i]) {
			printf("%3d %04x %04x\n", i, tmpBuf[i], spare[i]);
			goto end;
		      }
		    }
		    puthex6(sector);
		    puts("Spare Error");
		  }
		}
	      }
	      if (sector == 0x600)
		goto end;
#endif
	      sector += BLOCK_SIZE/512;
	      memset(myMallocAreaX, -1, BLOCK_SIZE/2);
	    }
	    fclose(fp);
	    puthex(sector);
	    puts("=sectors written");
	    PERIP(INT_ENABLEL0) |= INTF_UART_RX;
	    printf("Successfully finished!\n");
	    goto end2;
	}
    }

 end:
    printf("There were problems; quitting\n");
 end2:
    while (1)
      ;


#endif

#if 0
  ph = FsPhNandCreate(1);

  while (1) {
    static char in[200];
    char *tmp;
    int mode;

      puthex(((struct FsNandPhys *)ph)->nandType);
      puts("=nT");
	puthex(ph->eraseBlockSize);
	putchar(' ');
	puthex(ph->eraseBlocks);
	puts("=block size,erase blocks");
	//	NandGetIdent();


    puts("\n  1) Badblocks");
#ifdef DUMP_PHYS
    puts("  2) Dump phys");
#endif
#ifdef DUMP_FAT
    puts("  3) Dump fat");
#endif
#ifdef FAT_FORMAT
    puts("  4) FAT format");
#endif
#ifdef CHECK_PHYS
    puts(" 22) Check phys");
#endif
#ifdef CHECK_MAP
    puts(" 23) Check map");
#endif
    //puts("100) Determine flash type");
    //puts("101) Determine nand size");
    puts("111) Erase all");
#define ANYFLASH
#ifdef ANYFLASH
    puts("120) Init flash");
#else
    puts("128) 256MB L, 64(256) s/b");
#endif
//    puts("129) 128MB S, 32 s");
//    puts("130) 64MB S, 32 s");
    puts(" 50) Map fmt");
    puts(" 51) Map fmt w/ res.block");
    puts(" 10) Pr sectors");
    puts(" 11) Er block");
    puts(" 12) Wr block");
#ifdef WR_IMAGE
    puts(" 14) program img");
#endif
    fputs("\ncommand: ", stdout);
    fflush(stdout);

    fgets(in, 200, stdin);
    mode = strtol(in, &tmp, 0);
    if (tmp && *tmp && *tmp != '\n' && *tmp != '\r') {
      puts("Inv");
      continue;
    }
    switch (mode) {
    case 100:
	((struct FsNandPhys *)ph)->nandType =
	    (((struct FsNandPhys *)ph)->nandType + 1) & 7;
	//DetermineNandType();
      break;

    case 101:
	//DetermineNandSize();
      break;

#ifdef FAT_FORMAT
    case 4:
#define RES_SECT 4
    {
	static const __y u_int16 MBR[] = {
	  /*          M     S D     O S     5 .     0 Bytesps Spc   res */
	  0xe900, 0x904d, 0x5344, 0x4f53, 0x352e, 0x3000, 0x0202, RES_SECT<<8,
	  /*fats rootent      media fat size s/t  #tracks               */
	  0x0200, 0x0200, 0x00f8, 0xe600, 0x3f00, 0xff00, 0x0000, 0x0000,
	  /*totsec32      drive   esig sernum         N     O       N A */
	  0x00cc, 0x0100, 0x0000, 0x2923, 0xf8b2, 0xb84e, 0x4f20, 0x4e41,
	  /*M E                     F A     T 1     6                   */
	  0x4d45, 0x2020, 0x2020, 0x4641, 0x5431, 0x3620, 0x2020
	};
/*
  Notes:
  BPB_BytesPerSector * BPB_SectorsPerCluster <= 32768
  BPB_RsvdSecCnt must be non-zero, normally 1 for FAT16, 32 for FAT32.
  BPB_NumFATs should be 2 for compatibility
  BPB_RootEntCnt must be 0 for FAT32, for max compatibility 512 for FAT16.
  BPB_TotSec16 can be 0, then BPB_TotSec32 is used. Must be 0 for FAT32.
  BPB_Media must equal FAT[0] low byte.
  BPB_FatSz16 count of sectors for one FAT. Must be 0 for FAT32.
  BPB_SecPerTrk only relevant for media having a geometry.
  BPB_NumHeads only relevant for media having a geometry.
  BPB_HiddSec 0 for media that is not partitioned.
  BPB_TotSec32 total sectors on the volume.

  BPB_FatSz32  FAT32 only: sectors occupied by one FAT.
  BPB_ExtFlags FAT32 only: d0-d3 active FAT, if mirroring is disabled.
                           d7: 0=FAT is mirrored, 1=only one FAT active.
  BPB_RootClus FAT32 only: number of root directory cluster, normally 2.
 */
	int sectorsPerCluster = 1, fatSize;
	u_int32 clusters;
#if 1
	if ((ph->eraseBlocks & 1) == 0)
	    ph->eraseBlocks--; /* Reserve one erase block for settings */
	//ph->eraseBlocks = 0x1ff;
#endif
	map = FsMapFlCreate(ph, 0);
#if 0
	if (map->blocks <= 8400) { /*Upto 4.1MB*/
	    sectorsPerCluster = 0;/*Error*/
	} else if (map->blocks <= 32680) { /*Upto 16MB*/
	    sectorsPerCluster = 2;
	} else if (map->blocks <= 262144) { /*Upto 128MB*/
	    sectorsPerCluster = 4;
	} else if (map->blocks <= 524288) { /*Upto 256MB*/
	    sectorsPerCluster = 8;
	} else if (map->blocks <= 1048576) { /*Upto 512MB*/
	    sectorsPerCluster = 16;
	}
#endif

	do {
	    sectorsPerCluster <<= 1; /*first round = 2*/
	    clusters = (map->blocks-RES_SECT/*reserved*/) / sectorsPerCluster;
	} while (clusters >= 65525);
	fatSize = (clusters+255)/256;

	puthex6(map->blocks);
	puts("=sectors");
	puthex6(clusters);
	puts("=clusters");
	puthex(sectorsPerCluster);
	puts("=secspercluster");
	puthex(fatSize);
	puts("=fatSize");

	memset(myMinifatBuffer, 0, 512/2);
	memcpyYX(myMinifatBuffer, MBR, sizeof(MBR));
	myMinifatBuffer[255] = 0x55aa;
	myMinifatBuffer[  6] = 0x0200 | sectorsPerCluster;
	myMinifatBuffer[ 16] = SwapWord(map->blocks);
	myMinifatBuffer[ 17] = SwapWord(map->blocks>>16);
	myMinifatBuffer[ 11] = SwapWord(fatSize); /* FAT size */
	myMinifatBuffer[ 12] = SwapWord(map->blocks>>4); /* sectors per track */
	myMinifatBuffer[ 13] = 0x1000; /* number of tracks = 16*/
	map->Write(map, 0, 1, myMinifatBuffer);

	/* Allocates reserved clusters */
	memset(myMinifatBuffer, 0, 512/2);
	myMinifatBuffer[0] = 0xf8ff;
	myMinifatBuffer[1] = 0xffff;
	map->Write(map, RES_SECT, 1, myMinifatBuffer);
	map->Write(map, RES_SECT+fatSize, 1, myMinifatBuffer);
	map->Flush(map, 1);

	map = FsMapTnCreate(ph, 0); /* tiny mapper */
    }

	break;
#endif

    case 10:
    {
      u_int32 sector;
      int i;

      while (1) {
	fputs("sector: ", stdout);
	fflush(stdout);
	fgets(in, 200, stdin);
	i = strlen(in);
	while (i && (in[i-1] == '\n' || in[i-1] == '\r'))
	  in[--i] = '\0';
	sector = strtol(in, &tmp, 0);
	if (!in[0] || (tmp && *tmp && *tmp != '\n' && *tmp != '\r'))
	  break;
#if 1
	if (ph->Read(ph, sector, 1, myMinifatBuffer, tmpBuf) != 1) {
	  puts("Error");
	}
#else
	if (ph->Read(ph, sector, 1, myMinifatBuffer, NULL) != 1) {
	  puts("Error");
	}
	if (ph->Read(ph, sector, 1, NULL, tmpBuf) != 1) {
	  puts("Error");
	}
#endif
	PrintSector(myMinifatBuffer, tmpBuf);
	puts("");
      }
      break;
    }

    case 11:
    {
      u_int32 sector;
      int i;

      while (1) {
	fputs("Erase sector: ", stdout);
	fflush(stdout);
	fgets(in, 200, stdin);
	i = strlen(in);
	while (i && (in[i-1] == '\n' || in[i-1] == '\r'))
	  in[--i] = '\0';
	sector = strtol(in, &tmp, 0);
	if (!in[0] || (tmp && *tmp && *tmp != '\n' && *tmp != '\r'))
	  break;

	if (i = ph->Erase(ph, sector)) {
	  puthex(i);
	  puts(" Error");
	}
	//PrintSector(myMinifatBuffer, tmpBuf);
	//puts("");
      }
      break;
    }
    case 111:
      /* Erase all */
    {
      u_int32 sector, sectors = (u_int32)ph->eraseBlockSize * ph->eraseBlocks;
      int i;
      FILE *fp;

      puthex6(ph->eraseBlockSize);
      puts("=eraseBlockSize");
      puthex6(ph->eraseBlocks);
      puts("=eraseBlocks");
      puthex6(sectors);
      puts("=sectors");

      for (sector = 0x0; sector < sectors; sector += ph->eraseBlockSize) {
	if (ph->Erase(ph, sector)) {
	  puthex6(sector);
	  puts("Error");
#if 1 /*mark bad*/
	  memset(tmpBuf, 0xffffU, 8);
	  tmpBuf[0+2] = 0;
	  ph->Write(ph, sector, 1, NULL, tmpBuf);
#endif
	}
      }
      puts("Done");
      fflush(stdout);
      break;
    }

#if 1
    case 12:
    {
      u_int32 sector;
      int i;

      while (1) {
	fputs("#: ", stdout);
	fflush(stdout);
	fgets(in, 200, stdin);
	i = strlen(in);
	while (i && (in[i-1] == '\n' || in[i-1] == '\r'))
	  in[--i] = '\0';
	sector = strtol(in, &tmp, 0);
	if (!in[0] || (tmp && *tmp && *tmp != '\n' && *tmp != '\r'))
	  break;

	memset(myMinifatBuffer, -2, 256);
	for (i=0;i<256;i+=4)
	  myMinifatBuffer[i] = i*257;
	memset(tmpBuf, -1, 8);
	//tmpBuf[1] = 0x0101;
	//tmpBuf[7] = 0x0707;
	if (ph->Write(ph, sector, 1, myMinifatBuffer, tmpBuf) != 1) {
	  puts("WrErr");
	}
	if (ph->Read(ph, sector, 1, myMinifatBuffer, tmpBuf) != 1) {
	  puts("RdErr");
	  PrintSector(myMinifatBuffer, tmpBuf);
	}
	//puts("");
      }
      break;
    }
#endif

#ifdef WR_IMAGE
    case 14:
    {
	FILE *fp;
	int i;

	fputs("Filename: ", stdout);
	fflush(stdout);
	fgets(in, 200, stdin);
	i = strlen(in);
	while (i && (in[i-1] == '\n' || in[i-1] == '\r'))
	    in[--i] = '\0';

	fp = fopen(in, "rb");
	if (fp) {
	    u_int32 sector = 0;
	    puts("Programming");
	    PERIP(INT_ENABLEL0) &= ~INTF_UART_RX;
#define BLOCK_SIZE 2048
//#define BLOCK_SIZE 512
	    memset(myMallocAreaX, -1, BLOCK_SIZE/2);
	    while (fread(myMallocAreaX, 1, BLOCK_SIZE/2, fp)) {
	      if ((sector & (ph->eraseBlockSize-1)) == 0) {
		puthex6(sector);
		puts("Erasing");
		ph->Erase(ph, sector);
	      }
	      memset(tmpBuf, -1, 4*16/2);
	      /*
		"70ns" (82ns) separately 4*512 w+r, 5.5*400us = 2.2ms
		"70ns" (82ns) 2048 w+r, 3.2*400us = 1.28ms
		"25ns" (34ns) 2048 w+r = 0.83ms
	       */
	      if (
#if 1 && BLOCK_SIZE == 2048
		  LargePageWrite(sector, myMallocAreaX, tmpBuf) != 0
#else
		  ph->Write(ph, sector, BLOCK_SIZE/512, myMallocAreaX, tmpBuf) != BLOCK_SIZE/512
		  //MyNandPutPage(sector, myMallocAreaX, tmpBuf) != 0
#endif
		  ) {
		puts("WrError");
	      }
#if 1 /*Compare*/
	      {
		static u_int16 cmp[BLOCK_SIZE/2], spare[64/2];
		if (
#if 1 && BLOCK_SIZE == 2048
		    LargePageRead(sector, cmp, spare) != 0
#else
		    ph->Read(ph, sector, BLOCK_SIZE/512, cmp, spare) != BLOCK_SIZE/512
#endif
		    ) {
		  puthex6(sector);
		  puts("Read Error");
		}
		{
		  if (memcmp(cmp, myMallocAreaX, BLOCK_SIZE/2)) {
		    int i;
		    for (i=0;i<BLOCK_SIZE/2;i++) {
		      if (myMallocAreaX[i] != cmp[i]) {
			printf("%3d %04x %04x\n", i, myMallocAreaX[i], cmp[i]);
			break;
		      }
		    }
		    puthex6(sector);
		    puts("Compare Error");
		  } else if (memcmp(spare, tmpBuf, BLOCK_SIZE/32/2)) {
		    int i;
		    for (i=0;i<BLOCK_SIZE/32/2;i++) {
		      if (tmpBuf[i] != spare[i]) {
			printf("%3d %04x %04x\n", i, tmpBuf[i], spare[i]);
			break;
		      }
		    }
		    puthex6(sector);
		    puts("Spare Error");
		  }
		}
	      }
	      if (sector == 0x600)
		break;
#endif
	      sector += BLOCK_SIZE/512;
	      memset(myMallocAreaX, -1, BLOCK_SIZE/2);
	    }
	    fclose(fp);
	    puthex(sector);
	    puts("=sectors written");
	    PERIP(INT_ENABLEL0) |= INTF_UART_RX;
	}
        break;
    }
#endif

#if 0
    case 51:
	if (!(ph->eraseBlocks & 1))
	    ph->eraseBlocks--;
	/* fall through */
    case 50:
      map = FsMapFlCreate(ph, 1); /* format */
      map->Flush(map, 1);
      map->Delete(map);
      puthex(map->blocks>>16);
      puthex(map->blocks);
      puts("=map blocks: Mapper format complete");
      break;
#endif

#ifdef CHECK_MAP
    case 23:
	/* find root and follow */
    {
	register int i;
	u_int32 sector = 0;
	memset(tmpBuf, -1, 16/2);
	for (i=1;i<11;i++) {
	    sector = (u_int32)i * ph->eraseBlockSize;
	    ph->Read(ph, sector, 1, myMinifatBuffer, tmpBuf);
	    if ((tmpBuf[1] & 0xff) == 1)
		break;
	}
	if ((tmpBuf[1] & 0xff) == 1) {
	    __y u_int32 root;
	    while (1) {
		puthex6(sector);
		puts(" root");
		if ((tmpBuf[1] & 0xff) != 1) {
		    puthex((tmpBuf[1] & 0xff));
		    puts(" root type mismatch");
		    break;
		}
		root = sector;
		if (*(u_int32*)&tmpBuf[6] == 0xffffffffUL)
		    break;
		sector = *(u_int32*)&tmpBuf[6];
		ph->Read(ph, sector, 1, myMinifatBuffer, tmpBuf);
	    }

	} else {
	    puts("root not found");
	}

    }
      break;
#endif

    case 1:
      /* Scan for bad blocks */
    {
      u_int32 sector, sectors = (u_int32)ph->eraseBlockSize * ph->eraseBlocks;
      int i;
      FILE *fp;

      fputs("Filename (or empty): ", stdout);
      fflush(stdout);
      fgets(in, 200, stdin);
      i = strlen(in);
      while (i && (in[i-1] == '\n' || in[i-1] == '\r'))
	in[--i] = '\0';

      puthex6(ph->eraseBlockSize);
      puts("=eraseBlockSize");
      puthex6(ph->eraseBlocks);
      puts("=eraseBlocks");
      puthex6(sectors);
      puts("=sectors");

      if (in[0] && (fp = fopen(in, "wb"))) {
	fputs("Dumping bad blocks to \"", stdout);
	fputs(in, stdout);
	puts("\"\n");
      } else {
	fp = stdout;
      }
      puts("Scanning Bad Blocks");
      for (sector = 0x0; sector < sectors; sector += 1) {
	if (ph->Read(ph, sector, 4, NULL, tmpBuf) != 4) {
	}
	if ((tmpBuf[2] & 0xff) != 0xff) {
	  fputhex6(sector, fp);
	  fputs("=B\n", fp);
	}
      }
      if (fp && fp != stdout)
	fclose(fp);
      puts("Done");
      fflush(stdout);
      break;
    }
#ifdef CHECK_PHYS
    case 22:
      /* check physical */
    {
      u_int32 sector, sectors =
	(u_int32)ph->eraseBlockSize * ph->eraseBlocks;
      int i;

      puthex6(ph->eraseBlockSize);
      puts("=eraseBlockSize");
      puthex6(ph->eraseBlocks);
      puts("=eraseBlocks");
      puthex6(sectors);
      puts("=sectors");

      for (sector = 0x0; sector < sectors; sector++) {
	  if ((sector & 0x1ff) == 0) {
	      putchar('\r');
	      puthex6(sector);
	      fflush(stdout);
	  }
	  if (ph->Read(ph, sector, 1, myMinifatBuffer, tmpBuf) != 1 &&
	      (tmpBuf[1] & 0xff) != 0xff) {
#if 1
	      puthex6(sector);
	      puts(" N/A");
#endif
	  }
      }
      break;
    }
#endif

#ifdef DUMP_PHYS
    case 2:
      /* dump physical */
    {
      u_int32 sector, sectors =
	(u_int32)ph->eraseBlockSize * ph->eraseBlocks;
      u_int32 __y na = 0;
      FILE *fp;
      int i;

      fputs("Filename: ", stdout);
      fflush(stdout);
      fgets(in, 200, stdin);
      i = strlen(in);
      while (i && (in[i-1] == '\n' || in[i-1] == '\r'))
	in[--i] = '\0';

      puthex6(ph->eraseBlockSize);
      puts("=eraseBlockSize");
      puthex6(ph->eraseBlocks);
      puts("=eraseBlocks");
      puthex6(sectors);
      puts("=sectors");

      fp = fopen(in, "wb");
      if (fp) {
	fputs("Dumping Physical to \"", stdout);
	fputs(in, stdout);
	puts("\"\n");
	for (sector = 0x0; sector < sectors; sector++) {
	  if ((sector & 0x1ff) == 0) {
	    putchar('\r');
	    puthex6(sector);
	    putchar(' ');
	    puthex6(na);
	    fflush(stdout);
	  }
	  if (ph->Read(ph, sector, 1, myMinifatBuffer,
#if 1
		       tmpBuf
#else
		       (sector < ph->eraseBlockSize) ? NULL : tmpBuf
#endif
	      ) != 1) {
	    na++;
#if 0
	    puthex6(sector);
	    puts(" N/A");
#endif
	  }
	  fwrite(tmpBuf, 16/2, 1, fp);
	  fwrite(myMinifatBuffer, 512/2, 1, fp);
	}
	fclose(fp);
	putchar('\n');
	puthex6(na);
	puts("=N/A\nDump done.");
      } else {
#if 0
	fputs("Could not open \"", stdout);
	fputs(in, stdout);
	puts("\" for writing\n");
#endif
      }
      break;
    }
#endif

#ifdef DUMP_FAT
    case 3:
    {
      u_int32 sector, sectors;
      FILE *fp;
      int i;

      fputs("Filename: ", stdout);
      fflush(stdout);
      fgets(in, 200, stdin);
      i = strlen(in);
      while (i && (in[i-1] == '\n' || in[i-1] == '\r'))
	in[--i] = '\0';

      puts("Tiny-Mapper");
      map = FsMapTnCreate(ph, 0); /* tiny mapper */
      sectors = map->blocks;

      /* Dump fat */
      puthex6(sectors);
      puts("=sectors");

      fp = fopen(in, "wb");
      if (fp) {
	fputs("Dumping Logical (FAT) to \"", stdout);
	fputs(in, stdout);
	puts("\"\n");
	for (sector = 0x0; sector < sectors; sector++) {
	  if ((sector & 0x1ff) == 0) {
	    putchar('\r');
	    puthex(sector);
	    fflush(stdout);
	  }
	  if (map->Read(map, sector, 1, myMinifatBuffer) != 1) {
	    memset(myMinifatBuffer, 0, 256);
#if 0
	    puthex6(sector);
	    puts(" N/A");
#endif
	  }
	  fwrite(myMinifatBuffer, 512/2, 1, fp);
	}
	fclose(fp);
	puts("Dump done");
      } else {
	fputs("Could not open \"", stdout);
	fputs(in, stdout);
	puts("\" for writing\n");
      }
      map->Delete(map);
      break;
    }
#endif

#ifdef ANYFLASH
    case 120:
    {
      /* small page, 32 sectors per block, 128MB flash */
      int blockSizeExp = 5, flashSizeExp = 18, i;
      long wsns = GetParam("Waits (ns): ");
      __y long tmp;
      if (wsns == NAN)
	break;

      tmp = GetParam("NandType: ");
      if (tmp == NAN)
	break;
      ((struct FsNandPhys *)ph)->nandType = tmp;
      ((struct FsNandPhys *)ph)->waitns = wsns;

      tmp = GetParam("blockSizeExpr: ");
      if (tmp == NAN)
	break;
      blockSizeExp = tmp;
      ph->eraseBlockSize = (1 << blockSizeExp);
      tmp = GetParam("flashSizeExpr: ");
      if (tmp == NAN)
	break;
      flashSizeExp = tmp;
      ph->eraseBlocks = 1 << (flashSizeExp-blockSizeExp);

      puthex(1<<blockSizeExp);
      puts("=eraseBlockSize");
      puthex(ph->eraseBlocks);
      puts("=eraseBlocks");


      if (i = ph->Erase(ph, 0)) {
	puthex(i);
	puts("Erase failed");
      }
      memset(myMinifatBuffer, -1/*0xffffU*/, 256);
      myMinifatBuffer[0] = ('V' << 8) + 'L';
      myMinifatBuffer[1] = ('S' << 8) + 'I';
      myMinifatBuffer[2] = ((struct FsNandPhys *)ph)->nandType;
      myMinifatBuffer[3] = (blockSizeExp << 8) + flashSizeExp;
      myMinifatBuffer[4] = ((struct FsNandPhys *)ph)->waitns;
      memset(tmpBuf, -1, sizeof(tmpBuf));

      if (ph->Write(ph, 0, 1, myMinifatBuffer, tmpBuf) != 1) {
	puts("Write failed");
      }
      puts("Inited");
      break;
    }
#else
    case 128:
    {
      /* large page, 256 sectors per block, 256MB flash */
      int blockSizeExp = 8, flashSizeExp = 19;
      long wsns = GetParam("Waits (ns): ");
      if (wsns == NAN)
	break;

      ((struct FsNandPhys *)ph)->nandType = 0x0003;
      ((struct FsNandPhys *)ph)->waitns = wsns;

      ph->eraseBlockSize = (1 << blockSizeExp);
      ph->eraseBlocks = 1 << (flashSizeExp-blockSizeExp);

      ph->Erase(ph, 0);
      memset(myMinifatBuffer, -1/*0xffffU*/, 256);
      myMinifatBuffer[0] = ('V' << 8) + 'L';
      myMinifatBuffer[1] = ('S' << 8) + 'I';
      myMinifatBuffer[2] = ((struct FsNandPhys *)ph)->nandType;
      myMinifatBuffer[3] = (blockSizeExp << 8) + flashSizeExp;
      myMinifatBuffer[4] = ((struct FsNandPhys *)ph)->waitns;

      ph->Write(ph, 0, 1, myMinifatBuffer, NULL);

      { int i;
      for(i=0;i<256;i++) {
	  if(myMinifatBuffer[i] != 0xffffU) {
	      puthex(i);
	      puthex(myMinifatBuffer[i]);
	      puts("");
	  }
      }
      }

      puts("256MB L, 64(256)");
      break;
    }
#endif

#if 0
    case 129:
    {
      /* small page, 32 sectors per block, 128MB flash */
      int blockSizeExp = 5, flashSizeExp = 18;
      long wsns = GetParam("Waits (ns): ");
      if (wsns == NAN)
	break;

      ((struct FsNandPhys *)ph)->nandType = 0x0002;
      ((struct FsNandPhys *)ph)->waitns = wsns;

      ph->eraseBlockSize = (1 << blockSizeExp);
      ph->eraseBlocks = 1 << (flashSizeExp-blockSizeExp);

      ph->Erase(ph, 0);
      memset(myMinifatBuffer, -1/*0xffffU*/, 256);
      myMinifatBuffer[0] = ('V' << 8) + 'L';
      myMinifatBuffer[1] = ('S' << 8) + 'I';
      myMinifatBuffer[2] = ((struct FsNandPhys *)ph)->nandType;
      myMinifatBuffer[3] = (blockSizeExp << 8) + flashSizeExp;
      myMinifatBuffer[4] = ((struct FsNandPhys *)ph)->waitns;

      ph->Write(ph, 0, 1, myMinifatBuffer, NULL);
      puts("Inited 128MB Small page flash, 32 sectors per block");
      break;
    }
#endif
#if 0
    case 130:
    {
      /* small page, 32 sectors per block, 64MB flash */
      int blockSizeExp = 5, flashSizeExp = 17;
      long wsns = GetParam("Waits (ns): ");
      if (wsns == NAN)
	break;

      ((struct FsNandPhys *)ph)->nandType = 0x0002;
      ((struct FsNandPhys *)ph)->waitns = wsns;

      ph->eraseBlockSize = (1 << blockSizeExp);
      ph->eraseBlocks = 1 << (flashSizeExp-blockSizeExp);

      ph->Erase(ph, 0);
      memset(myMinifatBuffer, -1/*0xffffU*/, 256);
      myMinifatBuffer[0] = ('V' << 8) + 'L';
      myMinifatBuffer[1] = ('S' << 8) + 'I';
      myMinifatBuffer[2] = ((struct FsNandPhys *)ph)->nandType;
      myMinifatBuffer[3] = (blockSizeExp << 8) + flashSizeExp;
      myMinifatBuffer[4] = ((struct FsNandPhys *)ph)->waitns;

      ph->Write(ph, 0, 1, myMinifatBuffer, NULL);
      puts("Inited");
      break;
    }
#endif
#if 0
    case 131:
    {
      /* small page, 32 sectors (16kB) per block, 16MB flash */
      int blockSizeExp = 5, flashSizeExp = 15;
      long wsns = GetParam("Waits (ns): ");
      if (wsns == NAN)
	break;

      ((struct FsNandPhys *)ph)->nandType = 0x0000;
      ((struct FsNandPhys *)ph)->waitns = wsns;

      ph->eraseBlockSize = (1 << blockSizeExp);
      ph->eraseBlocks = 1 << (flashSizeExp-blockSizeExp);

      ph->Erase(ph, 0);
      memset(myMinifatBuffer, -1/*0xffffU*/, 256);
      myMinifatBuffer[0] = ('V' << 8) + 'L';
      myMinifatBuffer[1] = ('S' << 8) + 'I';
      myMinifatBuffer[2] = ((struct FsNandPhys *)ph)->nandType;
      myMinifatBuffer[3] = (blockSizeExp << 8) + flashSizeExp;
      myMinifatBuffer[4] = ((struct FsNandPhys *)ph)->waitns;

      ph->Write(ph, 0, 1, myMinifatBuffer, NULL);
      puts("Inited");
      break;
    }
#endif

    default:
      puthex(mode);
      puts("mode");
    }
  }

#endif
}
