#include <hwlocks.h>
#include <string.h>
#include <saturate.h>
#include <stdlib.h>
#include "unimap.h"
__mem_y struct PageAddressStruct deshardPage;

ioresult FlushPage(register __mem_y struct PageStruct *page);
static
/* Rewriter for writing two shards as one to desharding area. */
ioresult SimpliWriter(register s_int32 lba, register s_int32 sectors){
	u_int16 i, *target, shardCount = pur.shards, writePages = deshardControl.pagesToWrite, written = 0, pleaseWrite;

	while(deshardControl.pagesToWrite && sectors > 0){
		u_int32 sba;
		target = pageBuffer;
		/* Repurposing lbaPresent. */
		deshardPage.lbaPresent = sectors;
		if (!__F_WRITABLE(&devMlcFlash)){
			return S_ERROR;
		}
		/* Next page. Save last SBA for comparison. */
		sba = deshardPage.lastSba;
		NextWritePage((__mem_y struct PageStruct *)&deshardPage, 0);
		if (deshardPage.lastSba != sba){
			/* New erase block was allocated. */
			if (deshardPage.lastSba != sba + nf.sectorsPerEraseBlock){
				sba = deshardPage.nextSba;
			}
			/* Allocate bigger area. */
			NextWritePage((__mem_y struct PageStruct *)&deshardPage, PURITY_DESHARDING_BLOCKS);
			/* Is it still continous or is the first allocated EB missed? */
			if (deshardPage.nextSba == sba + nf.sectorsPerEraseBlock){
				/* sba can be used. */
				deshardPage.nextSba = sba;
			}
		}
		if (pur.pg[0].addr.lbaPresent && pur.pg[0].addr.firstLba == lba){
			FlushPage(&(pur.pg[0]));
		}
		if (pur.pg[1].addr.lbaPresent && pur.pg[1].addr.firstLba == lba){
			FlushPage(&(pur.pg[1]));
		}
		pleaseWrite = 0;
		for(i = 0; i < PURITY_SECTORS_PER_PAGE; i++){
			u_int32 lsn = -1;
			sba = ShardFindSector(lba);
			/* If SBA -1, sector isn't in book keeping. Let's just add the lba number to the
			   end and keep sector out of system. */
			if ((s_int32)sba != -1){
				/* Read the LBA data */
				lsn = nf.Read(sba>>3, (u_int16)(sba & 7) * SECTOR_SIZE_BYTES, 512, target);
				if (nf.lastReadRSFail){
					devMlcFlash.flags &= ~(__MASK_WRITABLE);
					return S_ERROR;
				}
				/* Integrity check. */
				if (lsn != lba) {
	#ifndef __VSDSP__
					printf("Purity deshard expected lba %x, got lba %x, from sba %x\n",lba,lsn,sba);
	#else
					printf("Purity deshard expected lba %lx, got lba %lx, from sba %lx\n",lba,lsn,sba);
	#endif
					InfoMapper();
					PrintShards();
					return S_ERROR;
				}
				/* Update shard book keeping. */
				if (S_OK != ShardAddSector(lba, deshardPage.nextSba + i)){
					SysError("Purity deshard: Update shards fail.");/* Disk is read only. */
					return S_ERROR;
				}
				pleaseWrite = 1;
			}
			/* Add LBA number to the end */
			*(u_int32*)(&target[256]) = (u_int32)lsn;
			/* Another happy customer. NEXT! */
			target += SECTOR_SIZE_WORDS;
			lba++;
		}
		/* Write the page. */
		if (pleaseWrite && nf.ProgramPage(deshardPage.nextSba / 8, pageBuffer) != S_OK){
			printf("\n\n\n\n\nWRITE FAIL IN DESHARD!!!!!!!!!!!!\n\n\n\n\n");
			while(1);
		}
		/* Now sectors have been written. */
		sectors -= 8;
		writePages--;
		/* Free some space if possible. */
		if (written++ > nf.pagesPerEraseBlock * 32){
			PushFUS();
			written = 0;
		}
		/* Should there be panic or death? */
		if ((pur.shards > PURITY_MAX_SHARDS - PURITY_DESHARD_THRESHOLD) ||
			deshardControl.deshardMode & DESHARD_DIE ||
			pur.shards > shardCount + deshardControl.blockStep){ /* page write count too low probably. */
			deshardPage.firstLba = lba;
			return;
		}
		if (writePages == 0){
			writePages = deshardControl.pagesToWrite;
			ReleaseHwLocksBIP(DESHARDER_LOCK, 0,0);
			Yield();/* Give some CPU time */
			ObtainHwLocksBIP(DESHARDER_LOCK, 0,0);
		}
	}
	deshardPage.firstLba = lba;
	return S_OK;
}

static
/* Check if write pages are pointing to
 * startLba ... startLBa+sectors area. */
u_int16 IsInPages(register s_int32 startLba,
				    register s_int32 sectors){
	if (pur.pg[0].addr.lbaPresent &&
	    pur.pg[0].addr.firstLba >= startLba &&
	    pur.pg[0].addr.firstLba < startLba + sectors)return 1;
	if (pur.pg[1].addr.lbaPresent &&
	    pur.pg[1].addr.firstLba >= startLba &&
	    pur.pg[1].addr.firstLba < startLba + sectors)return 2;
	return 0;
}

static
void HandleBlockRefresh(register __mem_y struct Shard *s){
	u_int32 refreshSba = nf.refreshQueue[0] << 3;
	if (s->startSba <= refreshSba + nf.sectorsPerEraseBlock &&
	    s->startSba + s->sectors > refreshSba){
	 	/* This shard belongs to refreshable EB. */
	 	s_int32 size = nf.sectorsPerEraseBlock;
	 	if (size > s->sectors){
			size = s->sectors;
		}
 		SimpliWriter (s->startLba, size);
	}
}

void SimpliRefresher(){
	if (nf.entriesToRefresh == 0)return;
	ObtainHwLocksBIP(DESHARDER_LOCK, 0,0);
	while(nf.entriesToRefresh){
		if((nf.refreshQueue[0] << 3) < PURITY_SURFACE_START){
			SaveState();
		} else {
			__mem_y struct Shard *s = &shard[0];
			u_int16 i;
			for(i = 0; i < pur.shards; i++){
				HandleBlockRefresh(s);
			}
		}
		nf.entriesToRefresh--;
		if(nf.entriesToRefresh){
			memmove(&nf.refreshQueue[0], &nf.refreshQueue[1],
					(nf.entriesToRefresh) * sizeof(u_int32));
		}
	}
	ReleaseHwLocksBIP(DESHARDER_LOCK, 0,0);
}
static
u_int16 WritePagesPerSector(register u_int32 usedSpace){
	u_int16 ret, spaceComp;
	u_int32 uselessSectors = usedSpace - deshardControl.sectors - (PURITY_DESHARDING_BLOCKS * nf.sectorsPerEraseBlock);
	if (uselessSectors > pur.usableSectors){
		uselessSectors = 0;
	}
	deshardControl.uselessSectors = uselessSectors;
	spaceComp = (u_int16)(uselessSectors / ((u_int32)nf.sectorsPerEraseBlock * deshardControl.blockStep));
	ret = 1 + spaceComp + pur.shards / (PURITY_MAX_SHARDS / DESHARD_MAX_WRITE_PAGES);
	if (ret > 0)ret--;
	if (ret > DESHARD_MAX_WRITE_PAGES) ret = DESHARD_MAX_WRITE_PAGES;
	return ret;
}

/* Sort shards to logically ascending order. */
int shardLbaComp(const __mem_y struct Shard *l, const __mem_y struct Shard *r){
	return Sat32To16(l->startLba - r->startLba);
}

ioresult SimpliSharder(){
	__mem_y struct Shard *s = &shard[0], *sFriend = NULL;
	u_int32 sectors, startLba = (u_int32)-1, writeSpace = pur.usableSectors - pur.nextAvailableSba;
	s_int32 fusLba, fusSectors;
	u_int16 i, pairFound = 0;
	static u_int16 chunkCount = 0;
	ObtainHwLocksBIP(DESHARDER_LOCK, 0,0);
	{
		__mem_y struct Shard *sFus;
		sFus = PushFUS(); /* Update some data in deshardControl. Sorting would break sFus. */
		fusLba = sFus->startLba;
		fusSectors = sFus->sectors;
	}
	if (deshardControl.sortNeeded){
		extern __mem_y struct Shard *lastShard;
		lastShard = NULL;
		qsorty((void *)shard, pur.shards, sizeof(struct Shard),
		(int (*)(const void *, const void *))shardLbaComp);
		deshardControl.sortNeeded = 0;
	} //else deshardControl.uselessSpace = 0;
	{
		u_int32 usedSpace = pur.nextAvailableSba - pur.firstUsedSba;
		if (pur.firstUsedSba > pur.nextAvailableSba){
			usedSpace = pur.usableSectors - PURITY_SURFACE_START - pur.firstUsedSba + pur.nextAvailableSba;
			writeSpace = pur.firstUsedSba - pur.nextAvailableSba;
		}
		deshardControl.pagesToWrite = WritePagesPerSector(usedSpace);
	}
	if (writeSpace > PURITY_DESHARDING_BLOCKS * nf.sectorsPerEraseBlock){
		writeSpace -= (PURITY_DESHARDING_BLOCKS * nf.sectorsPerEraseBlock);
	}
	/* Search the smallest pair and extension of last deshard. After that check if
	   sFus is the one which is wanted. This works in panic deshard and in normal deshard same way. */
	sectors = writeSpace +1; /* Set to a good default. */
	/* If sectors > writespace, chunk is clipped from the end. However previous shard grew
	 * and leftover reduced in size. In for loop the smallest pair is searched and bigger next chunk,
	 * won't get written. */
	if (!IsInPages(s->startLba, s->sectors) && s->startLba == deshardPage.firstLba){
		startLba = s->startLba;
		sectors = s->sectors;
	}
	s++;
	for (i = 1; i < pur.shards; i++){
		if (s->startLba == deshardPage.firstLba&&s->sectors <= sectors) {
			startLba = s->startLba;
			sectors = s->sectors;
		}
		//if (s->startLba == fusLba + fusSectors) sFriend = s;
		//printf("S-1: %lx (%lx), S: %lx (%lx) conn: %lx\n",(s-1)->startLba,(s-1)->sectors,s->startLba, s->sectors, (s-1)->startLba + (s-1)->sectors);
		//printf("Sectors: %lx, sum: %lx\n",sectors, s->sectors + (s-1)->sectors);
		if (!IsInPages((s-1)->startLba, (s-1)->sectors) && /* Not incomplete */
			 !IsInPages(s->startLba, s->sectors)&& /* Not incomplete */
			 (s-1)->startLba + (s-1)->sectors == s->startLba){  /* It's a pair.*/
			pairFound++;
			if (s->sectors + (s-1)->sectors < sectors /* It fits*/){
				startLba = (s-1)->startLba;
				sectors = (s-1)->sectors + s->sectors;
			}
			//printf("J %lx (%lx)", startLba, sectors);
		}
		s++;
	}
	if (!pairFound){
		deshardControl.noPairShards = pur.shards;	
	}
	/* Small pairs are always rewritten */
	if (sectors < nf.sectorsPerEraseBlock){goto writeIt;}
	if (deshardControl.uselessSectors &&
		pur.shards < PURITY_MAX_SHARDS - PURITY_DESHARD_THRESHOLD - PURITY_SMALL_HYSTERESIS){
		if (chunkCount++ > deshardControl.pagesToWrite){
			startLba = fusLba;
			sectors = fusSectors;
			chunkCount = 0;
		}
	} else chunkCount = 0;
	if (deshardControl.uselessSectors > writeSpace / 2 &&
		pur.shards < PURITY_MAX_SHARDS - PURITY_DESHARD_THRESHOLD - PURITY_SMALL_HYSTERESIS ){
		startLba = fusLba;
		/* Evil extension of fus. This may consume PURITY_SRFACE_SATART sectors.
		 * However this may solve infinite desharding. */
		sectors = fusSectors + PURITY_SURFACE_START;
		//if (sFriend)sectors += sFriend->sectors;
		/* Maximum write. Let I/O to starve a little bit. */
		deshardControl.pagesToWrite = DESHARD_MAX_WRITE_PAGES;
	} else if(!pairFound)goto finally;
writeIt:
	if (sectors > writeSpace)sectors = writeSpace;
	SimpliWriter(startLba, sectors);
finally:
	ReleaseHwLocksBIP(DESHARDER_LOCK, 0,0);
	return S_OK;
}
