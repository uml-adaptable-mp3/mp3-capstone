/// \file vo_fat.c FAT16/FAT32 Filesystem driver for VsOS
/// \author Panu-Kristian Poiksalo and Pasi Ojala, VLSI Solution Oy



#include <vo_stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "vsos.h"
#include "vo_fat.h"
//#include "forbid_stdout.h"
#include "strings.h"
#include <sysmemory.h>

/*
  Some functions from vo_fat.c with only FAT32 support.
 */

ioresult FatImageSectorRead(register __i0 VO_FILE *f, u_int32 sector) {
	if ((sector != f->currentSector) && (__F_DIRTY(f))) {
		f->flags &= ~(__MASK_DIRTY);
		f->dev->BlockWrite(f->dev, f->currentSector, 1, f->sectorBuffer);
	}
	if (sector != f->currentSector) {
		f->currentSector = sector;
		return f->dev->BlockRead(f->dev, sector, 1, f->sectorBuffer);
	}
	return S_OK;
}

u_int16 MegaFatGetByte(register u_int16 *buf, register u_int16 n) {
	//register u_int16 *p = &buf[n>>1];
	register u_int16 *p = buf + (n>>1);
	if (n & 1) {
		n = *p;
	} else {
		n = *p >> 8;
	}
	return n & 0xff;
}
u_int16 MegaFatGetWord(register u_int16 *buf, register u_int16 n) {
	return MegaFatGetByte(buf, n) | (MegaFatGetByte(buf, n+1)<<8);
}
u_int32 MegaFatGetLong(register u_int16 *buf, register u_int16 n) {
	return MegaFatGetWord(buf, n) | ((u_int32)MegaFatGetWord(buf, n+2)<<16);
}



//Calculate where a FAT entry is located
auto void VoFatClusterPosF32(register __i0 VO_FILE *f, register u_int32 fatCluster, u_int32 *sector, u_int16 *offset) {
	FatDeviceInfo *di=(FatDeviceInfo*)f->dev->deviceInfo;
	if (fatCluster >= di->totalClusters) {
		//Is this needed? Recovery code?
		*sector = 0;		
		*offset = 0;
		return; 		
	}
	*offset = ((u_int16)fatCluster & 0x7F) * 4; /* In bytes */
	*sector = (fatCluster >> 7) + di->fatStart;
	
}

u_int32 VoFatReadClusterRecordF32(register __i0 VO_FILE *f, u_int32 fatCluster) {
	FatFileInfo *fi=(FatFileInfo*)f->fileInfo;
	FatDeviceInfo *di=(FatDeviceInfo*)f->dev->deviceInfo;
	u_int32 sector;
	u_int16 offset;
	u_int32 t;
	u_int16 part2 = 0;

	VoFatClusterPosF32(f, fatCluster, &sector, &offset);
	if (!sector) return 0xffffffffU;

	if (sector != f->currentSector) {
		FatImageSectorRead(f, sector);
	}
	
	t = MegaFatGetLong(f->sectorBuffer,offset) & 0x0fffffffUL;
	
	return t;			
}



#if 0
#if 1
#define OPTIMIZE_SPEED
#endif

#define MIN(a,b) (((a)<(b))?(a):(b))

u_int32 VoFatCountFreeClusters(register VO_FILE *f, register u_int32 maxMiB) {
  //  FatFileInfo *fi=(FatFileInfo*)f->fileInfo;
  FatDeviceInfo *di=(FatDeviceInfo*)f->dev->deviceInfo;
  u_int32 res = 0, k = 1;
  u_int32 end = di->totalClusters + 2;
  u_int32 maxClusters = (maxMiB == 0xFFFFFFFFU) ?
    0xFFFFFFFFU : maxMiB*(1024*2)/di->fatSectorsPerCluster;

  for (k=2; k<end; k++) {
    if (!VoFatReadClusterRecordF32(f,k)) {
      res++;
    }
#ifdef OPTIMIZE_SPEED
    if (di->fatBits == 32 && !(k & 127) && di->totalClusters-k >= 128) {
      u_int32 *sB = (u_int32 *)(&f->sectorBuffer[2]);
      int i;
      for (i=1; i<128; i++) {
	if (!(*sB++)) {
	  res++;
	}
      }
      k += 127;
    }
#endif

    if (res >= maxClusters) {
      return maxMiB*1024;
    }
  }
  return res * di->fatSectorsPerCluster / 2;
}
#endif