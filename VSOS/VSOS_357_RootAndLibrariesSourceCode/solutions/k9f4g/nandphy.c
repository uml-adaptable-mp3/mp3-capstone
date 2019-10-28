/* For free support for VSIDE, please visit www.vsdsp-forum.com */

// Starting point template for creating VSOS3 applications.
// This will create a <projectname>.AP3 file, which you can copy to 
// your VS1005 Developer Board and run it from there.

// AP3 files require VSOS3 kernel 0.3x to run.

// If you rename your application to INIT.AP3, then the kernel will
// load it automatically after booting.

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <vsNand.h>
#include <sysmemory.h>
#include <string.h>
#include <vo_gpio.h>

#include "k9f4g.h"
#include "nflayout.h"


extern struct FsNandPhys *ph;
extern __mem_y u_int16 oldExtClock4KHz;
extern __mem_y u_int16 oldClockX;

u_int32 NfTrA(register u_int32 sector); //This is what we need to provide
__mem_y NandFlashBadBlocks nfBad;


// This function adds size of boot area to sector and maps around bad blocks
u_int32 NfTrA(register u_int32 sector) {
	u_int16 eb = EB(sector);
	u_int16 se = SUBSEC(sector);
	u_int16 i;

	eb += NF_FIRST_DATA_ERASEBLOCK; //how many eraseblocks set aside for boot data
	
	for (i=0; i<nfBad.bads; i++) {
		if (nfBad.bad[i].from == eb) {
			eb = nfBad.bad[i].to;
		}
	}
	sector = STARTSEC(eb);
	sector |= se;
	return sector;
}


int InitNandPhysical(void) {
	
	oldExtClock4KHz = 3072;
	oldClockX = 12;
	memsetY(&nfBad,0,sizeof(nfBad));
	ph = FsPhNandCreate(0);
	// call NFBADMAP.DL3 to build a bad block translation map
	RunProgram("nfbadmap", &nfBad);

	return S_OK;
}
