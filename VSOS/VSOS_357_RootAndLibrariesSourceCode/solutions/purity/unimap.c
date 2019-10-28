#ifndef __VSDSP__
#include "compat.h"
#include "flash.h"
#else
#include <vo_stdio.h>
#include <vo_fat.h>
#include <saturate.h>
#include "compact.h"
#include "devMlcFlash.h"
#include <audio.h>
#endif
#include "unimap.h"
#include <string.h>
#include <stdlib.h>



#define CHARS_PER_WORD (sizeof(u_int16)/sizeof(char))

u_int16 pageBuffer[NAND_PAGE_AND_SPARE_SIZE_WORDS];
u_int32 realDataStart = 0;
u_int32 fatStart;
u_int32 fat2Start;
u_int32 fatSize;

struct Purina __mem_y pur;
struct Shard __mem_y shard[PURITY_MAX_SHARDS];
/* Shortcut optimization for ShardAdd Sector*/
__mem_y struct Shard *lastShard;

/*
//__mem_y u_int16 pagedata[2 * NAND_PAGE_AND_SPARE_SIZE_WORDS]; // storage for 2 pages, 4KBytes each; 16 * 256 words
s_int32 lbas[16] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
s_int32 currentDataLba = -1;
s_int32 currentFatLba = -1;
*/
/*
__mem_y u_int16 * YFindSector(s_int32 lba) {
	u_int16 i;
	for (i=0; i<16; i++) {
		if (lbas[i] == lba) {
			return &pagedata[i*256];
		}
	}
	return NULL;
}
*/


__mem_y u_int16 * PageFindSector(register __mem_y struct PageStruct *pg, register s_int32 lba) {
	u_int16 subpage = (u_int16)lba & 7; //8 lbas per page
	if (pg->addr.firstLba != lba - subpage) {
		return NULL;
	}
	////x// printf("pg@%p sub:%d present:%x ",pg,subpage,pg->lbaPresent);
	if (!((u_int16)(pg->addr.lbaPresent) & (1 << subpage))) {
		////x// printf("n ");
		return NULL;
	}
	////x// printf("y ");
	return &pg->data[subpage * SECTOR_SIZE_WORDS];
}

__mem_y u_int16 * YFindSector(register s_int32 lba) {
	__mem_y u_int16 *result = PageFindSector(&(pur.pg[0]),lba);
	if (result) return result;
	return PageFindSector(&(pur.pg[1]),lba);
}

void PrintShards() {
	__mem_y struct Shard *s = &shard[0];
	u_int16 i;
	static const char *ends[] = {
		"", " (0)", " (1)", " (D)"
	};
	char *e;
	printf("\n___SHARDS___\n");

	for (i=0; i<pur.shards; i++) {
		e = ends[0];
		if (pur.pg[0].addr.firstLba >= s->startLba &&
	    pur.pg[0].addr.firstLba < s->startLba + s->sectors)e = ends[1];
		if (pur.pg[1].addr.firstLba >= s->startLba &&
	    pur.pg[1].addr.firstLba < s->startLba + s->sectors)e = ends[2];
		if (deshardPage.firstLba >= s->startLba &&
	    deshardPage.firstLba < s->startLba + s->sectors)e = ends[3];
#ifndef __VSDSP__
		printf("%3d: %6x..%6x -> %6x..%6x (size %6x) %s\n",
#else
		printf("%3d: %6lx..%6lx -> %6lx..%6lx (size %6lx) %s\n",
#endif
			i,s->startLba,s->startLba+s->sectors-1,s->startSba,s->startSba+s->sectors-1,s->sectors, e);
		
		s++;
	}
}

#if 0
s_int32 Save(u_int16 *p, s_int16 nWords, s_int32 toRow) {
	while (nWords > 0) {
		memcpy (pageBuffer, p, 256);
		//x// printf("Save from X:%p to row %lx\n",p,toRow);
		nf.ProgramPage(toRow++, pageBuffer);
		p += 256;
		nWords -= 256;
	}
	return toRow; //next row to write
}
#endif
s_int32 SaveY(register __mem_y u_int16 *p, register s_int16 nWords, register s_int32 toRow) {

	while (nWords > 0) {
		u_int16 thisWords = nWords;
		if (thisWords > 256 * CHARS_PER_WORD) {
			thisWords = 256 * CHARS_PER_WORD;
		}
		memcpyYX (pageBuffer, p, thisWords);
		//x// printf("Save from Y:%p to row %lx\n",p,toRow);
		nf.ProgramPage(toRow++, pageBuffer);
		p += thisWords / CHARS_PER_WORD;
		nWords -= thisWords;
	}
	return toRow; //next row to write
}
#if 0
s_int32 Load(u_int16 *p, s_int16 nWords, s_int32 fromRow) {
	while (nWords > 0) {
		u_int16 thisWords = nWords;
		if (thisWords > 256) thisWords = 256;
		//x// printf("Read from row %lx to X:%p\n",fromRow,p);
		nf.Read(fromRow++, 0, 512, pageBuffer);
		memcpy (p, pageBuffer, thisWords);
		p += thisWords;
		nWords -= thisWords;
	}
	return fromRow; //next row to read
}
#endif
s_int32 LoadY(register __mem_y u_int16 *p, register s_int16 nWords, register s_int32 fromRow) {
	while (nWords > 0) {
		u_int16 thisWords = nWords;
		if (thisWords > 256 * CHARS_PER_WORD){
			thisWords = 256 * CHARS_PER_WORD;
		}
		//x// printf("Read from row %lx to Y:%p\n",fromRow,p);
		nf.Read(fromRow++, 0, 512, pageBuffer);
		memcpyXY (p, pageBuffer, thisWords);
		p += thisWords / CHARS_PER_WORD;
		nWords -= thisWords;
	}
	return fromRow; //next row to read
}

void SaveState() {
	u_int32 toRow = 0;
	nf.Erase(0);
	pageBuffer[256] = sizeof(pur);
	pageBuffer[257] = sizeof(shard);
	toRow = SaveY((__mem_y void*)&pur, sizeof(pur), toRow);
	toRow = SaveY((__mem_y void*)&shard, sizeof(shard), toRow);
}

void LoadState() {
	u_int32 fromRow = 0;
	u_int32 dummyLba = nf.Read(0,0,512,pageBuffer);
	//if ((pageBuffer[256] == sizeof(pur)) && (pageBuffer[257] == sizeof(shard))) {
	if (dummyLba == (sizeof(pur) + ((u_int32)sizeof(shard)<<16))) {
		fromRow = LoadY((__mem_y void*)&pur, sizeof(pur), fromRow);
		fromRow = LoadY((__mem_y void*)&shard, sizeof(shard), fromRow);
		PrintShards();
#ifndef __VSDSP__
		printf("Loaded state from rows 0..%d\n",fromRow-1);
#else
		printf("Loaded state from rows 0..%ld\n",fromRow-1);
#endif
	} else {
#ifndef __VSDSP__
		printf("No format detected (%x)\n",dummyLba);
#else
		printf("No format detected (%lx)\n",dummyLba);
#endif
	}
}



__mem_y struct Shard *FindShard(register s_int32 lba) {
	u_int16 i;
	__mem_y struct Shard *s = &shard[0];
	//PrintShards();
	for (i=0; i<pur.shards; i++) {
		////x// printf("Find lba %lx start %lx size %x\n",lba,s->startLba,s->sectors);
		if ((s->startLba <= lba) && ((s->startLba + s->sectors) > lba)) {
			////x// printf(" :) ");
			return s;
		}
		s++;
	}
	////x// printf(" %lx:( ",lba);
	return NULL;
}

s_int32 ShardFindSector(register s_int32 lba) {
	__mem_y struct Shard *s = FindShard(lba);
	if (s) {
		////x// printf("FO ");
		return s->startSba + lba - s->startLba;
	}
	////x// printf("NOT ");
	return -1;
}


ioresult ShardAddShard(register s_int32 lba,  s_int32 sba, u_int32 n) {
	__mem_y struct Shard *s;
	if (pur.shards >= PURITY_MAX_SHARDS) {
		SysError("NAND out of shard mem, now read-only");
		devMlcFlash.flags &= ~(__MASK_WRITABLE);
		return S_ERROR;
	}
	s = &shard[pur.shards++];
	s->startLba = lba;
	s->startSba = sba;
	s->sectors = n;
	
	//PrintShards();
	return S_OK;
}

ioresult ShardAddSector(register s_int32 lba, register s_int32 sba) {
	__mem_y struct Shard *s;
	// Moved to outside function so resetting is possible when
	// shard array is sorted.
	//static __mem_y struct Shard *lastShard;
	static s_int32 lastSba;
	static s_int32 lastLba;

	#if 1 //simple optimization to continue previous shard
	if (lastShard && lastSba == sba + 1 && lastLba == lba + 1 && !FindShard(lba)) {
		lastShard->sectors++;
		lastSba = sba;
		lastLba = lba;
		return S_OK;
	} else {
		lastShard = 0;
	}
	#endif

	//Löytyykö tämä lba jo levyltä jostain?
	while(1) {
		if (!(s = FindShard(lba))) { //Ei löydy.
		//Voidaanko jatkaa olemassaolevaa?
		if ((s = FindShard(lba-1))) {
				if ((s->startLba + s->sectors == lba) && (s->startSba + s->sectors == sba)) {
					s->sectors++;
					return S_OK;
				}
			}
			//Ei voida. Tehdään uusi.
			return ShardAddShard(lba,sba,1);
		}
		//Löytyy jostain. Onko se shardin ainoa sektori?
		if (s->sectors == 1) {
			__mem_y struct Shard *s2;
			//On, voidaanko jatkaa jo olemassaolevaa?
			if ((s2 = FindShard(lba-1))) { //Kasvatetaan olemassaolevaa ja poistetaan vanha (se jossa oli yksi sektori)
				if ((s2->startLba + s2->sectors == lba) && (s2->startSba + s2->sectors == sba)) {
					s2->sectors++;
					/* FIXME: Use memmoveYY when it has been released. */
					//memmoveYY(s, &s[1], (&shard[pur.shards]-&s[1])*sizeof(s[0]));
					memcpyYY(s, &s[1], (&shard[pur.shards]-&s[1])*sizeof(s[0]));
					pur.shards--;
					return S_OK;
				}
			}
			//Ei, päivitetään vain sba
			s->startSba = sba;
			return S_OK;
		}
		//Ei, onko se shardin ensimmäinen sektori?
		if (s->startLba == lba) {
			//On, poistetaan se alusta
			s->startLba++;
			s->startSba++;
			s->sectors--;
		} else
		//Ei, onko se shardin viimeinen sektori?
		if (s->startLba + s->sectors == lba + 1) {
			//On, poistetaan se lopusta
			s->sectors--;
		} else {
			//Sektori on keskellä. Poistetaan ja splitataan.
			s_int32 n1 = lba - s->startLba;
			s_int32 n2 = s->sectors - n1 - 1;
			s->sectors = n1;
			ShardAddShard(lba+1,s->startSba+n1+1,n2);
		}
		////x// printf("Again.");
		//PrintShards();
	}
}

ioresult ReadLba(u_int32 lba, u_int16 *data) {
	__mem_y u_int16 *d;
	s_int32 sba;
	//x// printf("R%lx ",lba);

	if ((lba < realDataStart) && (lba >= fat2Start)) {
		lba -= fatSize;
	}

	d = YFindSector(lba);
	if (d) {
		//x// printf("at %p ",d);
		memcpyYX (data, d, 256 * CHARS_PER_WORD);
		return S_OK;
	}
	sba = ShardFindSector(lba);
	if (sba < 0) {
		//x// printf("z ");
		memset(data,0,256 * CHARS_PER_WORD);
		return S_OK;
	} else {
		u_int32 lsn = nf.Read((u_int32)(sba)>>3, (u_int16)(sba & 7) * SECTOR_SIZE_BYTES, 512, data);
		//x// printf("fromsba %lx ",sba);
		////x// printf("Expect:%lx, got:%lx\n",lba,lsn);
		if (nf.lastReadRSFail){
			devMlcFlash.flags &= ~(__MASK_WRITABLE);
			return S_ERROR;
		}
		if (lsn != lba) {
#ifndef __VSDSP__
			printf("Expected lba %x, got lba %x, from sba %x\n",lba,lsn,sba);
#else
			printf("Expected lba %lx, got lba %lx, from sba %lx\n",lba,lsn,sba);
#endif
			PrintShards();
			return S_ERROR;
		}
	}
	return S_OK;
}



/* set next writing SBA for page. If align is non zero, next align count erase blocks
 * are reserved for page. */
void NextWritePage(register __mem_y struct PageStruct *page, register u_int16 align){
	//printf("np: %u\n", align);
	page->addr.nextSba += (s_int32)PURITY_SECTORS_PER_PAGE;
	if (page->addr.nextSba >= page->addr.lastSba||align) {
		
		/* Next erase Block */
		page->addr.nextSba = pur.nextAvailableSba;
		if (align == 0)align = 1;
		pur.nextAvailableSba += (s_int32)nf.sectorsPerEraseBlock * align;
		/* Is FUS overrun? */
		if(page->addr.nextSba < pur.firstUsedSba && pur.nextAvailableSba >= pur.firstUsedSba){
			/* Reverse NAS to single step. */
			pur.nextAvailableSba -= (s_int32)nf.sectorsPerEraseBlock * (align -1);
			page->addr.lastSba = pur.nextAvailableSba;
			goto isItFull;
		}
		page->addr.lastSba = pur.nextAvailableSba;

		if((u_int32)pur.nextAvailableSba >= pur.usableSectors){
			/* Restart from the beginning. */
			pur.nextAvailableSba = PURITY_SURFACE_START;
			/* Limit the page to disk size */
			page->addr.lastSba = pur.usableSectors;
#ifdef __VSDSP__
			printf("tail 1. 0x%lx 0x%lx\n",
#else
			printf("tail 1. 0x%x 0x%x\n",
#endif
			       pur.firstUsedSba, pur.nextAvailableSba);
			if (pur.firstUsedSba - pur.nextAvailableSba <
			    nf.sectorsPerEraseBlock){
				PushFUS();
#ifdef __VSDSP__
				printf("tail 2. 0x%lx 0x%lx\n",
#else
				printf("tail 2. 0x%x 0x%x\n",
#endif
				       pur.firstUsedSba, pur.nextAvailableSba);
isItFull:				       
				if (pur.firstUsedSba - pur.nextAvailableSba <
				    nf.sectorsPerEraseBlock){
					SysError("Next write page: Disk Full. Set to Read-only.");
					SaveState();
					devMlcFlash.flags &= ~(__MASK_WRITABLE);
				}
			}
		}
  }
}

ioresult FlushPage(register __mem_y struct PageStruct *page){
	u_int16 i, *target = pageBuffer;
	__mem_y u_int16 *d = page->data;
	s_int32 lba = page->addr.firstLba;
	ioresult ret;
	if(page->addr.lbaPresent == 0){
		return S_OK;
	}
	//printf("f\n");

	if (page == &pur.pg[1])printf("Flushing Page 1\n");
	if(lba == 0){
		printf("LBA 0x%6lx-> 0x%6lx, present: 0x%8lx\n",
		page->addr.firstLba, page->addr.nextSba, page->addr.lbaPresent);
		InfoMapper();
		PrintShards();
	}
	for (i=0; i < PURITY_SECTORS_PER_PAGE; i++) {
		/* Is the sector in memory? */
		if ((u_int16)(page->addr.lbaPresent) & (1<<i)) {
			/*Yes. Copy it t pagebuffer*/
			memcpyYX(target, d, SECTOR_SIZE_WORDS * CHARS_PER_WORD);
		} else {
			/* No. Read it from flash to pagebuffer*/
			ret = ReadLba(lba, target);
			*(u_int32*)(&target[256]) = lba;
			if(ret != S_OK){
				printf("Fail 2");
				return ret;
			}
		}
		d += SECTOR_SIZE_WORDS;
		target += SECTOR_SIZE_WORDS;
		/* Update book keeping*/
		if (S_OK != (ret = ShardAddSector(lba, page->addr.nextSba + i))){
			printf("Fail 3");/* Disk is read only. */
			return ret;
		}
		lba++;
	}
	/* Write it to flash. */
	nf.ProgramPage(page->addr.nextSba / 8, pageBuffer);
	NextWritePage(page, 0);
	page->addr.lbaPresent = 0;

	return ret;
}

ioresult StoreLba(u_int32 lba, u_int16 *data) {
	// memcpy(pageBuffer, data, 256);
	// *(u_int32*)(&pageBuffer[256]) = lba;
	__mem_y struct PageStruct *p; // = &pur.pg[0];
	__mem_y u_int16 *d = YFindSector(lba);
	u_int16 subsector = (u_int16)lba & 7; //8 lbas per page
	ioresult ret = S_OK;
	//x// printf("  ###StoreLba %lx ",lba);

	if (!__F_WRITABLE(&devMlcFlash)){
		/* Shutdown desharder. It's useless. */
		KillDesharder();
		printf("Read only device.");
		return S_ERROR;
	}

	if (!d) {

		//x// printf("#a ");

		if (lba>=realDataStart) {
			p = &pur.pg[0];
			//x// printf("#b ");
			//ShardAddSector(lba,pur.nextSba);
			//nf.ProgramPage(pur.nextSba++, pageBuffer);
		} else {
			if (lba >= fat2Start) return S_OK;
			p = &pur.pg[1];
			//x// printf("#c ");
			////x// printf(" F%lx ",pur.nextFatSba);
			//ShardAddSector(lba,pur.nextFatSba);
			//nf.ProgramPage(pur.nextFatSba++, pageBuffer);
		}

		if (p->addr.nextSba == 0){
			/* This can happen with empty flash or (maybe) right after dfrag.*/
			NextWritePage(p, 0);
			p->addr.lbaPresent = 0;
		}

		if (p->addr.lbaPresent != 0) { //some sector(s) present
			u_int32 a = p->addr.firstLba;
			d = p->data;
			//x// printf("#d ");

			if (a != lba-subsector) { //need to flush

				if (S_OK != FlushPage(p)){
					printf("Flush fail\n");
					//return S_ERROR;
				}

			}
		}

		p->addr.firstLba = lba-subsector;
		p->addr.lbaPresent |= (1<<subsector);
		d = &p->data[subsector * SECTOR_SIZE_WORDS];
		//x// printf("tomem %p ",d);

	}
	memcpyXY (d, data, 256 * CHARS_PER_WORD);
	*(u_int32*)(&d[256]) = lba;
	
	if (lba == 0) {
//		FatDeviceInfo *di=(FatDeviceInfo *)&devMlcFlash.deviceInfo;
		/* Let StartFileSystem use nf.Read() and nf.Write()
		   RS error recovery fails aren't catched.
		 */
		ReleaseHwLocksBIP(DESHARDER_LOCK, 0,0);
		devMlcFlash.fs = StartFileSystem(&devMlcFlash,"");
		ObtainHwLocksBIP(DESHARDER_LOCK, 0,0);
		if (devMlcFlash.fs) {
			if ((data[0x20]&1) == 0) {
				//printf("Save state\n");
				SaveState();
				PrintShards();
			}
			UpdateFatBorders();
		}
		InfoMapper();
	}
	return ret;
}


void fini(void) {
	KillDesharder();
	SaveState();
	/* TODO: Clean up devmlcflash */
	vo_pdevices[PURITY_DRIVE_LETTER - 'A'] = 0;
}


void DeshardTask();
void CreateMapper() {
	/* Set some good defaults for pur.
	 * If load fails, mkfs to flash will work as it should. */
	pur.nextAvailableSba = PURITY_SURFACE_START;
	pur.firstUsedSba = PURITY_SURFACE_START;
	DevMlcFlashCreate(&devMlcFlash, NULL, 0);
	deshardControl.deshardMode = DESHARD_ALIVE | DESHARD_RUN;
	deshardControl.blockStep = nf.totalAccessibleEraseBlocks / DESHARD_MAX_WRITE_PAGES;
	deshardControl.tas = CreateTaskAndStack(
		DeshardTask, "Desharder", 0x100, 1);
}



/* Move pur.firstUsedSba forward and erase freed space.
 * Many side-effects:
 * 1) Calculate sector count.
 * 2) Decrement blocksToErase counter (Full deshard)
 */
__mem_y struct Shard *  PushFUS(){
	u_int16 i;
	__mem_y struct Shard *s = &shard[0], *sFus;
	s_int32 dist = 0x7ffffff, eRow, prevStart;
	__mem_y struct PageStruct *page;
	if(pur.shards == 0)return NULL;
	/* If shards aren't in order, sort'em! */
	/* This immplementation doesn't require sorted array.
	if (pur.disorderedShards){
		lastShard = NULL;
		pur.disorderedShards = 0;
		qsorty((void *)shard, pur.shards, sizeof(struct Shard),
		(int (*)(const void *, const void *))shardSbaComp);
	}
	*/
restart:
	deshardControl.sectors = 0;
	prevStart = 0;
	deshardControl.sortNeeded = 0;
	/* Find first shard. */
	for(i = 0; i < pur.shards; i++){
		s_int32 locDist = s->startSba - pur.firstUsedSba;
		if (locDist >= 0 && locDist < dist){
			sFus = s;
			dist = locDist;
		}
		if (s->startLba < prevStart){
			deshardControl.sortNeeded++;
		}
		prevStart = s->startLba;
		deshardControl.sectors += s->sectors;
		s++;
	}
	
	while(dist > nf.sectorsPerEraseBlock){
		/* No shards in FUS EB. Erase old FUS EB. */
		eRow = pur.firstUsedSba >> 3;
		nf.Erase(eRow);
		/* Next EB */
		pur.firstUsedSba += nf.sectorsPerEraseBlock;
		page = &(pur.pg[0]);
		/* Don't leave pagestructs' addresses behind. */
		for(i = 0; i < 2; i++){
			if ((page->addr.nextSba > pur.nextAvailableSba)||
				(page->addr.nextSba < pur.firstUsedSba && pur.firstUsedSba < pur.nextAvailableSba)){
				NextWritePage(page, 1);
				printf("Pulling page %u\n",i);
			}
			page++;
		}
		if((u_int32)pur.firstUsedSba >= pur.usableSectors){
			pur.firstUsedSba = PURITY_SURFACE_START;
			s = &shard[0];
			dist = 0x7ffffff;
			goto restart;
		}
		dist -= nf.sectorsPerEraseBlock;
	}
	if (!sFus){
		printf("Program error. Go home.\n");while(1);
	}
	return sFus;
}

void TestMapper(){
	u_int16 i;
	s_int32 lba, size;
	__mem_y struct Shard *s = &shard[0];
	for(i = 0; i < pur.shards; i++){
		lba = s->startLba;
		size = s->sectors;
		printf("Reading from LBA: 0x%6lx ", lba);
		while(size--){
			ReadLba(lba++, pageBuffer + 300);
		}
		printf("to 0x%6lx\n", lba - 1);
		s++;
	}
}

void InfoMapper(){
	/* Pur general */
	printf("Shards: %u FUS: 0x%6lx NAS: 0x%6lx RealDataStart: 0x%6lx\n",
			pur.shards, pur.firstUsedSba, pur.nextAvailableSba,realDataStart);
	/* File system */
	printf("fatStart: 0x%lx, fat2Start: 0x%lx, fatsize: 0x%lx\n", fatStart, fat2Start, fatSize);
	/* Write page chaches */
	printf("Page 0\n\tfirstLba: 0x%6lx\n\tnextSba:  0x%6lx\n\tlastSba:  0x%6lx\n\tPresent:  0x%08lx\n",
			pur.pg[0].addr.firstLba, pur.pg[0].addr.nextSba, pur.pg[0].addr.lastSba, pur.pg[0].addr.lbaPresent);
	printf("Page 1\n\tfirstLba: 0x%6lx\n\tnextSba:  0x%6lx\n\tlastSba:  0x%6lx\n\tPresent:  0x%08lx\n",
			pur.pg[1].addr.firstLba, pur.pg[1].addr.nextSba, pur.pg[1].addr.lastSba, pur.pg[1].addr.lbaPresent);
	/* Desharder */
	printf("Desharder mode: %x page multiplier: %u useless sectors: %lx\n", deshardControl.deshardMode,
			deshardControl.pagesToWrite, deshardControl.uselessSectors);
	printf("Deshard page \n\tfirstLba: 0x%6lx\n\tnextSba:  0x%6lx\n\tlastSba:  0x%6lx\n\tsectors:  0x%6lx\n",
			deshardPage.firstLba, deshardPage.nextSba, deshardPage.lastSba,deshardPage.lbaPresent);

}
void EraseDisk(){
	u_int16 i;
	KillDesharder();
	for(i = 0; i < nf.totalAccessibleEraseBlocks; i++){
		if (nf.Erase((u_int32)i * nf.pagesPerEraseBlock) != S_OK){
			printf("Erase error %lx\n",(u_int32)i * nf.pagesPerEraseBlock);
		}
	}
	memsetY(&pur, 0, sizeof(pur));
	pur.firstUsedSba = PURITY_SURFACE_START;
	pur.nextAvailableSba = PURITY_SURFACE_START;
}
void DropSectors(register s_int32 lba, register u_int32 size){
	s_int32 dummySba = nf.totalSectors + 0x800; /* Over the end*/
	u_int32 i;
	/* printf("Dropping: %06lx, %lx\n", lba, size); */
	for(i = 0; i < size; i++){
		ShardAddSector(lba++, dummySba++);
	}
	pur.shards--;
}
/* fopen is needed, here is path to use. */
const char devPath[] = {PURITY_DRIVE_LETTER, ':', '\0'};
u_int32 VoFatReadClusterRecordF32(register __i0 VO_FILE *f, u_int32 fatCluster);
void ReclaimFreeSpace(){
	FatDeviceInfo *di=(FatDeviceInfo *)&devMlcFlash.deviceInfo;
	u_int32 k, lba = di->dataStart, spc = di->fatSectorsPerCluster, kend = di->totalClusters;
	FILE *fp;
	/*
	If the disk is FAT12 or FAT16, it is so small, reclaiming
	isn't feasible. OTOH bringing support for them isn't
	justified complexity.
	This could be made faster. See vo_fat_partial.c and VoFatCountFreeClusters
	*/
	if (di->fatBits != 32 || !(fp = fopen(devPath, "s"))) return;
	DeshardPause();
	lba += 2*spc; /* datastart is 2 clusters behind as first cluster is 2. */
	for (k=2; k < kend; k++){
		if (!VoFatReadClusterRecordF32(fp,k)&& FindShard(lba)) {
			/* Cluster is free but it is found from shards so we're keeping deleted data.
			   Drop it!
			 */
			DropSectors(lba, spc);
		}
		lba += spc;
	}
	fclose(fp);
	deshardControl.deshardMode &= ~DESHARD_PAUSE;
}
void DeshardPause(){
	/* Locking isn't possible. VoFat... reads blocks so 
	   we ask politely desharded to pause
	 */
	while(deshardControl.deshardMode != DESHARD_DEAD && /* Don't hang on dead. */
	  !(deshardControl.deshardMode & DESHARD_PAUSED)){
		deshardControl.deshardMode |= DESHARD_PAUSE;
		Delay(1);
	}
}

void UpdateFatBorders(){
	FatDeviceInfo *di=(FatDeviceInfo *)&devMlcFlash.deviceInfo;
	u_int32 oldStart = realDataStart, i;
	printf("updateFatBorders\n");
	realDataStart = di->dataStart + (di->fatSectorsPerCluster * 2UL);
	if (oldStart != realDataStart){
		FlushPage(&pur.pg[0]);
		FlushPage(&pur.pg[1]);
	}
	fatStart = di->fatStart;
	if (di->fatBits == 32) {
		fatSize = (realDataStart - fatStart) / 2;
	} else {
		fatSize = (di->rootStart - fatStart) / 2;
	}
	fat2Start = fatStart + fatSize;
	if (oldStart != realDataStart){
		FlushPage(&pur.pg[0]);
		FlushPage(&pur.pg[1]);
		/* Drop the second FAT */
		DropSectors(fat2Start, fatSize);
	}
}
volatile struct DeshardTask deshardControl;

void KillDesharder(){
	/* Don't overkill */
	if(	deshardControl.deshardMode == DESHARD_DEAD)return;
	while(deshardControl.deshardMode != DESHARD_DEAD){
		deshardControl.deshardMode = DESHARD_DIE;
		Yield(); /* Let desharder die properly */
	}
	FreeTaskAndStack(deshardControl.tas);
	deshardControl.tas = NULL;
}
void DeshardTask(){
	s_int16 sleepMs = TICKS_PER_SEC;
	/* Don't start before everything is set up.*/
	Delay(100);
	while(deshardControl.deshardMode != DESHARD_DIE){
		if (pur.shards && fatStart && fat2Start && fatSize){
			sleepMs = DESHARD_MAX_WRITE_PAGES / 2 - deshardControl.pagesToWrite;
			SimpliRefresher();
			SimpliSharder();
		}
		while(deshardControl.deshardMode & DESHARD_PAUSE){
			deshardControl.deshardMode |= DESHARD_PAUSED;
			Delay(10);
		}
		deshardControl.deshardMode &= ~DESHARD_PAUSED;
		/* Sleep or Yield. */
		if (sleepMs > 0)Delay(sleepMs);
		else Yield();	
	}
	/* Tell we have died. */
	ObtainHwLocksBIP(DESHARDER_LOCK, 0,0);
	deshardControl.deshardMode = DESHARD_DEAD;
	ReleaseHwLocksBIP(DESHARDER_LOCK, 0,0);
}
