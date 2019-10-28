#include <vo_stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <apploader.h>
#include <vstypes.h>
#include <vs1005h.h>
#include <clockspeed.h>
#include <audio.h>
#include <timers.h>
#include <dsplib.h>
#include <crc32.h>
#include <kernel.h>
#include <cyclic.h>
#include <extSymbols.h>

/* Make this a unique 16-bit library identifier */
#define LIB_ID 0x2c91

u_int16 counter = 0;
void MyCyclicFunc(register struct CyclicNode *cyclicNode) {
  counter++;
}
struct CyclicNode myCyclicNode = {{0}, MyCyclicFunc};

/* This driver runs a cyclic process and adds a new symbol _tenthsCounter
   to the system. This can be accessed through C with the name
   tenthsCounter. */
ioresult init(char *parameters) {
  /* Add a new symbol */
  AddSymbol("_tenthsCounter", (void *)LIB_ID, (int)(&counter));
  /* Activate the cyclic function. */
  AddCyclic(&myCyclicNode, TICKS_PER_SEC/10, TICKS_PER_SEC/10);
}

void fini(void) {
  /* Stop the cyclic function. */
  DropCyclic(&myCyclicNode);
  /* Remove all symbols associated with LIB_ID. */
  SymbolDeleteLib((void *)LIB_ID);
}
