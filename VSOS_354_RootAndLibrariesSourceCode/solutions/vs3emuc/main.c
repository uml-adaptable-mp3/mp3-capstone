/* For free support for VSIDE, please visit www.vsdsp-forum.com */

// Starting point template for creating VSOS3 libraries and device drivers.
// This will create a <projectname>.DL3 file, which you can copy to 
// your VS1005 Developer Board's system disk's SYS subdirectory.

// If init(), main() or fini() are not needed, remove them from the solution.
// There's no need to have any unneeded functions in the library.

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <kernel.h>		// Kernel symbols
#include <consolestate.h> // appFlags etc
#include <imem.h>


// Original contents of int vectors after bootload - these should all point to ROM
__mem_y const u_int32 defaultIntVector[2][32] = {
  {
    0x2a24e14e, 0x2a202a4e, 0x2a2026ce, 0x2a2026ce,
    0x2a2026ce, 0x2a2026ce, 0x2a2026ce, 0x2a2026ce, 
    0x2a2026ce, 0x2a2026ce, 0x2a2026ce, 0x2a2026ce,
    0x2a2026ce, 0x2a2026ce, 0x2a24f14e, 0x2a00834e, 
    0x2a2026ce, 0x2a2026ce, 0x2a2026ce, 0x2a2026ce,
    0x2a2026ce, 0x2a2026ce, 0x2a2026ce, 0x2a2026ce, 
    0x2a2026ce, 0x2a2026ce, 0x2a2026ce, 0x2a2026ce,
    0x2a2026ce, 0x2a2026ce, 0x2a2026ce, 0x2a2026ce,
  },{
    0x2a26988e, 0x2a2026ce, 0x2a2026ce, 0x2a2026ce,
    0x2a2026ce, 0x2a2026ce, 0x2a2026ce, 0x2a2026ce,
    0x2a2026ce, 0x2a2026ce, 0x2a2026ce, 0x2a2026ce,
    0x2a2026ce, 0x2a2026ce, 0x2a23524e, 0x2a2026ce,
    0x2a2026ce, 0x2a2026ce, 0x2a2026ce, 0x2a2026ce,
    0x2a2026ce, 0x2a2026ce, 0x2a2026ce, 0x2a2026ce,
    0x2a2026ce, 0x2a2026ce, 0x2a2026ce, 0x2a2026ce,
    0x2a2026ce, 0x2a2026ce, 0x2a2026ce, 0x2a2026ce,
  }
};

auto void DisableIntVector(register u_int16 addr) {
  if ((addr-=0x20U) < 0x20U) { /* Is range 0x20 - 0x3f? */
    u_int16 intBase = INT_ENABLE0_LP + (addr>>4); /* addr = 32-47 -> INT_ENABLE0_LP, addr = 48-63 ->- INT_ENABLE1_LP */
    u_int16 intMask = ~(1U<<(addr&15));
    PERIP(intBase) &= intMask;
    intBase += INT_ENABLE0_HP - INT_ENABLE0_LP;
    PERIP(intBase) &= intMask;
  }
}

ioresult main(char *parameters) {
	u_int16 i;
	u_int16 version = 0;
	if (ReadIMem(0xFFFFU) == ROM_ID_VS1005H) {
	  version = 1;
	}
	printf("VS1005%c ready for vs3emu connection. Bye!\n",
	       'g'+version);
	Delay(10);
	for (i=0; i<0x20; i++) {
		DisableIntVector(i+0x20);
		WriteIMem(i+0x20, defaultIntVector[version][i]);
	}
	PERIP(INT_ENABLE0_LP) = INTF_UART_RX;
	Enable();
	while(1) {
		// UART communicates with the ROM monitor.
	}	
}

