/* For free support for VSIDE, please visit www.vsdsp-forum.com */

// Starting point template for creating VSOS3 libraries and device drivers.
// This will create a <projectname>.DL3 file, which you can copy to 
// your VS1005 Developer Board's system disk's SYS subdirectory.

// If init(), main() or fini() are not needed, remove them from the solution.
// There's no need to have any unneeded functions in the library.

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <vsNand.h>
#include <string.h>
#include <timers.h>
#include <vo_gpio.h>
#include <kernel.h>

#include "nflayout.h"
#define __NF_ALE_PIN 0x0f
#define __NF_CLE_PIN 0x0e


#ifdef printf
#undef printf
#endif
#define printf please_use_fprintf_to_stderr_instead

#define MAGIC_FDFE 0xfdfe

extern struct FsNandPhys *ph;


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
} NandFlash;

DLLIMPORT(nf) extern __mem_y NandFlash nf;
__mem_y NandFlashBadBlocks *b;


#define EB(a) ((a)>>nf.ebshift)
#define STARTSEC(a) ((u_int32)(a)<<nf.ebshift)

u_int16 nextReplacementBlock;
StdMeta meta;
u_int16 data[256];



u_int16 NewReplacementBlock() {
	while(1) {
		nextReplacementBlock++;
		ph->p.Read(&ph->p, STARTSEC(nextReplacementBlock), 1, data, (void*)&meta);
		if (meta.flags == 0xffff) {
			return nextReplacementBlock;		
		}
		if (nextReplacementBlock == nf.reserved_area_start + nf.max_bad_blocks + 2) {
			fprintf(stderr, "\nCannot format - too many bad blocks.\n");
			while(1);
		}
	}
}

void CreateRBATable() {
	u_int16 i;
	u_int16 n;

	PERIP(GPIO0_MODE) |= 0x07ff; //Set GPIO0 pins [10:0] as peripherals	
	GpioSetPin(__NF_ALE_PIN, 0); //Set ALE pin as output, pull low
	GpioSetPin(__NF_CLE_PIN, 0); //Set CLE pin as output, pull low	
	
	fprintf(stderr,"Creating initial bad block table...\n");
	fprintf(stderr,"nf:%p, ph:%p, magic:%d\n",&nf,&ph,nf.magic);
	fprintf(stderr,"Eraseblocks: %d\n",nf.data_eraseblocks);
	fprintf(stderr,"Reserved area start: %d\n",nf.reserved_area_start);
	nextReplacementBlock = nf.reserved_area_start+1;

	b->FDFE_marker = MAGIC_FDFE;
	n = 0;
	for (i=1; i<nf.reserved_area_start; i++) {
		ph->p.Read(&ph->p, STARTSEC(i), 1, data, (void*)&meta);
								
		if (meta.flags != 0xffffu) {
			fprintf(stderr,"\nBad eb %d ",i);
			b->bad[n].from = i;
			b->bad[n].to = NewReplacementBlock();
			fprintf(stderr," mapped to %d ",b->bad[n].to);
			n++;
		}
	}
	b->bads = n;
	fprintf(stderr,"\n%d bad block(s).\n",b->bads);

	memset(&meta,0xffff,sizeof(meta));
	memset(data,0xffff,sizeof(data));
	memcpyYX(data, b, sizeof(NandFlashBadBlocks));
	i = nf.reserved_area_start;
	ph->p.Erase(&ph->p, STARTSEC(i));
	ph->p.Write(&ph->p, STARTSEC(i), 1, data, (void*)&meta);
	i++;
	ph->p.Erase(&ph->p, STARTSEC(i));
	ph->p.Write(&ph->p, STARTSEC(i), 1, data, (void*)&meta);

}


u_int16 ReadRBATable() {
	u_int16 i = nf.reserved_area_start;
	fprintf(stderr,"\nRead NF RBA table from %d... ",i);
	ph->p.Read(&ph->p, STARTSEC(i), 1, data, (void*)&meta);
	if (data[0] == MAGIC_FDFE) {
		memcpyXY(b, data, sizeof(NandFlashBadBlocks));
		goto success;
	}
	i++;
	ph->p.Read(&ph->p, STARTSEC(i), 1, data, (void*)&meta);
	if (data[0] == MAGIC_FDFE) {
		memcpyXY(b, data, sizeof(NandFlashBadBlocks));
		goto success;
	}
	fprintf(stderr,"Failed. Maybe you need to low level format the nand flash.\n");	
	return S_ERROR;
	
	success:
	fprintf(stderr,"Ok, %d bad block(s).\n",b->bads);
	return S_OK;
}

void EraseAll() {
	u_int16 i;
	fprintf(stderr,"\nErasing %d blocks...\n",ph->p.eraseBlocks-1);
	for (i=1; i<ph->p.eraseBlocks; i++) {
		u_int16 r = ph->p.Erase(&ph->p, STARTSEC(i));
		if (r) {
			fprintf(stderr,"Block %d reports erase failure.\n",i);
		}
	}
	fprintf(stderr,"Done.\n");
}

void Erasure() {
	u_int16 i;

	fprintf(stderr, "\n\nS4 pressed. Do you want to low level format the NAND?\n [S1: Yes, S4: No]");
	while (GpioReadPin(0x03)) {
		//Wait
	}
	Delay(100);
	while(1) {
		if (GpioReadPin(0x03)) return;
		if (GpioReadPin(0x00)) {
			EraseAll();
			CreateRBATable();
			return;			
		}
	}
}


ioresult main(__mem_y NandFlashBadBlocks *p) {
	b = p;
	

	fprintf(stderr,"\nNand ph:\n");
	fprintf(stderr,"Eraseblocks: %d\n",ph->p.eraseBlocks);
	fprintf(stderr,"Eraseblock size: %d\n",ph->p.eraseBlockSize);
	fprintf(stderr,"Page size: %d\n",ph->p.pageSize);
	fprintf(stderr,"Nand type: %d\n",ph->nandType);
	
	

	if (ph->p.eraseBlocks == 0) {
		fprintf(stderr,"Physical says that the nand flash has 0 eraseblocks. It may be bricked.\n");
		while(1);
	}
	
	if (ph->p.eraseBlocks != nf.total_eraseblocks) {
		fprintf(stderr,"Your board may have a different nand flash IC installed than is required by this driver.\n");
		fprintf(stderr,"Physical says that there are %d eraseblocks.\n",ph->p.eraseBlocks);
		fprintf(stderr,"NFLayout says that there are %d eraseblocks.\n",nf.total_eraseblocks);
		while(1);
	}
	
	if ((ph->p.eraseBlockSize != nf.ebsize_sectors)) {
		fprintf(stderr,"NF layout does not agree with ph - it would be dangerous to continue.\n");
		fprintf(stderr,"Please verify your nand flash layout parameters.\n");
		while(1);
	}
	
	if (GpioReadPin(0x03)) {
		Erasure();
	}
	if (S_OK == ReadRBATable()) {
		return S_OK;
	}
	if (nf.magic = 4242) {
		CreateRBATable();
		return S_OK;
	}
	return S_ERROR;
}
