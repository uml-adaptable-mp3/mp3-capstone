/**
  \file xPerip.h Routines for handling the xPerip interrupt.
*/

/*
  VS1005g xPerip interrupt handling. (C) 2013 VLSI Solution Oy

  Revision History
  2013-05-30  1.00  HH  Initial version.
*/

#ifndef VS1005G_XPERIP_INT
#define VS1005G_XPERIP_INT



#ifndef ASM
  #include <vstypes.h>
  #include <lists.h>
  #include <exec.h>
#endif /* !ASM */




#ifndef ASM


/**
   XPERIP interrupt routine.
   Never call explicitly!
   Set up an interrupt vector INT_XPERIP to this by writing
   WriteIMem((void *)(0x20+INTV_XPERIP),
       0x2a000000e+((u_int32)((u_int16)XPeripIntAsm) << 6));
   Calls XPeripIntC().
*/
void XPeripIntAsm(void);
/**
   Called by XPeripInAsm(). Handles the XPERIP interrupts.
   This function is entered in a Forbid() state, but with all other
   interrupts active except the XPERIP interrupt.
   \param intNumber The interrupt bit in register XP_ST.
*/
auto void XPeripIntC(register __a0 u_int16 intNumber);

#endif /* !ASM */

#endif /* !VS1005G_XPERIP_INT */
