/* For free support for VSIDE, please visit www.vsdsp-forum.com */

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <consolestate.h>
#include <kernel.h>
#include "compact.h"
#include "unimap.h"
//#include "ycache.h"

ioresult main(char *parameters) {
	
	/* Info */
	if (parameters[0] == 'i'){
		InfoMapper();
		PrintShards();
	}
	/* Continue desharding. */
	if (parameters[0] == 'p'){
		deshardControl.deshardMode &= ~DESHARD_PAUSE;
	}
	/* Pause desharding. */
	if (parameters[0] == 'P'){
		DeshardPause();
	}
	/* Deshard and stare at it. */
	if (parameters[0] == 'D'){
		u_int16 prevShards = pur.shards, prevPages = deshardControl.pagesToWrite;
		deshardControl.deshardMode |= DESHARD_RUN; /* Remove pause */
		while(!(APP_FLAG_QUIT & appFlags)){
			if (prevShards != pur.shards||prevPages != deshardControl.pagesToWrite){
				prevPages = deshardControl.pagesToWrite;
				printf("Shards: %u Pg/wr: %u\n", pur.shards, prevPages);
				prevShards =  pur.shards;
			}
			Yield();
		}
	}
	/* Save */
	if (parameters[0] == 's'){
		SaveState();
	}
	/* Test */
	if (parameters[0] == 't'){
		TestMapper();
	}
	/* Zap */
	if (parameters[0] == 'Z'){
		EraseDisk();
	}
	if (parameters[0] == 'c'){
		ReclaimFreeSpace();
	}
	return S_OK;
}

void init(void) {
	u_int16 i=0;
	u_int16 *self;
	s_int32 start = 0;
	//InitYCache();
	stdout = stderr;
	CreateMlcFlash(&nf);
	if (nf.pageSizeBytes != 4096) {
		printf("Error: NAND page size is %d, but this driver is for page size 4096.\n",nf.pageSizeBytes);
		return;		
	}
	pur.usableSectors = (u_int32)nf.sectorsPerEraseBlock * nf.totalAccessibleEraseBlocks;
	while(1) { //Find our own library reference
		self = loadedLib[i++]; //for each loaded library
		if (self[2] == (u_int16)main) break; //if library main entry is our main function, then break.
	}
	self[1]++; //increase our reference count so we are not dropped from memory even if we are started from the commmand line
	
	CreateMapper();
	vo_pdevices[PURITY_DRIVE_LETTER - 'A'] = &devMlcFlash;
	/* Now is good and only time to drop the unused blocks. */
	ReclaimFreeSpace();
}





