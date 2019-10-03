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
#include <exec.h>
#include <swap.h>

#define FAT_LFN_SIZE 150
char longFileName[FAT_LFN_SIZE];
u_int32 lfnStartCluster; //of which file is this the long name of


//extern u_int16 vo_fat_allocationSizeClusters = 1; /// How many clusters are allocated at a time for writing to files


// This function writes bytes into a fat sector. If the sector is not the current sector, the current sector is
// written out first and the sector is read from the disk. 
ioresult FatImageWriteBytes(register __i0 VO_FILE *f, u_int32 sector, u_int16 byteOffset, u_int16 *data, u_int16 sourceIndex, u_int16 bytes) {
	FatDeviceInfo *di=(FatDeviceInfo*)f->dev->deviceInfo;			
	sector += (byteOffset / 512);
	byteOffset &= 511;	
	
#if 0
	printf("WriteBytes(sec=%ld, byteOff=%d, sIdx=%d, byt=%d), pos=%ld\n", sector, byteOffset, sourceIndex, bytes, f->pos);
#endif
	
	if ((sector != f->currentSector) && (__F_DIRTY(f))) {
		f->flags &= ~(__MASK_DIRTY);
		f->dev->BlockWrite(f->dev, f->currentSector, 1, f->sectorBuffer);
	}
	if (bytes == 0) {
		return S_OK; //flush only (0 words)
	}
	
	if (sector != f->currentSector) {
#if 0
		printf("#2\n");
#endif
		f->currentSector = sector;	
		if (byteOffset || (bytes!=512)) {	
			if (f->pos < f->fileSize || (byteOffset && !__F_DIRTY(f)) || sector <= di->dataStart) {
				f->dev->BlockRead(f->dev, sector, 1, f->sectorBuffer);
			} else {
				memset(f->sectorBuffer,0,512/2);
			}
		}
	}
	
	if ((byteOffset+bytes) > 512) {
		return SysError("Sector Overwrite");
	}
	MemCopyPackedBigEndian(f->sectorBuffer, byteOffset, data, sourceIndex, bytes);
	f->flags |= __MASK_DIRTY;
	return S_OK;
}

ioresult FatFlush(register __i0 VO_FILE *f) {
	if (__F_DIRTY(f)) {
		f->flags &= ~(__MASK_DIRTY);
		f->dev->BlockWrite(f->dev, f->currentSector, 1, f->sectorBuffer);
	}
	f->dev->BlockRead(f->dev, 0, 0, f->sectorBuffer);
}

ioresult FatImageSectorRead(register __i0 VO_FILE *f, u_int32 sector) {
	if ((sector != f->currentSector) && (__F_DIRTY(f))) {
		f->flags &= ~(__MASK_DIRTY);
		f->dev->BlockWrite(f->dev, f->currentSector, 1, f->sectorBuffer);
	}
	if (sector != f->currentSector) {
		f->currentSector = sector;
		//Disable();
		//fprintf(vo_stderr,"\n [%ld] f%p @%ld BR(%ld)->%p\n",f->pos-((sector-1696)*512L),f,f->pos,sector,f->sectorBuffer);
		//Enable();
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

#define __4__ + 0x0400
#define __2__ + 0x0200
#define __1__ + 0x0100

#define __BPB_BytsPerSec (MegaFatGetWord(fatBuffer, 11))
#define __BPB_SecPerClus (MegaFatGetByte(fatBuffer, 13))
#define __BPB_RsvdSecCnt (MegaFatGetWord(fatBuffer, 14))
#define __BPB_NumFats (MegaFatGetByte(fatBuffer, 16))
#define __BPB_RootEntCnt (MegaFatGetWord(fatBuffer, 17 ))
#define __BPB_TotSec16 (MegaFatGetWord(fatBuffer, 19))
#define __BPB_FatSz16 (MegaFatGetWord(fatBuffer, 22))
#define __BPB_SecPerTrk (MegaFatGetWord(fatBuffer, 24))
#define __BPB_TotSec32 (MegaFatGetLong(fatBuffer, 32))
#define __BPB_VolIdFat16 (MegaFatGetLong(fatBuffer, 39))
#define __BPB_FatSz32 (MegaFatGetWord(fatBuffer, 36))
#define __BPB_RootClus (MegaFatGetWord(fatBuffer, 44))

#define __CHECK_NOT_FAT ((fatBuffer[27] != ('F'<<8)+('A'<<0)) && (fatBuffer[41] != ('F'<<8)+('A'<<0)))


ioresult FatCreate(register __i0 DEVICE *dev, const char *name, u_int16 *fatBuffer){
	FatDeviceInfo *di=(FatDeviceInfo*)dev->deviceInfo;			
	u_int32 bpbSector = 0;	
	lfnStartCluster = 0; //unvalidate the long file name	
	if (!__F_CAN_SEEK_AND_READ(dev)) {
		return -10; //FAT needs a device which can do seek and read operations
	}	
	di->allocationStartCluster = 2; //where to start looking for free space
	dev->BlockRead(dev, 0, 1, fatBuffer);
	if (fatBuffer[255] != 0x55aa) {
		return -11; //Sector 0 is not MBR or BPB.
	}
	// Try to look for a BPB sector FAT signature (can be "FAT", "FAT12", "FAT16" or "FAT32")
	if (__CHECK_NOT_FAT)	{
		//The sector is not a BPB record, it might be a partition table.
		//Try to use the sector as a partition table and see if a FAT BPB record is found by dereferencing it.
		u_int16 partitionStartPtr = 227+(*name-'0')*16; //name "0" for first partition, name "1" for second partition etc
		bpbSector = ((u_int32)BYTESWAP(fatBuffer[partitionStartPtr+1])<<16) | BYTESWAP(fatBuffer[partitionStartPtr]); //endianness probably wrong
		dev->BlockRead(dev, bpbSector, 1/*sectors*/,/*to*/fatBuffer);		
	}
	if (__CHECK_NOT_FAT) {
		return -12; //Still cannot find a BPB record, give up. Seems it's not a FAT disk.
	}
	// Ok, finally we have something that looks like a FAT signature at the proper offset of the sector;
	// so now we Know that we have a valid BPB record (Bios Parameter Block) at sector bpbSector.	
	{
		u_int16 blocksPerSector = __BPB_BytsPerSec / 512;
		u_int32 rootDirSectors;
		u_int32 fatSize = __BPB_FatSz16 * blocksPerSector;
		u_int32 totalSectors = __BPB_TotSec16 * blocksPerSector;
		u_int16 bytesPerSector = __BPB_BytsPerSec;

		di->rootEntCnt = __BPB_RootEntCnt;
		di->numFats = __BPB_NumFats;
		
		rootDirSectors = ((di->rootEntCnt * 32) + (bytesPerSector - 1 )) / bytesPerSector;

		if (fatSize == 0) {
			fatSize = __BPB_FatSz32*blocksPerSector;
		}
		if (totalSectors == 0) {
			totalSectors = __BPB_TotSec32*blocksPerSector;
		}		
		di->fatSectorsPerCluster = (u_int16)(__BPB_SecPerClus * (bytesPerSector / 512));				
		{
			u_int32 dataSectors = totalSectors - (__BPB_RsvdSecCnt + (di->numFats * fatSize) + rootDirSectors);
			di->totalClusters = dataSectors / di->fatSectorsPerCluster + 2;	//Compensate for first cluster which is cluster #2
		}
		{
			u_int16 realFatBits = 32;
			if (di->totalClusters < 4085+2) { // totalClusters includes non-existent clusters 0 and 1
				realFatBits = 12;
			} else if (di->totalClusters < 65525+2) { // totalClusters includes non-existent clusters 0 and 1
				realFatBits = 16;
			}
			if (di->rootEntCnt == 0) {
				realFatBits = 32; // Force FAT32 because rootDirSectors is Zero
			}
			di->fatBits = realFatBits;
		}
		//				printf("## REALFATBITS %d\n", di->fatBits);
		di->fatStart = bpbSector + (u_int32)__BPB_RsvdSecCnt * (bytesPerSector / 512);
		di->rootStart = (fatSize * __BPB_NumFats) + di->fatStart;
		di->dataStart = (di->rootEntCnt / 16) + di->rootStart;
		di->dataStart -= (u_int32)di->fatSectorsPerCluster*2;	
		// for FAT32, set rootStart to be the root cluster number
		if (di->fatBits == 32) {
			di->rootStart = 2; /// \todo:get real value from BPB
		}
	}
	return S_OK;
}

u_int32 VoFatReadClusterRecord(register __i0 VO_FILE *f, u_int32 fatCluster);

auto Fragment FatGetFragmentFromFAT(register __i0 VO_FILE *f, u_int32 fatCluster, u_int32 filePos) {
 	FatDeviceInfo *di=(FatDeviceInfo*)f->dev->deviceInfo;
	FatFileInfo *fi=(FatFileInfo*)f->fileInfo;
	Fragment frag;
	//	u_int16 offset;
#if 0
	printf("\nFatGetFragment cluster:%lx, fileblock:%lx\n",fatCluster, filePos/512);
#endif

	frag.startByteOffset = 0;
	frag.sizeBytes = 0;
#if 0
	printf("##0: %ld\n", frag.sizeBytes);
#endif
	/*TODO: return empty cluster list if fatCluster < 2*/
	/* fatCluster means now current cluster, starting from 0, not 2. */
	/* note that startcluster=2 is already compensated in fatInitGlobals */
	#pragma msg 36 off
	goto l55; /* read first sector */	
	while (1) {
		u_int32 t;
		t = VoFatReadClusterRecord(f, fatCluster);
		frag.sizeBytes += (di->fatSectorsPerCluster*512);
#if 0
		printf("##1: %ld, secPerClu %d\n", frag.sizeBytes, di->fatSectorsPerCluster);
#endif
		if (t >= 0x0ffffff8UL) {
			/*last fragment finished, force filesize down to frag size*/
			if (f->fileSize > f->pos+frag.sizeBytes) {
#if 0
			  printf("Adjust file size, was:%ld, new:%ld+%ld bytes.\n",f->fileSize, f->pos, frag.sizeBytes);
#endif
				f->fileSize = f->pos+frag.sizeBytes;
			}							
			return frag;
		}
		++fatCluster;
		if (t != fatCluster) {
			/* if not sequential cluster, start next fragment */
			fatCluster = t;
			if (frag.startByteOffset <= filePos && filePos < frag.startByteOffset + frag.sizeBytes) {
			   return frag;
			}
	l55:
#if 0
			printf("##2: %lx %lx\n", frag.startByteOffset, frag.sizeBytes);
#endif
			frag.startByteOffset += frag.sizeBytes;
			frag.sizeBytes = 0;
			frag.startSector = fatCluster * di->fatSectorsPerCluster + di->dataStart;
		}
	}
}



//Calculate where a FAT entry is located
auto void VoFatClusterPos(register __i0 VO_FILE *f, register u_int32 fatCluster, u_int32 *sector, u_int16 *offset) {
	FatDeviceInfo *di=(FatDeviceInfo*)f->dev->deviceInfo;
	if (fatCluster >= di->totalClusters) { // totalClusters includes non-existent clusters 0 and 1
		//Is this needed? Recovery code?
		*sector = 0;		
		*offset = 0;
		return; 		
	}
	switch (di->fatBits) {	
		case 32:
		  *offset = ((u_int16)fatCluster & 0x7F) * 4; /* In bytes */
		  *sector = (fatCluster >> 7) + di->fatStart;
		  break;
		
		case 16:
		  *offset = ((u_int16)fatCluster & 0xff) * 2; /* In bytes */
		  *sector = (fatCluster >> 8) + di->fatStart;
		  break;
		
		default:
		  {
		    u_int32 t = fatCluster*3;
		    *offset = (u_int16)t & 0x3ff; /* In nibbles */
		    *sector = (t >> 10) + di->fatStart;
		  }
		  break;
	}
}

u_int32 VoFatReadClusterRecord(register __i0 VO_FILE *f, u_int32 fatCluster) {
	FatFileInfo *fi=(FatFileInfo*)f->fileInfo;
	FatDeviceInfo *di=(FatDeviceInfo*)f->dev->deviceInfo;
	u_int32 sector;
	u_int16 offset;
	u_int32 t;
	u_int16 part2 = 0;

#if 0
	if (fatCluster > 0x1ddee8)
	printf("VoFatReadClusterRecord(f, fatCluster=%ld)\n", fatCluster);
#endif

	VoFatClusterPos(f, fatCluster, &sector, &offset);
	if (!sector) return 0xffffffffU;

#if 0
	printf("Entering \"again\", fatBits = %d\n", di->fatBits);
#endif
 again:
	if (sector != f->currentSector) {
		FatImageSectorRead(f, sector);
	}

	switch (di->fatBits) {	
		case 32:
			t = MegaFatGetLong(f->sectorBuffer,offset) & 0x0fffffffUL;
			break;
		
		case 16:
			t = MegaFatGetWord(f->sectorBuffer,offset);	
			if (t >= 0xfff8U)
			  t |= 0x0fff0000UL; //POj:20090114
			break;

		default:
#if 0
		  printf("cluster %ld, sector %ld, offset %d %04x %04x %04x %04x\n",
			 fatCluster, sector, offset,
			 f->sectorBuffer[0],
			 f->sectorBuffer[1],
			 f->sectorBuffer[2],
			 f->sectorBuffer[3]);
#endif
		  if (part2) {
		    t |= MegaFatGetByte(f->sectorBuffer, offset>>1)<<8;
		  } else if (offset < 1022) {
		    t = MegaFatGetWord(f->sectorBuffer, offset>>1);
		  } else {
		    t = MegaFatGetByte(f->sectorBuffer, offset>>1);
		    offset -= 1022;
		    sector++;
		    part2 = 1;
		    goto again;
		  }
#if 0
		  printf("t %04lx\n", t);
#endif
		  if (offset & 1) {
		    t >>= 4;
		  } else {
		    t &= 0x0FFF;
		  }
#if 0
		  printf("##55 %08lx\n", t);
#endif
		  if (t >= 0xff8) {
		    t |= 0x0ffff000UL;
		  }
		  break;
	}
	return t;			
}


ioresult VoFatWriteClusterRecord(register __i0 VO_FILE *f, u_int32 fatCluster, u_int32 newValue) {
	FatFileInfo *fi=(FatFileInfo*)f->fileInfo;
	FatDeviceInfo *di=(FatDeviceInfo*)f->dev->deviceInfo;
	u_int32 sector;
	u_int16 offset;
	u_int16 v[2];		

	if (fatCluster < 2) return S_ERROR;		
	//printf("{Cluster%ld=>%ld}",fatCluster,newValue);
#if 0
	printf("#5 %ld %ld\n", fatCluster, newValue);
#endif
	VoFatClusterPos(f, fatCluster, &sector, &offset);

	v[0] = BYTESWAP((u_int16)newValue);
	v[1] = BYTESWAP((u_int16)(newValue >> 16));

	switch (di->fatBits) {	
		case 32:
		  FatImageWriteBytes(f,sector,offset,v,0,4);
		  break;

		case 16:
		  FatImageWriteBytes(f,sector,offset,v,0,2);
		  break;

		default: /* FAT12 */
		  {
		    u_int16 newVal16 = newValue;
#if 0
		    printf("Make entry 0x%04x to sector %ld offset %d\n", newVal16, sector, offset);
#endif
		    FatImageSectorRead(f, sector);
#if 0
		    printf("%04x %04x %04x %04x\n",
			   f->sectorBuffer[0],
			   f->sectorBuffer[1],
			   f->sectorBuffer[2],
			   f->sectorBuffer[3]);
#endif
		    if (offset & 1) {
		      v[0] = MegaFatGetByte(f->sectorBuffer, offset>>1);
		      v[0] = (v[0] & 0xf) | ((newVal16 & 0xf) << 4);
		    } else {
		      v[0] = newVal16;
		    }
#if 0
		    printf("v %04x\n", v[0]);
#endif
		    FatImageWriteBytes(f,sector,offset>>1,v,1,1);
		    if (offset >= 1022) {
		      sector++;
		      offset -= 1022;
		      FatImageSectorRead(f, sector);
		    } else {
		      offset += 2;
		    }
		    if (offset & 1) {
		      v[0] = newVal16>>4;
		    } else {
		      v[0] = MegaFatGetByte(f->sectorBuffer, offset>>1);
		      v[0] = (v[0] & 0xf0) | ((newVal16 & 0xf00) >> 8);
		    }
#if 0
		    printf("v %04x, s %ld, of %d\n", v[0], sector, offset);
#endif
		    FatImageWriteBytes(f,sector,offset>>1,v,1,1);
#if 0
		    printf("%04x %04x %04x %04x\n",
			   f->sectorBuffer[0],
			   f->sectorBuffer[1],
			   f->sectorBuffer[2],
			   f->sectorBuffer[3]);
#endif
		  }
		  break;
	}
	return S_OK;
}



/// GetFreeFragment. SPECIAL: Returns start CLUSTER, 0 if unsuccessful.
u_int32 VoFatGetFreeFragment(register __i0 VO_FILE *f,u_int32 requestedClusters) {
	FatFileInfo *fi=(FatFileInfo*)f->fileInfo;
	FatDeviceInfo *di=(FatDeviceInfo*)f->dev->deviceInfo;
	u_int32 start = 0;
	u_int32 size = 0;
	u_int32 k = di->allocationStartCluster-1;
	u_int32 r;
#if 0
	printf("VoFatGetFreeFragment: requestedClusters %ld, di->totalClusters %ld\n", requestedClusters, di->totalClusters);
#endif

	while (k < di->totalClusters) { // totalClusters includes non-existent clusters 0 and 1
		k++;
		r = VoFatReadClusterRecord(f,k);
		if (r == 0) { /*k is Unallocated*/
			if (start == 0) {
				start = k;
				size = 0;
#if 0
				printf("Free area start at cluster %ld.\n",start);
#endif
			}
			size++;
			if (size >= requestedClusters) {
				return_fragment:
#if 0
			  printf("Free area size: %ld clusters (%ld sectors).\n",size,size*di->fatSectorsPerCluster);
#endif
				di->allocationStartCluster = start+size; /// \todo is this correct? Does it leave chunks of unallocated space?
				fi->currentFragment.startByteOffset += fi->currentFragment.sizeBytes; /// \todo does this work if you seek?
				fi->currentFragment.startSector = start * di->fatSectorsPerCluster + di->dataStart; //VoFatClusterToSector(dirfile, startCluster);
				fi->currentFragment.sizeBytes = size * di->fatSectorsPerCluster * 512;
#if 0
	printf("##4: %ld\n", fi->currentFragment.sizeBytes);
#endif

#if 0
	printf("Allocated new fragment: %ld (%ld bytes)\n",start,fi->currentFragment.sizeBytes);
#endif
				return start;
			}
		} else { /* k is Allocated */
			if (start) { // At least one free cluster was found before running into allocated space.
				goto return_fragment;
			}
		}
	}
	if (start) { // At least one free cluster was found before running into end of disk.
		goto return_fragment;
	}				
	SysError ("Disk Full");
	return 0;
}


u_int32 WriteClusterChain(register __i0 VO_FILE *f, Fragment *frag, s_int16 allocateMore) {
	FatDeviceInfo *di=(FatDeviceInfo*)f->dev->deviceInfo;
	u_int32 startCluster = (frag->startSector - di->dataStart) / di->fatSectorsPerCluster;
	u_int32 sizeClusters = (frag->sizeBytes + (u_int32)di->fatSectorsPerCluster * 512 - 1) / ((u_int32)di->fatSectorsPerCluster * 512);
	u_int32 nextInChain = 0x0fffffffu;
#if 0
	printf("p 0x%lx\n", f->pos);
	printf("startSec %lx StartCl %lx\n", frag->startSector, startCluster);
	printf("sizeByts %lx SizeCl  %lx\n", frag->sizeBytes, sizeClusters);
#endif

	if (allocateMore) {
	  nextInChain = VoFatGetFreeFragment(f, (allocateMore < 0) ? 2 : vo_fat_allocationSizeClusters);
	  if (nextInChain == 0) {
	    nextInChain = 0x0fffffffu;
	    f->flags &= ~__MASK_WRITABLE; //Disk full, remove file writability
	  }
	}
		
	while (--sizeClusters) {	
		VoFatWriteClusterRecord(f, startCluster, startCluster+1);
		if (!(startCluster & 127) && sizeClusters >= 128 && di->fatBits == 32) {
			/* This speed optimization makes writing long cluster lists
			   significantly faster. (2018-01-31)
			   Example: writing one million clusters for 17 files
			   on a 256 GB Transcend card used to take 89 seconds, but
			   with this code it now takes only 4 seconds. */
			while (sizeClusters >= 128) {
				u_int32 *p = (u_int32 *)(f->sectorBuffer);
				int i;
				for (i=0; i<128; i++) {
					*p++ = Swap32Mix(++startCluster);
				}
				f->dev->BlockWrite(f->dev, f->currentSector++, 1, f->sectorBuffer);
				sizeClusters -= 128;
			}
			sizeClusters++;
			f->flags &= ~__MASK_DIRTY;
			f->currentSector = 0xFFFFFFFFU;
		} else {
			startCluster++;
		}
	}
#if 0
	printf("st %lx nx %lx\n", startCluster, nextInChain);
#endif
	VoFatWriteClusterRecord(f, startCluster, nextInChain);
#if 0
	printf("#1\n");
#endif
	FatFlush(f);	
#if 0
	printf("End of WriteClusterChain\n");
#endif
	return startCluster+1; //Return the number of the next possibly free cluster
}

auto u_int32 VoFatFindSector(register __i0 VO_FILE *f, register __reg_d u_int32 pos) {
	FatFileInfo *fi=(FatFileInfo*)f->fileInfo;
	if (__F_DIRTY(f)) fflush(f);
	if (pos < fi->currentFragment.startByteOffset ||
		pos >= fi->currentFragment.startByteOffset + fi->currentFragment.sizeBytes) {
		fi->currentFragment = FatGetFragmentFromFAT(f, fi->startCluster, pos);
	}
	return fi->currentFragment.startSector + ((pos - fi->currentFragment.startByteOffset) / 512);
}

#define PLEASE_ALLOCATE_LITTLE_MORE -1
#define PLEASE_ALLOCATE_MORE 1
#define DONT_ALLOCATE_MORE 0

u_int16 VoFatWriteFile(register __i0 VO_FILE *f, void *buf, u_int16 sourceIndex, u_int16 byteSize) {
	FatFileInfo *fi=(FatFileInfo*)f->fileInfo;
	u_int16 written = 0;
	__y u_int16 dstIdx = (u_int16)(f->pos & 511);
	u_int16 writeAmount;
		 
	/* Is our position in the wrong fragment, but not past end of file? */
	if (f->pos < fi->currentFragment.startByteOffset ||
	    (f->pos < f->fileSize && f->pos >= fi->currentFragment.startByteOffset+fi->currentFragment.sizeBytes)) {
#if 0
	  printf("HJÄLP1 %08lx %08lx %08lx %08lx!\n",
		 f->pos,
		 fi->currentFragment.startByteOffset,
		 fi->currentFragment.sizeBytes,
		 fi->currentFragment.startSector);
#endif
	  VoFatFindSector(f, f->pos);
#if 0
	  printf("HJÄLP2 %08lx %08lx %08lx %08lx!\n",
		 f->pos,
		 fi->currentFragment.startByteOffset,
		 fi->currentFragment.sizeBytes,
		 fi->currentFragment.startSector);
#endif
	}

	while (byteSize) {
		writeAmount = byteSize;			
		if ((f->pos >> 9) < ((f->pos + writeAmount - 1) >> 9)) {
			// Write from current offset until the end of the current sector
			writeAmount = 512 - ((u_int16)f->pos & 511);
		}
		{
			s_int32 fragPos = f->pos - fi->currentFragment.startByteOffset; //byte position in current fragment
			u_int32 thisSec = (fragPos/512) + fi->currentFragment.startSector;

			while (fragPos >= fi->currentFragment.sizeBytes) {
#if 0
			  if (f->pos >= 1063L*512 && f->pos <= 1066L*512) {
			    printf("WriteClusterChain\n");
			  }
#endif
#if 0
			  printf("Call WriteClusterChain #1 (please allocate more)\n");
#endif
				WriteClusterChain(f, &fi->currentFragment, PLEASE_ALLOCATE_MORE);
				if (!__F_WRITABLE(f)) {	
					f->flags |= __MASK_WRITABLE;				
					return written;
				}
				fragPos = f->pos - fi->currentFragment.startByteOffset; //byte position in current fragment
				thisSec = (fragPos/512) + fi->currentFragment.startSector;
			}			
			//f, sector, byteoffset, data, bytes
			FatImageWriteBytes(f, thisSec, (u_int16)(f->pos&511), buf, sourceIndex, writeAmount); //writeAmount <= 512
			sourceIndex += writeAmount;
			byteSize -= writeAmount;
			f->pos += writeAmount;
			if (f->pos > f->fileSize) {
				f->fileSize = f->pos;
			}
			written += writeAmount;
		}
	}
	return written;
}


ioresult VoFatCloseFile(register __i0 VO_FILE *f){ ///< Flush and close file
	FatFileInfo *fi=(FatFileInfo*)f->fileInfo;
	DirectoryEntry *entry = (void*)&f->sectorBuffer[fi->directoryEntryNumber*16];
	FatDeviceInfo *di=(FatDeviceInfo*)f->dev->deviceInfo;

	if (__F_WRITABLE(f)) {
		// CLOSING A WRITABLE FILE - UPDATE DIRECTORY
		VoFatFindSector(f, f->fileSize);
		FatImageSectorRead(f,fi->directoryEntrySector);
		entry->fileSizeLo = BYTESWAP((u_int16)(f->fileSize));
		entry->fileSizeHi = BYTESWAP((u_int16)((f->fileSize) >> 16));
		//This code needs currentTime struct from vo_fat.c. It follows the K&R C standard tm struct definition

		entry->writeDate = BYTESWAP(((currentTime.tm_year-80)<<9) | ((currentTime.tm_mon+1) << 5) | currentTime.tm_mday);
		entry->writeTime = BYTESWAP((currentTime.tm_hour << 11) | (currentTime.tm_min << 5) | (currentTime.tm_sec/2));
#if 0
		printf("writeDate %04x, writeTime %04x\n", BYTESWAP(entry->writeDate), BYTESWAP(entry->writeTime));
#endif

		if (f->fileSize == 0) {
			di->allocationStartCluster = ((u_int32)(entry->firstClusHi) << 16) | (entry->firstClusLo); //restore to free pool
			entry->firstClusHi = entry->firstClusLo = 0;
		}
		f->dev->BlockWrite(f->dev, f->currentSector, 1, f->sectorBuffer); //Write the directory entry		
		if (f->fileSize == 0) {
			return S_OK; //Return without writing cluster chain.
		}


		// Write the cluster chain of the last fragment		
#if 0
		  printf("Call WriteClusterChain #2 (don't allocate more)\n");
#endif

		  {
			u_int32 bytesInCurrentFrag = f->fileSize - fi->currentFragment.startByteOffset;
		    fi->currentFragment.sizeBytes = bytesInCurrentFrag;
		  }

		di->allocationStartCluster = WriteClusterChain(f, &fi->currentFragment, DONT_ALLOCATE_MORE);


	}				
	return S_OK;
}


u_int16 VoFatReadFile(register __i0 VO_FILE *f, void *buf, u_int16 byteOff, u_int16 byteSize) {
	__y u_int16 dstIdx = byteOff;
	u_int16 readAmount;
	
#if 0
	if (__F_DIRTY(f)) {
	  fflush(f);
	}
#endif
	
	while (byteSize) {
		readAmount = byteSize;
	
		/* Check sector boundary. */
		if ((f->pos >> 9) < ((f->pos + readAmount - 1) >> 9)) {
			readAmount = 512 - ((u_int16)f->pos & 511); //lohkon loppuun
			#if 0 //&& defined(USE_DEBUG)
			printf("left %d\n", readAmount);
			#endif
		}
		if (f->pos + readAmount > f->fileSize) {
			readAmount = (u_int16)(f->fileSize - f->pos);
			f->flags |= __MASK_EOF;
			#if 0 //&& defined(USE_DEBUG)
			printf("eof %d\n", readAmount);
			#endif
			if ((s_int16)readAmount <= 0) {
				return dstIdx - byteOff; /*EOF*/
			}
		}
	
		{		
			u_int32 sect = VoFatFindSector(f, f->pos);
			if (f->currentSector != sect) {
//				if (f->dev->BlockRead(f->dev, sect, 1, f->sectorBuffer) != S_OK) {
				if (FatImageSectorRead(f, sect) != S_OK) {
					f->flags |= __MASK_ERROR | __MASK_EOF;
					SysError("Read");
					return 0;
				}
				f->currentSector = sect;
			}
		}
		//printf("readAmount %d to dstIdx %d from srcIdx %d\n", readAmount, dstIdx, (u_int16)(f->pos & 511));
		//printf("source sectorBuffer(%04x):%04x %04x\n",f->sectorBuffer, f->sectorBuffer[0],f->sectorBuffer[1]);
		MemCopyPackedBigEndian(buf, dstIdx, f->sectorBuffer, (u_int16)(f->pos & 511), readAmount);
		//printf("destination buf:%04x %04x\n",buf[0],buf[1]);
		dstIdx += readAmount;
		f->pos += readAmount;
		byteSize -= readAmount;
	}
	return dstIdx - byteOff;
}




char *FatNameFromDirEntry(const u_int16 *dirEntry, char *longName){
	u_int16 i;
	char *l = longName;
	*l = 0;
	for (i=0;i<4;i++){
		*l++ = *dirEntry >> 8;
		*l++ = *dirEntry++ & 0xff;
	}
	l--;
	while (*l <= ' ') {
		l--;
	}	
	if (*dirEntry >> 8 != ' ') {
		*++l = '.';
	}
	*++l = *dirEntry >> 8;
	*++l = *dirEntry++ & 0xff;
	*++l = *dirEntry >> 8;
	while (*l <= ' '){
		l--;
	};
	*++l = '\0';
	//printf("E%s ",longName);
	return longName;	
}



// f MUST point to a DIRECTORY sector and its pos MUST point to next character after the directory entry to be used!
ioresult FatOpenEntry(register __i0 VO_FILE *f, DirectoryEntry *de) {
 	//FatDeviceInfo *di=(FatDeviceInfo*)f->dev->deviceInfo;
	volatile FatFileInfo *fi=(FatFileInfo*)f->fileInfo;
	//memset(f,0,sizeof(VO_FILE));
		
	fi->directoryEntrySector = (f->currentSector);
	fi->directoryEntryNumber = (u_int16)((f->pos - 32) & 511) / 32;

	f->pos = 0;
	f->fileSize = ((u_int32)BYTESWAP(de->fileSizeHi) << 16) | BYTESWAP(de->fileSizeLo) ;
	
	//printf("File size %ld bytes\n",f->fileSize);
	fi->startCluster = ((u_int32)BYTESWAP(de->firstClusHi) << 16) | BYTESWAP(de->firstClusLo) ;
	
	//printf("startCluster:%ld, fatEntry:%ld\n",fi->startCluster,VoFatReadClusterRecord(f,fi->startCluster));
	fi->currentFragment = FatGetFragmentFromFAT(f, fi->startCluster, 0);
	//printf("Fragment start:%ld, size %ld\n",fi->currentFragment.startSector, fi->currentFragment.sizeBytes);

	/* Store file attributes in ungetc_buffer: File, Directory, Label... */
	f->ungetc_buffer = de->attr&0xff;

	if (longFileName[0]) {
		//printf("HERE:%s\n",longFileName);
		//printf("Fi(%ld,%p)",fi->startCluster,f);
		lfnStartCluster = fi->startCluster;
	} else {
		lfnStartCluster = 0;
	}

	return S_OK;		
}


ioresult FatExtendDirectory(register __i0 VO_FILE *dirfile) {
	u_int32 newInChain = 0;
	u_int16 oldFlags = dirfile->flags;
	FatFileInfo *fi  =(FatFileInfo*)dirfile->fileInfo;
	FatDeviceInfo *di = (FatDeviceInfo*)dirfile->dev->deviceInfo;

#if 0
	printf("#&# Nyt olisi hyva aika varata lisaa\n");
#endif
	if (fi->currentFragment.startSector < di->dataStart) {
	  /* If startSector < dataStart, then we are in FAT12/16 root directory.
	     FAT12/16 root directories cannot be extended. */
	  return SysError("Root full");
	}
#if 0
	printf("df %p, fl %04x\n",
	       dirfile, dirfile->flags);
#endif
	dirfile->flags = __MASK_PRESENT | __MASK_OPEN | __MASK_SEEKABLE | __MASK_READABLE | __MASK_WRITABLE | __MASK_FILE;
#if 0
	printf("cfStart %ld\n", fi->currentFragment.startSector);
	printf("cfSize %ld\n", fi->currentFragment.sizeBytes);
#endif
	//printf("fwr %d\n", fwrite(&newInChain, sizeof(newInChain), 1, dirfile));
	WriteClusterChain(dirfile, &fi->currentFragment, PLEASE_ALLOCATE_LITTLE_MORE);
#if 0
	printf("fileSize %ld, pos %ld\n", dirfile->fileSize, dirfile->pos);
	printf("cfStart %ld\n", fi->currentFragment.startSector);
	printf("cfSize %ld\n", fi->currentFragment.sizeBytes);
#endif
	WriteClusterChain(dirfile, &fi->currentFragment, DONT_ALLOCATE_MORE);
	if (!(dirfile->flags & __MASK_WRITABLE)) {
	  //	  printf("#1# S_ERROR\n");
	  return S_ERROR;
	}
#if 0
	printf("df %p, fl %04x\n",
	       dirfile, dirfile->flags);
#endif
	{
	  int i;
	  u_int32 origPos = dirfile->pos;
	  for (i=0; i<fi->currentFragment.sizeBytes; i+=4) {
	    u_int32 t = 0;
	    fwrite(&t, sizeof(t), 1, dirfile);
	  }
	  fflush(dirfile);
	  dirfile->pos = origPos;
	}
	dirfile->flags = oldFlags;
	return S_OK;
}

// dirFile MUST point to a DIRECTORY sector and its pos MUST point to next character after the directory entry to be used!
ioresult FatMakeEntry(register __i0 VO_FILE *dirfile, const char *name) {
 	volatile FatDeviceInfo *di=(FatDeviceInfo*)dirfile->dev->deviceInfo;
//	volatile FatFileInfo *fi=(FatFileInfo*)dirfile->fileInfo;
 	//FatDeviceInfo *di=(FatDeviceInfo*)dirfile->dev->deviceInfo;
	FatFileInfo *fi=(FatFileInfo*)dirfile->fileInfo;
	u_int16 *secbuf = dirfile->sectorBuffer;
	DirectoryEntry *entry;
	u_int16 i,j;
	char name83[13];
	//printf("ennen: di->dataStart=%ld, di->fatSectorsPerCluster=%d\n",di->dataStart,di->fatSectorsPerCluster);

	//Make new file.
	fi->directoryEntrySector = (dirfile->currentSector);
	fi->directoryEntryNumber = (u_int16)((dirfile->pos - 32) & 511) / 32;
#if 0
	printf("#8 deSec:%ld deNum:%d\n", fi->directoryEntrySector, fi->directoryEntryNumber);
#endif
	entry = (void*)&dirfile->sectorBuffer[fi->directoryEntryNumber*16];

	dirfile->pos = 0;
	dirfile->fileSize = 0;
	fi->currentFragment.sizeBytes = 0;	
#if 0
	printf("##5.1: %ld\n", fi->currentFragment.sizeBytes);
#endif
	fi->startCluster = VoFatGetFreeFragment(dirfile, vo_fat_allocationSizeClusters);
#if 0
	printf("##5.2: start %ld, size %ld, dirpos %ld, dirsize %ld\n",
	       fi->startCluster, fi->currentFragment.sizeBytes,
	       dirfile->pos, dirfile->fileSize);
#endif

	FatImageSectorRead(dirfile, fi->directoryEntrySector);
	memset(entry, 0, 16); //first clear the entry to all zeros
	memset(name83,' ', sizeof(name83)); // note:the space character (0x20) is also a good initial file attribute ("archive")
	name83[sizeof(name83)-1]=0;
	i=0;j=0;
	while (j<11 && *name && i<12) {
		if (*name == '.') {
			j=7;						
		} else {
			name83[j] = toupper(*name);
			//printf("(%s)",name83);
		}
		name++;
		i++;
		j++;
	}
#if 0
	for (i=0; i<11; i++) {
		if ((name83[i]>='a') && (name83[i]<='z')) {
			name83[i] -= 'a'-'A';
		}
	}
#endif

	//printf("Name:[%s] NAME83: [%s]\n",name,name83);
	for (i=0; i<6; i++) { //Put in file 8.3 file name and attribute 0x20
		entry->filename[i] = (name83[i*2]<<8) + (name83[i*2+1]);
	}
	{
		u_int32 startCluster = fi->startCluster;
		entry->firstClusLo = BYTESWAP ((u_int16)startCluster);
		entry->firstClusHi = BYTESWAP ((u_int16)(startCluster >> 16));
	}

//printf("startCluster:%ld, secperclus:%d, datastart:%ld\n",fi->startCluster, di->fatSectorsPerCluster, di->dataStart);
//debug(fi);
	fi->currentFragment.startSector = fi->startCluster * di->fatSectorsPerCluster + di->dataStart; //VoFatClusterToSector(dirfile, startCluster);
	//	fi->currentFragment.sizeBytes = (u_int32)vo_fat_allocationSizeClusters * di->fatSectorsPerCluster * 512;
#if 0
	printf("##6: %ld\n", fi->currentFragment.sizeBytes);
#endif

	entry->fileSizeLo = BYTESWAP ((u_int16)(fi->currentFragment.sizeBytes));
	entry->fileSizeHi = BYTESWAP ((u_int16)(fi->currentFragment.sizeBytes >> 16));
		
	//printf("About to write entry.\n");	
	//PrintBuffer(secbuf);
	
	dirfile->dev->BlockWrite(dirfile->dev, fi->directoryEntrySector, 1, secbuf);
	//printf("New file starts at cluster %ld, sector %ld, allocated %ld bytes.\n",fi->startCluster,fi->currentFragment.startSector,fi->currentFragment.sizeBytes);
	dirfile->flags = __MASK_PRESENT | __MASK_OPEN | __MASK_SEEKABLE | __MASK_WRITABLE;	
	//printf("di->dataStart=%ld, di->fatSectorsPerCluster=%d\n",di->dataStart,di->fatSectorsPerCluster);
	
	return S_OK;
}

int FileNameCompare(register const char *filespec, register const char *candidate, register s_int16 n) {
	const char *w;
	int cLen;
	
	if (n<=0) return -1; //empty name does not match
	w=strchr(filespec,'*');
	if (w >= filespec+n) {
		w = NULL;
	}
	//fprintf(vo_stderr,"C %s,%s,%s, n=%d\n",filespec,candidate,w?w:"(null)", n);	
	if (!w) {
		if (strlen(candidate)>n) {
			return -1;
		}
		return strncasecmp(candidate, filespec, n);
	}
	//Compare left until *
	if (strncasecmp(candidate, filespec, w-filespec)) {
		return -1;
	}
	w++;
	n -= w-filespec;
	if (n <= 0) {
		return 0;
	}
	//fprintf(vo_stderr,"#1\n");
	cLen = strlen(candidate);
	if (cLen < n) {
		return -1;
	}
	candidate += cLen-n;
	//fprintf(vo_stderr,"e %s,%s n=%d\n",candidate,w,n);	
	//compare right from *
	return strncasecmp(candidate,w,n);	
}

#if 0
u_int16 GetByte(u_int16 *array, u_int16 byteIndex) {
	if (byteIndex & 1) {
		return array[byteIndex>>1] & 0xff;
	} else {
		return array[byteIndex>>1] >> 8;
	}
}
#else
#define GetByte(s,idx) MegaFatGetByte((s),(idx))
#endif


extern const FILEOPS FatFileOps;

ioresult FatFindFile(register __i0 VO_FILE *f, u_int32 dirStartCluster, char *name, const char *mode) {
 	FatDeviceInfo *di=(FatDeviceInfo*)f->dev->deviceInfo;
	FatFileInfo *fi=(FatFileInfo*)f->fileInfo;
	DirectoryEntry de;
	u_int16 searchingForDirectory = 0;
	s_int32 longNameStartPos = -1;
	u_int16 lfnReset = 0;
	u_int16 unlinkEntry = 0;
	u_int16 writeEntry = 0;
	u_int16 skipEntries = 0;
	char tempName[13];

	//Clear the long file name buffer
	memset(longFileName,0,sizeof(longFileName));
	lfnStartCluster = 0;

	if (f->op != &FatFileOps) return S_ERROR; //Cannot FAT search if FS is not FAT

	if (mode[0] == 'w') { //write
	  unlinkEntry = 1;
	  writeEntry = 1;
	}

	if (mode[0] == 'U') { //Unlink
	  unlinkEntry = 1;
	}
	
	if (mode[0] == 'N') {
#if 0
		printf("### SFD\n");
#endif
		searchingForDirectory = 0;
		goto readEntry;
	}

	if(mode[2]=='#') { //Skip N entries
		skipEntries = atoi(&mode[3]);
	}

	while (1) {
		u_int16 i;
		nextEntry:
		tryAgain:
		searchingForDirectory = 0;
		//		printf("##name \"%s\", unlinkEntry %d, writeEntry %d\n", name, unlinkEntry, writeEntry);
		for (i=0; name[i]; i++) {
			if (name[i]=='/' || name[i]=='\\') {
			  //			  printf("Jaahas, %c @ %d\n", name[i], i);
#ifdef VCC
				searchingForDirectory = i; 
#else
				__asm {
				  // Required to go around LCC bug (HH 2013-12-13)
				  get (searchingForDirectory),a0
				  get (i),a1
				  mv a1,a0
				  set a0,(searchingForDirectory)
				}
#endif
				break;
			}
		}
#if 0
		printf("FindFile: name:%s dir:%d\n",name,searchingForDirectory);
		//printf("dirStartCluster:%ld\n",dirStartCluster);
#endif

		if ((dirStartCluster == 0) && (di->fatBits != 32)) { //Fat12, Fat16
			fi->currentFragment.startSector = di->rootStart;
			f->fileSize = fi->currentFragment.sizeBytes = (u_int32)di->rootEntCnt*32;
			f->pos = 0;
#if 0
			printf("##7: startSec %ld, size %ld\n",
			       fi->currentFragment.startSector, f->fileSize);
#endif

			fi->currentFragment.startByteOffset = 0;			
		} else { //Fat32
			if (dirStartCluster == 0) {
				dirStartCluster = di->rootStart;
			}
			fi->startCluster = dirStartCluster;
			f->pos = 0;
			f->fileSize = 65536*32; //Max length of FAT directory
			fi->currentFragment = FatGetFragmentFromFAT(f, fi->startCluster, 0);
		}
				
		while (1) {
			//printf("dirflags:%04x ",f->flags);
			if __F_EOF(f) {
				return SysError("End of dir");
			}
readEntry:
#if 0
			printf("### f->fileSize %ld\n", f->fileSize);
#endif
			if (f->pos >= f->fileSize && !unlinkEntry && writeEntry) {
#if 0
			  printf("## Extend\n");
#endif
			  if (FatExtendDirectory(f)) {
			    return SysError("Disk full");
			  }
			}

			if (!f->op->Read(f, &de, 0, 32)) { //Read directory entry
			  //			  printf("### FLAGS2 %04x\n", f->flags);
			  if (__F_EOF(f)) {
			    //			    printf("### EOF %d %d!\n", writeEntry, unlinkEntry);
			    if (unlinkEntry) {
			      //			      printf("#0\n");
			      if (writeEntry) {
				unlinkEntry = 0;
				f->pos = 0;
				f->flags &= ~(__MASK_EOF);
				//				printf("#1\n");
				goto nextEntry;
			      } else {
				//				printf("#2\n");
				return E_FILE_NOT_FOUND;
			      }
			    }
			  }
#if 0
			  return SysError("Can't read dir");
#else
			  // 2018-02-01 HH: File ending while reading a directory happens when the directory is
			  // full and is not an error situation.
			  return E_FILE_NOT_FOUND;
#endif
			}
#if 0
			printf("READ %d %d %d %04x %04x %04x %04x endPos %ld\n",
			       searchingForDirectory, writeEntry, unlinkEntry,
			       de.filename[0], de.filename[1],
			       de.filename[2], de.filename[3],
			       f->pos);
#else
			__asm {
			  get (&de),a0
			    add a0,null,a1
			    add a,null,b
			    add a,null,c
			}
#endif

			if (!searchingForDirectory && writeEntry &&
			    !unlinkEntry &&
			    (((de.filename[0] & 0xff00) == 0x0000) ||
			     ((de.filename[0] & 0xff00) == 0xe500))) {
			  int t;
			  // Create a new file and a directory entry for it.
			  //			  printf("Make FAT entry before %ld\n", f->pos);
			  fi->currentFragment.startByteOffset = 0;
			  t = FatMakeEntry(f, name);
#if 0
			  printf("##5 %ld %ld %ld\n", fi->currentFragment.startSector,
				 fi->currentFragment.startByteOffset, f->pos);
#endif
			  return t;
			}

			
			if (!(de.filename[0] & 0xff00)) {
				//End of directory
				break;
			}				
			if ((de.filename[0]>>8) == 0xe5) {
				longNameStartPos = -1;
			} else {
				if ((de.attr & __ATTR_LONGNAME) != __ATTR_LONGNAME) {
					
					if (++lfnReset == 2) {
						longFileName[0] = 0;
						lfnReset = 1;
					}
					FatNameFromDirEntry((u_int16*)&de, tempName);
					if (longFileName[0]==0) {
						//fprintf(vo_stderr, "strcpy(d, %s)\n", tempName);
						strcpy(longFileName,tempName); //If no longFileName, set it from short file name
						
					}
					if (searchingForDirectory) {
						if (((!FileNameCompare(name,longFileName,searchingForDirectory))
						|| (!FileNameCompare(name,tempName,searchingForDirectory)))) {
							//fprintf(stderr,"directory found %s\n",tempName);
							name += searchingForDirectory+1;
							dirStartCluster = ((u_int32)BYTESWAP(de.firstClusHi) << 16) | BYTESWAP(de.firstClusLo) ; 
							goto nextEntry;
						}
					} else {
						u_int16 namelen = strlen(name);
						
						if (mode[0]=='s' || mode[0]=='N') { //Finding File for directory list
							if (f->extraInfo) { //Return extended file info
								u_int16 len = f->ungetc_buffer-18;
								char *p = &(f->extraInfo[len]);
								strncpy(f->extraInfo,longFileName,len);
								*p++ = 0;
								f->extraInfo = p;
								strcpy(p,tempName);
								p += 13;
								*p++ = de.filename[4]>>8;   //First letter of file extension
								*p++ = de.filename[4]&0xff; //Second letter of file extension
								*p++ = de.attr>>8;          //Third letter of file extension
								*p = 0;
							}
							f->ungetc_buffer = de.attr&0xff; //stash attributes for file searching
							return S_OK; //for findnext or findfirst, the entry matches without comparing the file name
						}
						
						if (((!FileNameCompare(name,longFileName,namelen))
							|| (!FileNameCompare(name,tempName,namelen)))
							&& ((de.attr & __ATTR_VOLUMEID) == 0) && (skipEntries-- == 0)) {

						  if (unlinkEntry) {
						    u_int32 endPos = f->pos;
						    u_int32 currCluster = ((u_int32)(BYTESWAP(de.firstClusHi)) << 16) | BYTESWAP(de.firstClusLo);
						    // Remove current entry
#if 0
						    printf("######!!###Unlink entry \"%s\", pos %ld, longNameStartPos %ld\n", name, f->pos, longNameStartPos);
#endif
						    f->pos = (longNameStartPos>=0) ? longNameStartPos : f->pos-32;
#if 1
						    do {
						      u_int16 d = 0xe5e5;
#if 0
						      printf("Clear %ld, firstCluster %ld\n", f->pos, currCluster);
#endif
						      f->op->Write(f, &d, 0, 1);
						      f->pos += 31;
						    } while (f->pos < endPos);
#endif

						    do {
						      u_int32 oldCluster = currCluster;
#if 0
						      printf("## CC 0x%06lx -> ", currCluster);
#endif
						      currCluster = VoFatReadClusterRecord(f, currCluster);
						      if (!currCluster) {
						      	
						      	break;
						      }
							  
						      VoFatWriteClusterRecord(f, oldCluster, 0);
#if 0
						      printf(" 0x%06lx\n", currCluster);
#endif
						    } while (currCluster < 0x0ffffff8);

						    FatFlush(f);
						    if (writeEntry) {
						      unlinkEntry = 0;
						      goto tryAgain;
						    }
						    return S_OK;
						  } else {
						    s_int16 result = writeEntry ? S_ERROR : FatOpenEntry(f,&de);
						    return result;
						  }
						}
					}
					longNameStartPos = -1;
				} else {
				  //longname				
					
				u_int16 fn = GetByte((void*)&de,0);
				register u_int16 idx = ((fn & 0x3f) - 1) * 13;
				static const short places[] = {
					1, 3, 5, 7, 9, 14, 16, 18, 20, 22, 24, 28, 30, -1
				};
				register const short *p = &places[0];
				while (*p >= 0 && idx < FAT_LFN_SIZE-1 /* space for NUL */) {
					longFileName[idx++] =
					(GetByte((void*)&de, *p+1) << 8) + GetByte((void*)&de, *p++);
				}
				/* last long entry ? */
				if ( fn & 0x40 /*LAST_LONG_ENTRY*/ ) {
					if (idx >= FAT_LFN_SIZE)
					idx = FAT_LFN_SIZE-1;
					longFileName[idx] = '\0';
				}
				lfnReset = 0;
				


				  if (longNameStartPos < 0) {
				    longNameStartPos = f->pos-32;
#if 0
				    printf("longNameStartPos %ld %04x %04x ",
					   longNameStartPos, de.filename[0], de.filename[1]);
				    putchar(de.filename[0] & 0xFF);
				    putchar(de.filename[1] & 0xFF);
				    putchar(de.filename[2] & 0xFF);
				    putchar(de.filename[3] & 0xFF);
				    putchar(de.filename[4] & 0xFF);
				    putchar(de.filename[7] >> 8);
				    putchar(de.filename[8] >> 8);
				    putchar(de.filename[9] >> 8);
				    putchar(de.filename[10] >> 8);
				    putchar(de.filename[11] >> 8);
				    putchar(de.filename[12] >> 8);
				    putchar(de.filename[14] >> 8);
				    putchar(de.filename[15] >> 8);
				    putchar('\n');
#endif
				  }
				}
			} /* !de.filename[0]>>8) != 0xe5) */
		} /* while (1); */
		//SysReport("File not found");
#if 0
		printf("#### file not found %d %d\n", unlinkEntry, writeEntry);
#endif
		if (unlinkEntry && writeEntry) {
		  unlinkEntry = 0;
		  goto tryAgain;
		}
		return E_FILE_NOT_FOUND;
	}
}

ioresult FatFindFirst(register __i0 VO_FILE *f, const char *name, char *result, u_int16 len) {
	if (len>18) { //Short file name + extension + zeros
		f->extraInfo = result;
		f->ungetc_buffer = len;	
	} else {
		f->extraInfo = NULL; //not enough space to return an extended result
	}
	if (S_OK == FatFindFile(f,0,name,"s")) {
		return S_OK;		
	} else {
		return S_ERROR;
	}
}

ioresult FatFindNext(register __i0 VO_FILE *f, char *result, u_int16 len) {
	if (len>18) { //Short file name + extension + zeros
		f->extraInfo = result;
		f->ungetc_buffer = len;	
	} else {
		f->extraInfo = NULL; //not enough space to return an extended result
	}
	if (S_OK == FatFindFile(f,0,"","N")) {
		return S_OK;		
	} else {
		return S_ERROR;
	}
}


ioresult VoFatOpenFile(register __i0 VO_FILE *f, const char *name, char *mode) {
 	FatDeviceInfo *di=(FatDeviceInfo*)f->dev->deviceInfo;
	FatFileInfo *fi=(FatFileInfo*)f->fileInfo;
	ioresult errorCode;
  int append = 0, update = 0;
  char modeCopy[4];
  strncpy(modeCopy, mode, 3);
  modeCopy[3] = '\0';

  update = strchr(mode, '+') ? 1 : 0;
  if ((mode[0] != 'r' || update) && !(f->dev->flags & __MASK_WRITABLE)) {
	  return E_FILE_NOT_FOUND;
	}

  if (modeCopy[0] == 'a') {
    append = 1;
    modeCopy[0] = 'r';
  }

 tryagain:
  if ((errorCode = FatFindFile(f, 0, name, modeCopy)) == S_OK) {				
    if (modeCopy[0]=='U') {
		  return S_OK; // Unlinked file, only __MASK_PRESENT is set
		}
    f->pos = 0;
    if (append) {
      f->pos = f->fileSize;
    }
    if (mode[0]=='r' && !update) { /* Note: NOT modeCopy */
		f->flags = __MASK_OPEN | __MASK_READABLE | __MASK_SEEKABLE | __MASK_PRESENT;
			return S_OK; //Found a file for reading
		}
    f->flags = __MASK_OPEN | __MASK_READABLE | __MASK_WRITABLE | __MASK_SEEKABLE | __MASK_PRESENT;
		return S_OK;
  }

  if (append) {
    /* If can't find file in append mode, try to create it in write mode. */
    modeCopy[0] = 'w';
    append = 0;
    goto tryagain;
	}

	// File not found
	if (mode[0]=='r'||mode[0]=='U') {
		//SysReport("File not found");
		return errorCode;
	}
	if (mode[0]=='w') {
		return SysError("Cannot create file");
	}
	return S_OK;
}

//char *IdentifyFat(register __i0 void *fs, char *buf, u_int16 bufsize){
//	return "FAT";
//}

const char* IdentifyFat (register __i0 VO_FILE *f, char *buf, u_int16 bufsize) {
	static char s[13];
	if ((f) && (__F_OPEN(f)) && ((void*)f != &FatFileSystem)) {		
		if (f->dev->fs == &FatFileSystem) {
			FatFileInfo *fi=(FatFileInfo*)f->fileInfo;
			if (fi->startCluster == lfnStartCluster) {
				return longFileName;
			} else {	
				u_int16 *buf=malloc(256);
				char *result = "FILE";
				if (buf) {
					f->dev->BlockRead(f->dev,fi->directoryEntrySector,1,buf);					
					result = FatNameFromDirEntry(&buf[(fi->directoryEntryNumber)*16], s);
					free(buf);
				}
				return result;
			}
		}
	}
	return "FAT";
}



#pragma msg 30 off //CommonErrorResultFunction triggers warning
const FILEOPS FatFileOps = {
	VoFatOpenFile, //ioresult (*Open) (register __i0 VO_FILE *f, const char *name, const char *mode); ///< Find and open a file
	VoFatCloseFile, //ioresult (*Close)(register __i0 VO_FILE *f); ///< Flush and close file
	CommonErrorResultFunction,	//ioresult (*Ioctl)(register __i0 VO_FILE *f, s_int16 request, char *argp); ///< For extensions, not used.	
	VoFatReadFile, //u_int16  (*Read) (register __i0 VO_FILE *f, void *buf, u_int16 destinationIndex, u_int16 bytes); ///< Read bytes to *buf, at byte index
	VoFatWriteFile, //u_int16  (*Write)(register __i0 VO_FILE *f, void *buf, u_int16 sourceIndex, u_int16 bytes); ///< Write bytes from *buf at byte index
};
#pragma msg 30 on

const FILESYSTEM FatFileSystem = {
	__MASK_PRESENT |__MASK_OPEN | __MASK_SEEKABLE | __MASK_READABLE | __MASK_WRITABLE, //u_int16  flags; //< present, initialized,...
	IdentifyFat, //char*    (*Identify)(void *obj, char *buf, u_int16 bufsize);
	FatCreate, // ioresult (*Create)(DEVICE *dev, char *name);
	0, //ioresult (*Delete)(DEVICE *dev);
	0, //ioresult (*Ioctl) (DEVICE *dev);
	&FatFileOps, //FILEOPS  *op;	// Opened file inherits this set of file operations.
};
