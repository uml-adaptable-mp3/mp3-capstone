#include <stdio.h>
#include <vs1005g.h>
#include "xPerip.h"

/* This function is entered in a Forbid() state, but with all other
   interrupts active except the XPERIP interrupt. */
#if 1
auto void XPeripIntC(register __a0 u_int16 intNumber) {
#if 0
  static u_int16 intCount = 0;
  /* You really should not print from an interrupt. It's a bad idea overall
     and will very probably lead to lockups. */
  printf("Int %d," /* " st %04x,"*/ " cnt %d\n",
	 intNumber, /*PERIP(XP_ST),*/ ++intCount);
#endif

  switch (intNumber) {
    // Definitions for XP_ST bits are in vs1005g.h
  case XP_ST_ETXRB_HALF2_INT_B:
    break;
  case XP_ST_ETXRB_HALF1_INT_B:
    break;
  case XP_ST_ERXRB_HALF2_INT_B:
    break;
  case XP_ST_ERXRB_HALF1_INT_B:
    break;
  case XP_ST_SPIERR_INT_B:
    break;
  case XP_ST_RSEC_RDY_INT_B:
    break;
  case XP_ST_RSDEC_RDY_INT_B:
    break;
  case XP_ST_RSENC_RDY_INT_B:
    break;
  case XP_ST_SD_INT_B:
    break;
  case XP_ST_NF_INT_B:
    break;
  case XP_ST_SPI_STOP_INT_B:
    break;
  case XP_ST_SPI_START_INT_B:
    break;
  case XP_ST_ETHRX_INT_B:
    break;
  case XP_ST_ETHTX_INT_B:
    break;
  default:
    break;
  }
}
#endif


#if 0
/* To test compiler bug in LCC 1.44. This compiles ok. */
int Func1Ok(register int n) {
  switch(n) {
  case 0:
    break;
  case 1:
    break;
  case 2:
    break;
  case 3:
    break;
  }
  return n;
}

/* To test compiler bug in LCC 1.44. This compiles wrong (default clause
   not handled by compiler). */
int Func2Err(register int n) {
  switch(n) {
  case 0:
    break;
  case 1:
    break;
  case 2:
    break;
  case 3:
    break;
  default:
    break;
  }
  return n;
}
#endif
