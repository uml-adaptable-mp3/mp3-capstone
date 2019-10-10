/*

  FatInfo - Show information of a FAT32 device

*/
#include <vo_stdio.h>
#include <volink.h>     /* Linker directives like DLLENTRY */
#include <apploader.h>  /* RunLibraryFunction etc */
#include <string.h>
#include <stdlib.h>
#include <audio.h>
#include <consolestate.h>
#include <saturate.h>
#include <vo_fat.h>
#include <swap.h>
#include <ctype.h>
#include <kernel.h>
#include "bytemem.h"

u_int16 myBuf[256];

void PrintBlock(register DEVICE *dev, register u_int32 block, register int verbose) {
  int i;
  u_int16 *p = myBuf;
#if 0
  printf("PrintBlock(dev=%p, bl=%ld, v=%d)\n", dev, block, verbose);
#endif

  if (dev->BlockRead(dev, block, 1, myBuf)) {
    printf("E: Media cannot be read\n");
    return;
  }

  if (!verbose) return;
  printf("  Block 0x%lx (%ld)\n", block, block);
  for (i=0; i<256; i+=8) {
    int j;
    printf("    %03x:", 2*i);
    for (j=0; j<8; j++) {
      printf(" %04x", *p++);
    }
    printf(" ");
    p -= 8;
    for (j=0; j<8; j++) {
      int k;
      for (k=1; k>=0; k--) {
	int c = (*p >> 8*k) & 0xFF;
	char ch = (isprint(c) && c != 0xff) ? c : '.';
	printf("%c", ch);
      }
      p++;
    }
    printf("\n");
  }
}


s_int16 PrintFat32(register DEVICE *dev, register u_int32 partitionStart, register int verbose) {
  int i, c;
  int retVal = -1;
  int canTBeFat32 = 0;
  u_int16 t1;

  u_int32 fatStart;
  u_int32 fatSize;
  u_int32 rootStart;

  /* FAT SECTOR 0 */
  PrintBlock(dev, partitionStart, verbose);

  printf("  FAT32 SECTOR 0:\n");
  printf("    Jump Code + NOP: ");
  for (i=0x00; i<0x03; i++) {
    printf(" %02x", GetBE8(myBuf, i));
  }
  printf("\n");
  printf("    OEM Name: ");
  for (i=0; i<2; i++) {
    int j;
    for (j=0x03; j<0x0b; j++) {
      printf(!i ? " %02x" : "%c", GetBE8(myBuf, j));
    }
    printf(!i ? " \"" : "\"\n");
  }
  printf("    Bytes Per Sector %d\n"
	 "    Sectors Per Cluster %d\n"
	 "    Reserved Sectors %d\n"
	 "    Number Of FATs %d\n",
	 GetLE16(myBuf, 0x0b),
	 GetBE8(myBuf, 0x0d),
	 GetLE16(myBuf, 0x0e),
	 (t1 = GetBE8(myBuf, 0x10)));
  if (!t1) {
    printf("      -> CAN'T BE FAT32 (perhaps exFAT?)\n");
    canTBeFat32 = 1;
  }
  printf("    16-bit Number of Sectors per FAT %d\n", (t1=GetLE16(myBuf, 0x13)));
  if (t1) {
    printf("      -> CAN'T BE FAT32\n");
    canTBeFat32 = 1;
  }
  printf("    Media Descriptor (0xf8 for HDs) 0x%x\n", GetBE8(myBuf, 0x15));
  printf("    16-bit FAT size %d\n", (t1=GetLE16(myBuf, 0x16)));
  if (t1) {
    printf("      -> CAN'T BE FAT32\n");
    canTBeFat32 = 1; 
  }
  printf("    Sectors Per Track %d\n"
	 "    Number of Heads %d\n"
	 "    Number of Hidden Sectors in Partition %ld\n"
	 "    32-bit Number of Sectors in Partition %ld\n",
	 GetLE16(myBuf, 0x18),
	 GetLE16(myBuf, 0x1a),
	 GetLE32(myBuf, 0x1c),
	 GetLE32(myBuf, 0x20));
  if (canTBeFat32) {
    goto finally;
  }
  printf("    32-bit Number of Sectors per FAT %ld\n"
	 "    Flags 0x%x\n"
	 "    Version of FAT32 Drive 0x%x\n"
	 "    Cluster Number of the Start of the Root Directory %ld\n"
	 "    Sector Number of the FileSystem Information Sector %d\n"
	 "    Sector Number of the Backup Boot Sector %d\n"
	 "    Logical Drive Number of Partition %d\n"
	 "    Extended Signature (0x29) 0x%x\n"
	 "    Serial Number of Partition 0x%04lx\n",
	 GetLE32(myBuf, 0x24),
	 GetLE16(myBuf, 0x28),
	 GetLE16(myBuf, 0x2a),
	 GetLE32(myBuf, 0x2c),
	 GetLE16(myBuf, 0x30),
	 GetLE16(myBuf, 0x32),
	 GetLE16(myBuf, 0x40),
	 GetBE8 (myBuf, 0x42),
	 GetLE32(myBuf, 0x43));
  printf("    Volume name of Partition:");
  for (i=0; i<2; i++) {
    int j;
    for (j=0x47; j<0x52; j++) {
      printf(!i ? " %02x" : "%c", GetBE8(myBuf, j));
    }
    printf(!i ? " \"" : "\"\n");
  }
  retVal = 0;
  printf("    Boot Record Signature (0x55 0xaa) 0x%02x 0x%02x\n", GetBE8(myBuf, 0x1fe), GetBE8(myBuf, 0x1ff));


  fatStart = 0+GetLE16(myBuf, 0x0e);
  fatSize = GetLE32(myBuf, 0x24);
  rootStart = fatStart + fatSize * GetBE8(myBuf, 0x10);

  /* FAT SECTOR 1 */
  PrintBlock(dev, partitionStart+1, verbose);

  printf("  FAT32 SECTOR 1:\n"
	 "    First Signature (0x52 0x52 0x61 0x41) 0x%02x 0x%02x 0x%02x 0x%02x\n"
	 "    Signature of FSInfo Sector (0x72 0x72 0x41 0x61) 0x%02x 0x%02x 0x%02x 0x%02x\n"
	 "    Number of Free Clusters 0x%08lx\n"
	 "    Most Recently Allocated Cluster 0x%08lx\n"
	 "    Boot Record Signature (0x55 0xaa) 0x%02x 0x%02x\n",
	 GetBE8(myBuf, 0), GetBE8(myBuf, 1), GetBE8(myBuf, 2), GetBE8(myBuf, 3),
	 GetBE8(myBuf, 0x1e4), GetBE8(myBuf, 0x1e5), GetBE8(myBuf, 0x1e6), GetBE8(myBuf, 0x1e7),
	 GetLE32(myBuf, 0x1e8),
	 GetLE32(myBuf, 0x1ec),
	 GetBE8(myBuf, 0x1fe), GetBE8(myBuf, 0x1ff));

  printf("  fatStart %ld, fatSize %ld, rootStart %ld\n",
	 fatStart, fatSize, rootStart);

  /* FAT ROOT DIRECTORY SECTOR */
  printf("  FAT ROOT SECTOR 0x%lx (%ld)\n", partitionStart+rootStart, partitionStart+rootStart);
  PrintBlock(dev, partitionStart+rootStart, verbose);

 finally:
  return retVal;
}

ioresult main(char *parameters) {
  int i, nParam;
  char *p = parameters;
  ioresult retCode = S_ERROR;
  FILE *fp = NULL;
  int verbose = 0;
  DEVICE *dev = NULL;
  int didSomething = 0;
  u_int32 t32;
  char driveLetter = '\0';

  nParam = RunProgram("ParamSpl", parameters);
  for (i=0; i<nParam; i++) {
    char *endP;
    u_int32 paramVal = strtol(p, &endP, 0);
    if (!strcmp(p, "-h")) {
      printf("Usage: FatInfo x: [blkNum] [-v|+v|-h]\n"
	     "x:\tDrive name\n"
	     "blkNum\tPrint contents of a disk block\n"
	     "-v|+v\tVerbose on/off\n"
	     "-h\tShow this help\n");
      goto finally;
    } else if (!strcmp(p, "-v")) {
      verbose = 1;
    } else if (!strcmp(p, "+v")) {
      verbose = 0;
    } else if (!(*endP)) {
      if (!dev) {
	printf("E: No drive letter specified\n");
	goto finally;
      }
      PrintBlock(dev, paramVal, 1);
      didSomething++;
    } else {
      driveLetter = toupper(*p);
      dev = NULL;
      if (strlen(p) != 2 || p[1] != ':' ||
	  driveLetter < 'A' || driveLetter > 'Z') {
	printf("E: Illegal drive name \"%s\"\n", p);
	goto finally;
      }
      if (!(dev = VODEV(driveLetter))) {
	printf("E: Non-existing drive \"%s\"\n", p);
	goto finally;
      }
      ioctl(dev, IOCTL_RESTART, NULL);
    }
    p += strlen(p)+1;
  }

  if (didSomething) {
    goto finally;
  }

  if (!dev) {
    printf("E: No drive letter specified\n");
    goto finally;
  }

  if (dev->BlockRead(dev, 0, 1, myBuf)) {
    printf("E: Media cannot be read\n");
    goto finally;
  }

  if (GetBE8(myBuf, 0x0) == 0xeb) {
    printf("FAT FOUND IN ROOT BLOCK\n");

    PrintFat32(dev, 0, verbose);
  } else if (GetBE16(myBuf, 0x1fe) == 0x55aa) {
    u_int16 partitionBase = 0x1be;
    printf("FOUND PARTITION TABLE\n");

    PrintBlock(dev, 0, verbose);

    for (i=0; i<4; i++) {
      t32 = GetLE32(myBuf, partitionBase+8);
      printf("PARTITION%d = 0x%08lx", i+1, t32);
      if (t32) {
	static const char *partTypes[] = {
	  "empty", "FAT12", "XENIX root", "XENIX usr",
	  "FAT16", "Ext.part. CHS", "FAT16B", "IFS/HPFS/NTFS/exFAT/QNX",
	  "FAT12/FAT16/OS2/AIX/etc", "QNX/OS9/OS2/etc", "OS2 Boot/Coherent swap", "FAT32 CHS",
	  "FAT32 LBA", "reserved", "FAT16B LBA", "Ext.part. LBA"
	};
	u_int16 partitionType = GetBE8(myBuf, partitionBase+4);
	printf(", type 0x%02x (%s)\n",
	       partitionType,
	       partitionType < sizeof(partTypes)/sizeof(partTypes[0]) ?
	       partTypes[partitionType] : "unknown");
	/* Lexar 128 GB has partitions of type 0xf4, address 0xf4f4f4f4 */
	if (t32 != 0xf4f4f4f4) {
	  PrintFat32(dev, t32, verbose);
	}
      } else {
	printf(" (none)\n");
      }
      partitionBase += 0x10;
    }
  } else {
    printf("FOUND NOTHING IN BLOCK 0\n");
    PrintBlock(dev, 0, verbose);
  }

  retCode = S_OK;
 finally:
  if (fp) {
    fclose(fp);
    fp = NULL;
  }
  
  return retCode;
}
