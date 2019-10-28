/* For free support for VSIDE, please visit www.vsdsp-forum.com */

/// \file nandmap.c VSOS3 Device driver for a single type of SLC NAND flash
/// \author Panu-Kristian Poiksalo, VLSI Solution Oy

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <vsNand.h>
#include <stdlib.h>
#include <string.h>
#include <timers.h>
#include <clockspeed.h>
#include <iochannel.h>
#include <hwLocks.h>
#include <ctype.h>
#include <vo_fat.h>
#include <vo_gpio.h>
#include <kernel.h>

#include "k9f4g.h"
#include "nflayout.h"

#define LOCK()    ObtainHwLocksBIP(HLB_USER_0, HLIO_0_14|HLIO_0_15, HLP_NONE); //not final - todo: figure out correct lock bits
#define UNLOCK() ReleaseHwLocksBIP(HLB_USER_0, HLIO_0_14|HLIO_0_15, HLP_NONE); //not final - todo: figure out correct lock bits




// Index for canonical regions, for speedup
typedef struct {
	u_int16 fromLEB;
	u_int16 toSEB;
	u_int16 n;
} CanonicalIndex;

#define CDX_ENTRIES 50
__mem_y CanonicalIndex cdx[CDX_ENTRIES];
u_int16 ncdx = 0;


extern struct FsNandPhys *ph;
u_int32 NfTrA(register u_int32 sector);

// Optimization parameters - these affect the driver's memory consumption and efficiency
#define JOURNAL_BLOCKS 5 //Number of simultaneous journal blocks in use

// Helper constants - do not modify
#define NF_EBMAPSIZE (NF_EBSIZE_SECTORS/16)
//#define NF_IDATA_EB (NF_FIRST_EBB + NF_ERASEBLOCKS + NF_JOURNAL_ERASEBLOCKS)
#define NOT_EMPTY -1
#define NF_LOGICALSIZE_SECTORS (NF_EBSIZE_SECTORS * NF_ERASEBLOCKS)

// Helper macros - do not modify
#define NF_READ_SSM(s,d,m) (ph->p.Read(&ph->p, (NfTrA(s)), 1, (d), (u_int16*)(m)))
#define NF_WRITE_SSM(s,d,m) (ph->p.Write(&ph->p, (NfTrA(s)), 1, (d), (u_int16*)(m)))
#define ENDSEC(e) (STARTSEC(e)+(NF_EBSIZE_SECTORS - 1))
#define NF_ERASE_SEBM(b) (ph->p.Erase(&ph->p, NfTrA(STARTSEC(b))))


void NF_READ_SS(u_int32 ssn, u_int16 *data, void *meta) {
	LOCK();
	NF_READ_SSM(ssn,data,meta);
	UNLOCK();
}

void NF_WRITE_SS(u_int32 ssn, u_int16 *data, void *meta) {
	volatile u_int16 attempts;
	static u_int16 vbuf[256];
	StdMeta vmeta;
	u_int16 i;
	
	attempts=0;
	tryagain:
	if (attempts > 6) {
		fprintf(vo_stderr,"PERMANENT FAILURE(%d); QUITTING\n",attempts);
		return;
	}
	attempts++;
	
	((StdMeta*)meta)->flags = 0xffffu;
	//if (EB(ssn) > highestKnownSEB) highestKnownSEB = EB(ssn);
	LOCK();
	i=NF_WRITE_SSM(ssn,data,meta);
	UNLOCK();
	//printf("W%d ",i);		
	if (i!=1) {
		printf("WRITE ERROR %d AT SSN %ld\n",i,ssn);
	}
	NF_READ_SS(ssn,vbuf,&vmeta);
	if (((StdMeta*)meta)->lsn != vmeta.lsn) {
		fprintf(stderr," META ERR SSN %lx ",ssn);
		goto tryagain;
	}
	for (i=0; i<256; i++) {
		if (data[i] != vbuf[i]) { fprintf(stderr,"DATA ERR SSN %lx ",ssn); goto tryagain; }
	}

}

void NF_ERASE_SEB(u_int16 seb) {
	ncdx = 0; //Invalidate the canonical index
	LOCK();
	NF_ERASE_SEBM(seb);
	UNLOCK();
}

typedef struct DevNandFlashHardwareInfo {
  u_int32 totalBlocks;
} devNandFlashHwInfo;


TranslateS16 cc[1];
//#define CLEARCC() (memset(cc,0xffff,sizeof(cc)))
#define CLEARCC() (cc[0].from = 0xffff)

u_int16 g_data[256];
StdMeta g_meta;

	
typedef struct {
	u_int16 fromLEB;
	u_int16 toSEB;
	u_int16 nextFreeSector;
	u_int16 inOrder;
	u_int16 bitmap[NF_EBMAPSIZE];
} JournalBlock;

typedef struct {
	struct FsNandPhys *ph;
	u_int16 magic;
	u_int16 ebsize_sectors;
	u_int16 ebshift;
	u_int16 pagesize_sectors;
	u_int16 total_eraseblocks;
	u_int16 data_eraseblocks;
	u_int16 journal_eraseblocks;
	u_int16 first_data_eraseblock;
	u_int16 reserved_area_start;
	u_int16 max_bad_blocks;
	u_int16 tag1;
	u_int16 tag2;	
	u_int16 journals;
	JournalBlock journal[JOURNAL_BLOCKS];			
} NandFlash;
__mem_y NandFlash nf = {
	(void*)1075, 4242, NF_EBSIZE_SECTORS, NF_EBSHIFT, NF_PAGESIZE_SECTORS, NF_TOTAL_ERASE_BLOCKS,
	NF_ERASEBLOCKS, NF_JOURNAL_ERASEBLOCKS, NF_FIRST_DATA_ERASEBLOCK, NF_RESERVED_AREA_START,
	NF_MAX_BAD_BLOCKS,
};

__mem_y u_int16 lebBitmap[(NF_ERASEBLOCKS+15)/16];

ioresult DevNandFlashCreate (register __i0 DEVICE *dev, void *name, u_int16 extraInfo);
ioresult DevNandFlashInput(register __i0 DEVICE *dev, u_int16 port, void *buf, u_int16 bytes);
ioresult DevNandFlashOutput(register __i0 DEVICE *dev, u_int16 port, void *buf, u_int16 bytes);	
ioresult DevNandFlashBlockRead(register __i0 DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data);
ioresult DevNandFlashBlockWrite(register __i0 DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data);
ioresult DevNandFlashDelete(register __i0 DEVICE *dev);
char* DevNandFlashIdentify(register __i0 void *obj, char *buf, u_int16 bufsize);
IOCTL_RESULT DevNandFlashIoctl(register __i0 DEVICE *dev, s_int16 request, IOCTL_ARGUMENT arg);



const DEVICE __mem_y devNandFlashDefaults = {   
  0, // u_int16 flags; //< present, block/char
  DevNandFlashIdentify, //char*    (*Identify)(void *obj, char *buf, u_int16 bufsize);   
  DevNandFlashCreate, //ioresult (*Create)(DEVICE *dev, char *name, struct filesystem_descriptor *fs);
  DevNandFlashDelete, //ioresult (*Delete)(DEVICE *dev);
  DevNandFlashIoctl, //ioresult (*Ioctl)(DEVICE *dev, s_int16 request, s_int32 arg); //Start, Stop, Flush, Check Media
  // Stream operations
  0, //u_int16  (*Read) (register __i0 DEVICE *self, void *buf, u_int16 destinationIndex, u_int16 bytes); ///< Read bytes to *buf, at byte index
  0, //u_int16  (*Write)(register __i0 DEVICE *self, void *buf, u_int16 sourceIndex, u_int16 bytes); ///< Write bytes from *buf at byte index
  // Block device operations
  DevNandFlashBlockRead, //ioresult (*BlockRead)(DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data);
  DevNandFlashBlockWrite, //ioresult (*BlockWrite)(DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data);
  // Stream operations
  //DevNandFlashInput, //ioresult (*Input)(DEVICE *dev, u_int16 port, void *buf, u_int16 bytes);
  //DevNandFlashOutput, //ioresult (*Output)(DEVICE *dev, u_int16 port, void *buf, u_int16 bytes);
  //FILESYSTEM *fs;
  //DIRECTORY *root; //Zero if the device cannot hold directories
  //u_int16  deviceInstance;
  //u_int16  hardwareInfo[6]; ///< Device driver's info of this hardware
  //u_int16  deviceInfo[10]; // For filesystem use, size TBD   
};
void BlockRead(register __i0 DEVICE *phy, register u_int32 firstBlock, u_int16 *data);
void MergeBlock(u_int16 i);
 
#define END_FRAME 1
#define DONT_END_FRAME 0


__mem_y JournalBlock *FindJournal(u_int16 logicalEB) {
	u_int16 i;
	for (i=0; i<nf.journals; i++) {
		if (nf.journal[i].fromLEB == logicalEB) {
			return &nf.journal[i];	
		}
	}
	return NULL;
}

u_int16 GetFromJournal(register u_int32 lsn, u_int16 *data) {
	u_int16 i;
	__mem_y JournalBlock *j = FindJournal(EB(lsn));
	u_int16 se = SUBSEC(lsn);
	u_int32 ssn;
	static StdMeta meta;
	if (!j) { return 0; } //No journal for this lsn
	if (!(j->bitmap[se>>4] & (1 << (se&0xf)))) { return 0; } //Non-journaled subsector according to bitmap
	
	ssn = STARTSEC(j->toSEB);
	if (j->inOrder) { //Journal in order, read from canonical location
		NF_READ_SS(ssn + se, data, &meta);
		return 1;
	} else { //Journal not in order, read from end to beginning to find the lsn
		ssn += j->nextFreeSector-1;
		for (i=0; i<j->nextFreeSector; i++) {
			NF_READ_SS(ssn, data, &meta);			
			if (meta.lsn == lsn) {
				return (ssn & NF_EBSIZE_SECTORS-1)+1; //LSN match in meta, found.
			}
			ssn--;
		}		
	}
	return 0; //Not found in journal
}

s_int16 FindCanonicalEB(u_int16 leb) {
	u_int16 i;
	static u_int16 cand = 0;
	__mem_y JournalBlock *j;
	StdMeta meta;
	
	for (i=0; i<ncdx; i++) {
		if ((leb >= cdx[i].fromLEB) && ((cdx[i].fromLEB + cdx[i].n) > leb)) {
			cand = (leb-cdx[i].fromLEB) + cdx[i].toSEB;
		}
	}
	

	if (leb >= NF_ERASEBLOCKS) return S_ERROR;
	if ((lebBitmap[leb >> 4] & (1 << (leb&0xf))) == 0) return S_ERROR; //Nonexistent leb according to lebBitmap
	
	if (cc[0].from == leb) return cc[0].to; 	
	cc[0].from = leb;
	cc[0].to = S_ERROR;
	j = FindJournal(leb);

	for (i=0; i<NF_ERASEBLOCKS+NF_JOURNAL_ERASEBLOCKS; i++) {
		if (j && j->toSEB==cand) {
			cand++; i++; //it's a journal, not canonical, skip
		}
		NF_READ_SS(STARTSEC(cand), g_data, &meta);
		if (EB(meta.lsn) == leb) {
			cc[0].to = cand;
			return cand;
		}
		cand++;
		if (cand >= NF_ERASEBLOCKS+NF_JOURNAL_ERASEBLOCKS) cand = 0;
	}
	return S_ERROR; //-1 == Not found
}


#define SET_LEB_BITMAPS(e) (lebBitmap[(e) >> 4] |= (1 << ((e)&0xf)))
void SET_LEB_BITMAP(u_int16 eb) {
	SET_LEB_BITMAPS(eb);
}




u_int16 FindGetCanonicalSector(u_int32 lsn, u_int16 *data) {
	u_int16 leb = EB(lsn);
	s_int16 seb = FindCanonicalEB(leb);
	StdMeta meta;
	if (seb == S_ERROR) return 0;
	NF_READ_SS(STARTSEC(seb) + SUBSEC(lsn), data, &meta);
	return 1;
}


u_int16 BlockIsEmpty(u_int16 seb) {
	NF_READ_SS(STARTSEC(seb),g_data,&g_meta);
	if (g_meta.lsn == 0xffffffffu) return 1;
	return 0;	
}




u_int16 GetNewBlock() {
	static u_int16 nexteb = 0;
	do {
		nexteb++;
		if (nexteb >= NF_ERASEBLOCKS + NF_JOURNAL_ERASEBLOCKS) {
			nexteb = 0;
		}
	}
	while (!BlockIsEmpty(nexteb));
	return nexteb;
}




void NandBlockRead(register u_int32 firstBlock, u_int16 *data);
void MergeJournal(__mem_y JournalBlock *j);


ioresult DevNandFlashCreate(register __i0 DEVICE *dev, void *name, u_int16 extraInfo) {
	devNandFlashHwInfo *hw = &(dev->hardwareInfo);
	memcpyYX(dev, &devNandFlashDefaults, sizeof(DEVICE));
	hw->totalBlocks = (u_int32)NF_ERASEBLOCKS * NF_EBSIZE_SECTORS;
	dev->deviceInstance = __nextDeviceInstance++;
	dev->flags = __MASK_PRESENT | __MASK_OPEN | __MASK_SEEKABLE | __MASK_READABLE | __MASK_WRITABLE;	
	return dev->Ioctl(dev, IOCTL_RESTART, 0);
	
}

ioresult DevNandFlashDelete(register __i0 DEVICE *dev) {
  return S_OK;
}

const char *DevNandFlashIdentify(register __i0 void *dev, char *buf, u_int16 bufsize) {
	return "Nand Flash";
}


void NandBlockRead(register u_int32 lsn, u_int16 *data) {	
	if (GetFromJournal(lsn, data)) return;
	if (FindGetCanonicalSector(lsn, data)) return;
	memset(data,0,256);
}

__mem_y JournalBlock *NewJournal (u_int16 leb) {
	__mem_y JournalBlock *j;
	if (nf.journals >= JOURNAL_BLOCKS) {
		MergeJournal(&nf.journal[1]);
	}
	j = &nf.journal[nf.journals++];
	memsetY (j,0,sizeof(JournalBlock));	
	j->fromLEB = leb;
	j->toSEB = GetNewBlock();
	j->inOrder = 1;
	return j;	
}


void DropJournal(__mem_y JournalBlock *j) {
	memcpyYY(j, j+1, (&nf.journal[JOURNAL_BLOCKS-1]-j)*sizeof(*j));
	nf.journals--;
}


void MergeJournal(__mem_y JournalBlock *j) {
	//Journal block finalized
	u_int16 ceb = FindCanonicalEB(j->fromLEB);
	StdMeta meta;
	CLEARCC();
	//fprintf(stderr,"Merge %d->SEB %d\n",j->fromLEB,j->toSEB);
	if ((!j->inOrder) || (j->nextFreeSector < NF_EBSIZE_SECTORS)) {
		// Not in order. Build new canonical to EB(tsn)
		u_int32 lsn = STARTSEC(j->fromLEB);    	//Logical Sector Number
		u_int32 csn = STARTSEC(ceb);			//Canonical Sector Number
		u_int32 tsn = STARTSEC(GetNewBlock());	//Target Sector Number
		u_int16 i;
		static u_int16 j_data[256];
		for (i=0; i<NF_EBSIZE_SECTORS; i++) {
			meta.lsn = lsn;
			memset(g_data,0,sizeof(g_data)); //1. fill with zeros
			if (ceb != S_ERROR) { //There is a canonical block for this lsn block
				NF_READ_SS(csn, g_data, &meta); //2. Get Canonical if it exists
			} 
			if (GetFromJournal(lsn, j_data)) {
				memcpy(g_data,j_data,256);
			} //3. Get from journal if it exists
			meta.lsn = lsn;
			NF_WRITE_SS(tsn, g_data, &meta); //4. write to tsn.			
			tsn++;
			lsn++;
			csn++;
		}
		NF_ERASE_SEB(j->toSEB); //erase Journal Block
	}
	SET_LEB_BITMAP(j->fromLEB); //fromLEB has become canonical, set bitmap so FindCanonicalEB will search for it
	DropJournal(j);
	if (ceb != S_ERROR) {
		NF_ERASE_SEB(ceb);
		cc[0].from = S_ERROR;
	}
	CLEARCC();	
}

void NandBlockWrite(register u_int32 lsn, u_int16 *data) {
	StdMeta meta;
	u_int16 eb = EB(lsn);
	u_int32 ssn;
	__mem_y JournalBlock *j = FindJournal(eb);	
	if (!j) j = NewJournal (eb);
	ssn = STARTSEC(j->toSEB) + j->nextFreeSector++;
	meta.flags = 0xffffu;
	meta.lsn = lsn;
	NF_WRITE_SS(ssn, data, &meta);
	j->bitmap[(SUBSEC(lsn))>>4] |= (1 << (lsn & 0xf)); 
	if (SUBSEC(lsn) != SUBSEC(ssn)) {
		j->inOrder = 0;
	}
			
	//This may have finalized the journal block, let's see.
	if (j->nextFreeSector == NF_EBSIZE_SECTORS) {
		MergeJournal(j);
	}
}



ioresult DevNandFlashBlockRead(register __i0 DEVICE *dev, u_int32 firstBlock, u_int16 blocks, u_int16 *data){
	//ObtainHwLocksBIP(HLB_USER_0, HLIO_0_14|HLIO_0_15, HLP_NONE); //not final - todo: figure out correct lock bits
	while(blocks--) {
		NandBlockRead(firstBlock, data);
		firstBlock++;
		data += 256;
	}
	//ReleaseHwLocksBIP(HLB_USER_0, HLIO_0_14|HLIO_0_15, HLP_NONE); //not final - todo: figure out correct lock bits
	return S_OK;
}


ioresult DevNandFlashBlockWrite(register __i0 DEVICE *dev,  u_int32 firstBlock, u_int16 blocks, u_int16 *data){
	//ObtainHwLocksBIP(HLB_USER_0, HLIO_0_14|HLIO_0_15, HLP_NONE); //not final - todo: figure out correct lock bits
	while(blocks--) {
		NandBlockWrite(firstBlock, data);
		firstBlock++;
		data += 256;
	}
	//ReleaseHwLocksBIP(HLB_USER_0, HLIO_0_14|HLIO_0_15, HLP_NONE); //not final - todo: figure out correct lock bits
	return S_OK;
}


IOCTL_RESULT DevNandFlashIoctl(register __i0 DEVICE *dev, s_int16 request, IOCTL_ARGUMENT arg) {
	ioresult errCode = S_OK;
	devNandFlashHwInfo *hw = &(dev->hardwareInfo);
	//printf("DevNandFlashIoCtl (%d) kutsuttu\n",request);
	switch (request) {
		case IOCTL_RESTART: {
			dev->fs = StartFileSystem(dev, "0");
			if (!dev->fs) {
				dev->flags = __MASK_PRESENT;
				return S_ERROR;
			}
			break;
		}
		case IOCTL_GET_GEOMETRY:
			if (arg) {
				((DiskGeometry*)arg)->sectorsPerBlock = 1;
				((DiskGeometry*)arg)->totalSectors = (hw->totalBlocks);
			}
			return hw->totalBlocks; //obsolete
			break;
		default: {
			errCode = S_ERROR;
			break;
		}
	}
	return errCode;
}

void NandSpeedup() {
	s_int16 i=-1;
	u_int16 eb;
	u_int16 leb;
	u_int16 thisLEB=0;
	u_int16 thisSEB=0;
	u_int16 n;
	
	printf("Cleaning Nand..\n");
	while (nf.journals) MergeJournal(&nf.journal[0]);
		
	//build index
	i=0;
	for (eb=0; eb<NF_ERASEBLOCKS + NF_JOURNAL_ERASEBLOCKS; eb++) {
		NF_READ_SS(STARTSEC(eb), g_data, &g_meta);
		leb = EB(g_meta.lsn);		
		if (leb < NF_TOTAL_ERASE_BLOCKS) {
			//fprintf(stderr,"%d:%d ",eb,leb);
			if (i<0) {
				i=0;
				thisLEB = leb;
				thisSEB = eb;
				n=0;
			}
			if ((leb == thisLEB+n) && (eb == thisSEB+n)) {
				n++;
			} else {
				cdx[i].fromLEB = thisLEB;
				cdx[i].toSEB = thisSEB;
				cdx[i].n = n;
				i++;
				ncdx = i;
				if (i >= CDX_ENTRIES) {
					return;
				}
				initial:
				thisLEB = leb;
				thisSEB = eb;
				n=1;
			}
		}
	}
	cdx[i].fromLEB = thisLEB;
	cdx[i].toSEB = thisSEB;
	cdx[i].n = n;
	i++;
	ncdx = i;

	/*
	for (i=0; i<ncdx; i++) {
		fprintf(stderr,"%02d: %d->%d,%d\n",i,cdx[i].fromLEB,cdx[i].toSEB,cdx[i].n);
	}
	*/
}



void ScanFlash () {
	u_int16 eb;
	u_int16 i;
	//nf.journals = 0;
	//memsetY (nf.journal[0].bitmap, 0xffffu, sizeof(nf.journal[0].bitmap));
	for (eb=0; eb<NF_ERASEBLOCKS + NF_JOURNAL_ERASEBLOCKS; eb++) {
		u_int16 leb;
		NF_READ_SS(STARTSEC(eb), g_data, &g_meta);
		leb = EB(g_meta.lsn);
		if (leb < NF_ERASEBLOCKS) {
			SET_LEB_BITMAP(leb);
			//highestKnownSEB = eb;
		}
		
		if ((nf.journals < JOURNAL_BLOCKS) && (g_meta.lsn != 0xffffffffu)) { //First sector is programmed -> may be journal or canonical
			nf.journal[nf.journals].fromLEB = leb; //in case it turns out to be a journal
			nf.journal[nf.journals].inOrder = 1;
			NF_READ_SS(ENDSEC(eb), g_data, &g_meta);
			if (g_meta.lsn == 0xffffffffu) { //Last sector is not programmed -> it is a journal block
				u_int16 i;
				nf.journal[nf.journals].toSEB = eb;
				for (i=0; i<NF_EBSIZE_SECTORS; i++) {
					NF_READ_SS(STARTSEC(eb)+i, g_data, &g_meta);
					if (g_meta.lsn != 0xffffffffu) {
						u_int16 sec = SUBSEC(g_meta.lsn);
						nf.journal[nf.journals].nextFreeSector = i+1;
						nf.journal[nf.journals].bitmap[sec>>4] |= (1 << (sec & 0xf));
						if (sec != i) nf.journal[nf.journals].inOrder = 0; 
					}
				}
				//memsetY(nf.journal[nf.journals].bitmap, 0xffff, sizeof(nf.journal[nf.journals].bitmap));
				nf.journals++;
			}
		}
	}
	
	for (i=0; i<nf.journals; i++) {
		fprintf(stderr,"JB[%x->%x] ",nf.journal[i].toSEB,nf.journal[i].fromLEB);
	}

}


int InitNandPhysical(void);
ioresult main(char *parameters) {
	u_int16 i;
	u_int16 blk;
	static DEVICE nand;

	AddSymbol("_nf",0,(u_int16)(&nf));	
	memsetY (lebBitmap,0,sizeof(lebBitmap));
	CLEARCC();


	InitNandPhysical();
	ScanFlash();
	NandSpeedup();
	
			 
	if (S_OK == DevNandFlashCreate(&nand, "", 0)) {		
		printf("Nand: Ok.\n");
	} else {
		printf("No Nand or FS.\n");
	}
	vo_pdevices[(parameters[0]|0x20) - 'a'] = &nand;
	printf("%c: %s, handled by %s\n",parameters[0],nand.Identify(&nand,NULL,0),nand.fs ? nand.fs->Identify(nand.fs, NULL, 0):"None");

	return S_ERROR;
}

