/*

  Format - Format a FAT32 volume

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
#include <time.h>
#include <math.h>
#include <kernel.h>
#include "bytemem.h"

#define DEFAULT_PARTITION_SECTORS 8192
#define DEFAULT_ALIGN_FAT 8192

u_int16 myBuf[256];

u_int16 fatRoot[6][256] = {
  {
    /* 0: ROOT: BLOCK 0, ALSO MAKE A COPY TO BLOCK 6 */
    /*000:*/ 0xeb58,0x9076,0x6c73,0x6964,0x6973,0x6b00,0x0208,0x2000, /*ëX.vlsidisk... .*/
    /*010:*/ 0x0200,0x0000,0x00f8,0x0000,0x3d00,0x1f00,0x0000,0x0000, /*.....ø..=.......*/
    /*020:*/ 0x0070,0x1d00,0x5907,0x0000,0x0000,0x0000,0x0200,0x0000, /*.p..Y...........*/
    /*030:*/ 0x0100,0x0600,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*040:*/ 0x8000,0x2964,0x3bac,0xf356,0x2020,0x2020,0x2020,0x2020, /*..)d;¬óV       .*/
    /*050:*/ 0x2020,0x4641,0x5433,0x3220,0x2020,0x0e1f,0xbe77,0x7cac, /*  FAT32   ..¾w|¬*/
    /*060:*/ 0x22c0,0x740b,0x56b4,0x0ebb,0x0700,0xcd10,0x5eeb,0xf032, /*"Àt.V´.»..Í.^ëð2*/
    /*070:*/ 0xe4cd,0x16cd,0x19eb,0xfe54,0x6869,0x7320,0x6973,0x206e, /*äÍ.Í.ë.This is n*/
    /*080:*/ 0x6f74,0x2061,0x2062,0x6f6f,0x7461,0x626c,0x6520,0x6469, /*ot a bootable di*/
    /*090:*/ 0x736b,0x2e20,0x2050,0x6c65,0x6173,0x6520,0x696e,0x7365, /*sk.  Please inse*/
    /*0a0:*/ 0x7274,0x2061,0x2062,0x6f6f,0x7461,0x626c,0x6520,0x666c, /*rt a bootable fl*/
    /*0b0:*/ 0x6f70,0x7079,0x2061,0x6e64,0x0d0a,0x7072,0x6573,0x7320, /*oppy and..press */
    /*0c0:*/ 0x616e,0x7920,0x6b65,0x7920,0x746f,0x2074,0x7279,0x2061, /*any key to try a*/
    /*0d0:*/ 0x6761,0x696e,0x202e,0x2e2e,0x200d,0x0a00,0x0000,0x0000, /*gain ... .......*/
    /*0e0:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*0f0:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*100:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*110:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*120:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*130:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*140:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*150:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*160:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*170:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*180:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*190:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*1a0:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*1b0:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*1c0:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*1d0:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*1e0:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*1f0:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x55aa, /*..............Uª*/
  }, {
    /* 1: FILE SYSTEM INFORMATION: BLOCK 1 */
    /*000:*/ 0x5252,0x6141,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*RRaA............*/
    /*010:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*020:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*030:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*040:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*050:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*060:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*070:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*080:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*090:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*0a0:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*0b0:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*0c0:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*0d0:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*0e0:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*0f0:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*100:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*110:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*120:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*130:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*140:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*150:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*160:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*170:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*180:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*190:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*1a0:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*1b0:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*1c0:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*1d0:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /*................*/
    /*1e0:*/ 0x0000,0x0000,0x7272,0x4161,0xffff,0xffff,0x0200,0x0000, /*....rrAa........*/
    /*1f0:*/ 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x55aa, /*..............Uª*/
  }, {
    /* 2: FAT */
    /*000:*/ 0xf8ff,0xff0f,0xffff,0xff0f,0xf8ff,0xff0f,0x0000,0x0000, /* ø.......ø.......*/
  }, {
    /* 3: ROOT DIRECTORY */
    /*000:*/ 0x5620,0x2020,0x2020,0x2020,0x2020,0x2008,0x0000,0x406b, /*V          ...@k*/
    /*010:*/ 0x954a,0x954a,0x0000,0x406b,0x954a,0x0000,0x0000,0x0000, /*.J.J..@k.J......*/
  }, {
    /* 4: EMPTY BLOCK */
    0
  }, {
    /* 5: MBR */
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    //    0x2016,0x0330,0x6900,0x0000,0x0000,0x0000,0x0000,0x0082, /* 1b0 */
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0082, /* 1b0 */
    0x0300,0x0cfe,0xffff,0x0020,0x0000,0x0058,0xdd01,0x0000, /* 1c0 */
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /* 1d0 */
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /* 1e0 */
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x55aa  /* 1f0 */
  }
};

/*

FAT32 Boot Record

This information is located in the first sector of every partition.

Offset   Description                                      Size
00h      Jump Code + NOP                                  3 Bytes
03h      OEM Name (Probably MSWIN4.1)                     8 Bytes
0Bh      Bytes Per Sector                                 1 Word
0Dh **** Sectors Per Cluster                              1 Byte  **** >= 8
0Eh **** Reserved Sectors                                 1 Word
10h **** Number of Copies of FAT                          1 Byte
11h      Maximum Root DirectoryEntries (N/A for FAT32)    1 Word
13h      Number of Sectors inPartition Smaller than 32MB  1 Word
         (N/A for FAT32)
15h      Media Descriptor (F8h forHard Disks)             1 Byte
16h      Sectors Per FAT in Older FATSystems (N/A for     1 Word
         FAT32)
18h      Sectors Per Track                                1 Word
1Ah      Number of Heads                                  1 Word
1Ch      Number of Hidden Sectors inPartition             1 Double Word
20h **** Number of Sectors inPartition                    1 Double Word
24h **** Number of Sectors Per FAT                        1 Double Word **** < 16384
28h      Flags (Bits 0-4 IndicateActive FAT Copy) (Bit 7
         Indicates whether FAT Mirroringis Enabled or
         Disabled ) (If FATMirroring is Disabled, the FAT 1 Word
         Information is onlywritten to the copy indicated
         by bits 0-4)
2Ah      Version of FAT32 Drive (HighByte = Major         1 Word
         Version, Low Byte = Minor Version)
2Ch      Cluster Number of the Startof the Root Directory 1 Double Word
30h      Sector Number of the FileSystem Information
         Sector (See Structure Below)(Referenced from the 1 Word
         Start of the Partition)
32h      Sector Number of the BackupBoot Sector           1 Word
         (Referenced from the Start of the Partition)
34h      Reserved                                         12 Bytes
40h      Logical Drive Number of Partition                1 Byte
41h      LSb = 1 = dirty, clear to 0                      1 Byte
42h      Extended Signature (29h)                         1 Byte
43h **** Serial Number of Partition                       1 Double Word
47h      Volume Name of Partition                         11 Bytes
52h      FAT Name (FAT32)                                 8 Bytes
5Ah      Executable Code                                  420 Bytes
1FEh     Boot Record Signature (55hAAh)                   2 Bytes

File System Information Sector

Usually this is the Second Sector of the partition, although since there is a
reference in the Boot Sector to it, I'm assuming it can be moved around. I
never got a complete picture of this one. Although I do know where the
important fields are at.

Offset    Description                                      Size
00h       First Signature (52h 52h 61h41h)                 1 Double Word
04h       Unknown, Currently (Might just be Null)           480 Bytes
1E4h      Signature of FSInfo Sector(72h 72h 41h 61h)      1 Double Word
1E8h      Number of Free Clusters (Setto -1 if Unknown)    1 Double Word
1ECh      Cluster Number of Cluster that was Most Recently 1 Double Word
          Allocated.
1F0h      Reserved                                         12 Bytes
1FCh      Unknown or Null                                  2 Bytes
1FEh      Boot Record Signature (55hAAh)                   2 Bytes

FAT32 Drive Layout

Offset                                                   Description
Start of Partition                                       Boot Sector
Start + # of ReservedSectors                             Fat Tables
                                                         Data Area
Start + # of Reserved + (#of Sectors Per FAT * 2)        (Starts
                                                         with Cluster #2)


 */





#if 0
auto u_int16 GetBE8(register __i0 u_int16 *p, register u_int16 __b0 byteOffset) {
  return (p[byteOffset>>1] >> ((1-(byteOffset&1))*8)) & 0xFF;
}

auto u_int16 GetLE16(register __i0 u_int16 *p, register u_int16 __b0 byteOffset) {
  return GetBE8(p, byteOffset) | (GetBE8(p, byteOffset+1)<<8);
}

auto u_int16 GetBE16(register __i0 u_int16 *p, register u_int16 __b0 byteOffset) {
  return GetBE8(p, byteOffset+1) | (GetBE8(p, byteOffset)<<8);
}

auto u_int32 GetLE32(register __i0 u_int16 *p, register u_int16 __b0 byteOffset) {
  return GetLE16(p, byteOffset) | ((u_int32)GetLE16(p, byteOffset+2)<<16);
}


auto u_int32 GetBE32(register __i0 u_int16 *p, register u_int16 __b0 byteOffset) {
  return GetBE16(p, byteOffset+2) | ((u_int32)GetBE16(p, byteOffset)<<16);
}

auto u_int16 BitReverse8(register __d0 u_int16 x) {
  x = ((x >> 1) & 0x55) | ((x & 0x55) << 1);
  x = ((x >> 2) & 0x33) | ((x & 0x33) << 2);
  x = ((x >> 4) & 0x0f) | ((x & 0x0f) << 4);
  return x;
}

auto u_int16 BitReverse16(register __d0 u_int16 x) {
  x = ((x >> 8) & 0x00ff) | ((x & 0x00ff) << 8);
  x = ((x >> 4) & 0x0f0f) | ((x & 0x0f0f) << 4);
  x = ((x >> 2) & 0x3333) | ((x & 0x3333) << 2);
  x = ((x >> 1) & 0x5555) | ((x & 0x5555) << 1);
  return x;
}

auto u_int32 BitReverse32(register __reg_d u_int32 x) {
  x = ((x >> 1) & 0x55555555) | ((x & 0x55555555) << 1);
  x = ((x >> 2) & 0x33333333) | ((x & 0x33333333) << 2);
  x = ((x >> 4) & 0x0f0f0f0f) | ((x & 0x0f0f0f0f) << 4);
  x = ((x >> 8) & 0x00ff00ff) | ((x & 0x00ff00ff) << 8);
  x = (x >> 16) | (x << 16);
  return x;
}


u_int32 BitReverseN32(register __reg_d u_int32 x, register __c0 u_int16 n) {
  return BitReverse32(x<<(32-n));
}

u_int16 BitReverseN16(register __d0 u_int16 x, register __c0 u_int16 n) {
  return BitReverse16(x<<(16-n));
}

void SetBE8(register __i0 u_int16 *p, register u_int16 __a0 byteOffset, register u_int16 __b0 data) {
  u_int16 d1 = data;
  MemCopyPackedBigEndian(p, byteOffset, &d1, 1, 1);
}

void SetBE16(register __i0 u_int16 *p, register u_int16 __a0 byteOffset, register u_int16 __b0 data) {
  u_int16 d1 = data;
  MemCopyPackedBigEndian(p, byteOffset, &d1, 0, 2);
}

void SetLE16(register __i0 u_int16 *p, register u_int16 __a0 byteOffset, register u_int16 __b0 data) {
  SetBE16(p, byteOffset, Swap16(data));
}

void SetBE32(register __i0 u_int16 *p, register u_int16 __a0 byteOffset, register u_int32 __reg_b data) {
  u_int16 d1[2];
  d1[0] = (u_int16)(data>>16);
  d1[1] = (u_int16)data;
  MemCopyPackedBigEndian(p, byteOffset, d1, 0, 4);
}

void SetLE32(register __i0 u_int16 *p, register u_int16 __a0 byteOffset, register u_int32 __reg_b data) {
  SetBE32(p, byteOffset, Swap32(data));
}
#endif


void SetCHS(register __i0 u_int16 *p, register u_int16 __a0 byteOffset, register u_int32 __reg_b data) {
  int cylinder, sector, head;
#if 0
  printf("C/H/S %ld (0x%lx) -> ", data, data);
#endif
  if (data >= 1024L*63L*255L) {
    data=1024L*63L*255L-1;
  }
  sector = (int)(data % 63)+1;
  data /= 63;
  head = (int)(data % 255);
  data /= 255;
  cylinder = (int)data;
#if 0
  printf("%d/%d/%d -> ", cylinder, head, sector);
#endif
  SetBE8(p, 0, head&0xFF);
  SetBE8(p, 1, ((cylinder>>8)<<6)|sector);
  SetBE8(p, 2, cylinder&0xFF);
#if 0
  printf("result 0x%06lx\n", GetBE32(p, 0) >> 8);
#endif
}





ioresult main(char *parameters) {
  int i, nParam;
  char *p = parameters;
  ioresult retCode = S_ERROR;
  int verbose = 1;
  DEVICE *dev = NULL;
  u_int32 t32[4];
  DiskGeometry geometry = {0};
  time_t tt = time(NULL);
  u_int32 forceSizeMiB = 0;
  u_int16 forceClusterSize = 0;
  int numberOfFats = 1;
  u_int32 serialNumber = BitReverse32(clock()) ^ ReadTimeCount();
  int justForce = 0, confirmation = 1;
  char driveLetter = '\0';
  char *label = "VSOS";
  int eraseAll = 0;
  s_int32 defaultPartitionSectors = DEFAULT_PARTITION_SECTORS;
  s_int32 partitionSectors = 0;
  s_int32 alignFat = DEFAULT_ALIGN_FAT;
  s_int32 reservedSectors = 0;
  int makePartitionTable = 0, dryRun = 0;


  nParam = RunProgram("ParamSpl", parameters);
  for (i=0; i<nParam; i++) {
    if (!strcmp(p, "-h")) {
      printf("Usage: Format x: [-v|+v|-llabel|-sx|-cx|-fx|-ix|-p|-px|-F|-y|+y|-h]\n"
	     "x:\tDrive name\n"
	     "-v|+v\tVerbose on/off\n"
	     "-llabel\tSet disk label\n"
	     "-sx\tForce size to x MiB\n"
	     "-cx\tForce cluster size to x 512-byte sectors\n"
	     "-fx\tSet number of FATs (1 or 2)\n"
	     "-ix\tSet 32-bit serial number volume ID to x\n"
	     "-p|+p\tMake/don't make partition table\n"
	     "-px\tReserve x MiB for partition table\n"
	     "-F|+F\tForce / don't force making file system even if illegal\n"
	     "-y|+y\tDon't ask / Ask for confirmation\n"
	     "-n|+n\tDry run (don't actually write) on/off\n"
	     "-a|+a\tErase (very slow) / Don't erase all data\n"
	     "-h\tShow this help\n");
      goto finally;
    } else if (!strcmp(p, "-v")) {
      verbose = 1;
    } else if (!strcmp(p, "+v")) {
      verbose = 0;
    } else if (!strncmp(p, "-l", 2)) {
      char *pp = p+2;
      label = pp;
      if (strlen(pp) > 11 || !*pp) {
	printf("E: Volume label \"%s\" too short or long\n", pp);
	goto finally;
      }
      while (*pp) {
	char c = toupper(*pp);
	if (!((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_')) {
	  printf("E: Volume label \"%s\" contained other characters than "
		 "A-Z, 0-9, _\n", p+2);
	  goto finally;
	}
	pp++;
      }
    } else if (!strncmp(p, "-s", 2)) {
      if ((forceSizeMiB = strtol(p+2, NULL, 0)) < 1) {
	printf("E: Illegal disk size in parameter \"%s\"\n", p);
	goto finally;
      }
    } else if (!strncmp(p, "-c", 2)) {
      if ((forceClusterSize = strtol(p+2, NULL, 0)) < 1 ||
	  (1<<QsortLog2(forceClusterSize)) != (forceClusterSize<<1) ||
	  forceClusterSize > 64) {
	printf("E: Illegal cluster size in parameter \"%s\"\n", p);
	goto finally;
      }
    } else if (!strncmp(p, "-f", 2)) {
      if ((numberOfFats = strtol(p+2, NULL, 0)) < 1 || numberOfFats > 2) {
	printf("E: Illegal number of FATs in parameter \"%s\"\n", p);
	goto finally;
      }
    } else if (!strncmp(p, "-p", 2)) {
      partitionSectors = strtol(p+2, NULL, 0)*2048;
      makePartitionTable = 1;
    } else if (!strncmp(p, "+p", 2)) {
      partitionSectors = 0;
      makePartitionTable = 0;
    } else if (!strncmp(p, "-i", 2)) {
      serialNumber = strtol(p+2, NULL, 0);
    } else if (!strcmp(p, "-F")) {
      justForce = 1;
    } else if (!strcmp(p, "+F")) {
      justForce = 0;
    } else if (!strcmp(p, "-y")) {
      confirmation = 0;
    } else if (!strcmp(p, "+y")) {
      confirmation = 1;
    } else if (!strcmp(p, "-n")) {
      dryRun = 1;
    } else if (!strcmp(p, "+n")) {
      dryRun = 0;
    } else if (!strcmp(p, "-a")) {
      eraseAll = 1;
    } else if (!strcmp(p, "+a")) {
      eraseAll = 0;
    } else {
      int newDriveLetter = toupper(*p);
      if (strlen(p) != 2 || p[1] != ':') {
	printf("E: Unknown parameter \"%s\"\n", p);
	goto finally;
      }
      if (newDriveLetter < 'A' || newDriveLetter > 'Z') {
	printf("E: Illegal drive letter \"%s\"\n", p);
	goto finally;
      }
      if (!VODEV(newDriveLetter)) {
	printf("E: Non-existing drive \"%s\"\n", p);
	goto finally;
      }
      if (driveLetter) {
	printf("E: Redefined drive name \"%c\": with \"%s\"\n", driveLetter, p);
	goto finally;
      }
      driveLetter = newDriveLetter;
    }
    p += strlen(p)+1;
  }

  if (!driveLetter) {
    printf("E: No drive name\n");
    goto finally;
  }

  dev = VODEV(driveLetter);
  if (!dev) {
    printf("E: Can't open %c:\n", driveLetter);
    goto finally;
  }

  /* Make sure we'll get the information of the disk currently in the drive. */
  ioctl(dev, IOCTL_RESTART, NULL);
  dev->BlockRead(dev, 0, 1, myBuf); /* Read -> force clocks. */
  ioctl(dev, IOCTL_GET_GEOMETRY, (void *)(&geometry));

  if (forceSizeMiB) {
    geometry.totalSectors = forceSizeMiB * 2048;
  }

  while (geometry.totalSectors < defaultPartitionSectors*900) {
    defaultPartitionSectors /= 2;
#if 0
    printf("<< partSect -> %ld\n", labs(partitionSectors));
#endif
  }
  if (partitionSectors == 0) {
    partitionSectors = defaultPartitionSectors;
  }
  if (!makePartitionTable) {
    partitionSectors = 0;
  }

  while (geometry.totalSectors < alignFat*900) {
    alignFat /= 2;
  }

#if 0
  printf("totalSect %ld/1024=%ld, partSecr %ld\n",
	 geometry.totalSectors, geometry.totalSectors/1024, partitionSectors);
#endif

  if (verbose) {
    printf("Raw disk geometry:  %ld sectors (%ld MiB = %4.1f GiB), "
	   "%d sect per block\n",
	   geometry.totalSectors,
	   geometry.totalSectors/(2*1024),
	   geometry.totalSectors*(1.0/(2.0*1024.0*1024.0)),
	   geometry.sectorsPerBlock);
  }
  if (geometry.sectorsPerBlock != 1) {
    printf("E: Bad disk geometry (must be 1 sector per block)\n");
    goto finally;
  }

  {
    u_int32 sectors = geometry.totalSectors-partitionSectors;
    u_int16 sectorsPerCluster = forceClusterSize ? forceClusterSize : 8;
    u_int32 sectorsPerFat = 0;
    u_int32 clusters = 0;
    u_int32 requiredSectors = 0;
    u_int32 startOfFileSystem = 0;
    u_int32 i32;

    while (!forceClusterSize && sectorsPerCluster < 64 &&
	   sectors/(1024L*1024L) >= (u_int32)(sectorsPerCluster)) {
      sectorsPerCluster *= 2;
    }
#if 0
    printf("Sectors per cluster %d\n", sectorsPerCluster);
#endif
    clusters = (sectors-reservedSectors)/sectorsPerCluster;
    /* NOTE: 65525 is CORRECT, according to Microsoft document
       "FAT: General Overview of On-Disk Format",
       Chapter "FAT Type Determination". */
    while (!forceClusterSize && clusters < 65525 && sectorsPerCluster > 1) {
      sectorsPerCluster /= 2;
      clusters *= 2;
    }

    do {
      clusters--;
      sectorsPerFat = (clusters+2+127)/128;
      /* "+7" is there because FAT32 needs res.sectors 0...6 available */
      reservedSectors = ((numberOfFats*sectorsPerFat+7+(alignFat-1)) &
			 ~(alignFat-1)) - numberOfFats*sectorsPerFat;
      startOfFileSystem = reservedSectors + numberOfFats*sectorsPerFat;
      requiredSectors = startOfFileSystem + clusters*sectorsPerCluster;
    } while (requiredSectors > sectors);

    if (verbose) {
      printf("  sectors %ld, sectorsPerCluster %d, clusters %ld\n"
	     "  numberOfFats %d, sectorsPerFat %ld, reservedSectors %ld\n"
	     "  startOfFat %ld, startOfFileSystem startOfFat+%ld\n"
	     "  serialID 0x%08lx\n",
	     requiredSectors, sectorsPerCluster, clusters,
	     numberOfFats, sectorsPerFat, reservedSectors,
	     partitionSectors, startOfFileSystem,
	     serialNumber);
    }

    if (clusters < 65525) {
      printf("%c: Disk too small for FAT32 (%ld clusters < %ld minimum)\n",
	     justForce ? 'W' : 'E', clusters, 65525);
      if (!justForce) goto finally;
    }

    /* Here we lose up to sectorsPerCluster-1 sectors,
       which is practically nothing. */
    sectors = requiredSectors;

    /* Partition table */
    SetCHS( fatRoot[5], 0x1be+0x1, partitionSectors);
    SetCHS( fatRoot[5], 0x1be+0x5, partitionSectors-1+sectors);
    SetLE32(fatRoot[5], 0x1be+0x8, partitionSectors);
    SetLE32(fatRoot[5], 0x1be+0xC, sectors);

    /* Root sector */
    SetBE8( fatRoot[0], 0x0d, sectorsPerCluster);
    SetLE16(fatRoot[0], 0x0e, reservedSectors);
    SetBE8( fatRoot[0], 0x10, numberOfFats);
    SetLE32(fatRoot[0], 0x20, sectors);
    SetLE32(fatRoot[0], 0x24, sectorsPerFat);
    SetLE32(fatRoot[0], 0x43, serialNumber);
    if (label) {
      u_int16 op0 = 0x47, op3 = 0;
      char *lp = label;
      while (*lp) {
	char ch = toupper(*lp);
	lp++;
	SetBE8(fatRoot[0], op0++, ch);
	SetBE8(fatRoot[3], op3++, ch);
      }
    }

    if (tt != TIME_NOT_FOUND && tt != TIME_NOT_SET) {
      struct tm *tm = localtime(&tt);
      u_int16 dat = ((tm->tm_year-80)<<9) | ((tm->tm_mon+1)<<5) | tm->tm_mday;
      u_int16 tim = (tm->tm_hour<<11) | (tm->tm_min<<5) | (tm->tm_sec/2);
      SetLE16(fatRoot[3], 0x18, dat); /* DIR_WrtDate    */
      SetLE16(fatRoot[3], 0x10, dat); /* DIR_CrtDate    */
      SetLE16(fatRoot[3], 0x12, dat); /* DIR_LstAccDate */
      SetLE16(fatRoot[3], 0x16, tim); /* DIR_WrtTime    */
      SetLE16(fatRoot[3], 0x0e, tim); /* DIR_CrtTime    */
    }

    if (confirmation) {
      int ch;
      printf("Are you sure you want to format drive %c:? All data will be lost!"
	     " (Y/N)\n", driveLetter);
      do {
	ch = fgetc(stdin);
	ch = toupper(ch);
	if (ch == 'N') {
	  printf("E: Aborted\n");
	  goto finally;
	}
      } while (ch != 'Y');
    }

    if (partitionSectors) {
      if (verbose) {
	printf("Writing partition table (%1.1f MiB of data)...\n",
	       partitionSectors/(1024*2.0));
      }
      if (!dryRun && dev->BlockWrite(dev, 0, 1, fatRoot[5])) {
	printf("E: Cannot write to device\n");
	goto finally;
      }
      for (i32=1; i32<partitionSectors; i32++) {
	if (!dryRun && dev->BlockWrite(dev, i32, 1, fatRoot[4])) {
	  printf("E: Cannot write to device\n");
	  goto finally;
	}
      }
    }

    if (verbose) {
      printf("Writing FAT32 information (%1.1f MiB of data)...\n",
	     startOfFileSystem/(1024*2.0));
    }
    Delay(100); /* Wait for UART buffers to empty. */

    for (i32=0; i32<startOfFileSystem; i32++) {
      int idx = 4; /* Clear everything up to file system. */
      if (!i32 || i32 == 6) {
	idx = 0; /* FAT Root & FAT Root copy */
      } else if (i32 == 1) {
	idx = 1; /* File System Information */
      } else if (i32 == reservedSectors) {
	idx = 2; /* Free cluster table block 0 */
      } else if (i32 == reservedSectors+sectorsPerFat && numberOfFats > 1) {
	idx = 2; /* Copy of free cluster table block 0 */
      }
      if (!dryRun &&
	  dev->BlockWrite(dev, partitionSectors+i32, 1, fatRoot[idx])) {
	printf("E: Cannot write to device\n");
	goto finally;
      }
    }
    /* FAT Root Dir. */
    for (i=0; i<sectorsPerCluster; i++) {
      if (!dryRun && dev->BlockWrite(dev, partitionSectors+startOfFileSystem+i,
				     1, fatRoot[i ? 4 : 3])) {
	printf("E: Cannot write to device\n");
	goto finally;
      }
    }

    if (eraseAll) {
      u_int32 startTime = ReadTimeCount(), nextReport = startTime+TICKS_PER_SEC;
      if (verbose) {
	printf("Erasing data area... (Press Ctrl-C to abort)\n"
	       "0 s, 0%%, 0.0 MiB/s ");
      }
      for (i32=partitionSectors+startOfFileSystem+sectorsPerCluster;
	   i32<geometry.totalSectors; i32++) {
	u_int32 now = ReadTimeCount();
	if (!dryRun && dev->BlockWrite(dev, i32, 1, fatRoot[4])) {
	  printf("\nW: Cannot write to device.\n");
	  goto finally;
	}
	if (verbose && now >= nextReport) {
	  u_int32 elapsed = now - startTime;
	  nextReport += TICKS_PER_SEC;
	  printf("\r%ld s, %ld%%, %2.1f MiB/s ", elapsed/TICKS_PER_SEC,
		 100*i32/geometry.totalSectors,
		 (i32-startOfFileSystem-partitionSectors-sectorsPerCluster)*
		 (512.0/1024.0/1024.0*1000.0)/elapsed);
	}
	if (appFlags & APP_FLAG_QUIT) {
	  printf("\nW: Aborted clearing data area. File system is still OK.\n");
	  i32 = geometry.totalSectors;
	}
      }
      if (verbose) printf("\r");
    }

    i32 = clusters*sectorsPerCluster;
    if (verbose) {
      printf("Formatted capacity: %ld sectors (%ld MiB = %4.1f GiB)\n",
	     i32,
	     i32/(2*1024),
	     i32*(1.0/(2.0*1024.0*1024.0)));
    }

    dev->BlockRead(dev, 0, 1, myBuf); /* Read -> force write op to finish. */
    ioctl(dev, IOCTL_RESTART, NULL);  /* Restart to get new geometry to VSOS. */
  }

  retCode = S_OK;
 finally:

  return retCode;
}
