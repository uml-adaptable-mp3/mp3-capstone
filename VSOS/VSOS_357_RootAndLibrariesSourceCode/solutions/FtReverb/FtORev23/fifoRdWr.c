/// \file devSdSdExt.c VsOS Device driver for SD card in native SD mode
/// \author Timo Solla, Henrik Herranen, VLSI Solution Oy

/* NOTE!
   This file is used to compile several projects with different preprocessor
   directives! */

#include <vo_stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vs1005h.h>
#include "fifoRdWr.h"

auto void xp_fiford(LCC_REGISTER u_int16 *dbuf, register u_int16 addr,
		    register u_int16 wcnt) { 
  PERIP(XP_CF) |= XP_CF_RDBUF_ENA;     // read fifo
  PERIP(XP_ADDR) = addr;               // addr ->
  PERIP(XP_IDATA);             // 1st dummy read (for pipeline)
  PERIP(XP_IDATA);             // 2nd dummy read (for pipeline)

  dbuf = XP_FifoRd8(dbuf, wcnt>>3);
  wcnt &= 7;
  while (wcnt--) {
    *dbuf++ = PERIP(XP_IDATA);           // read from fifo
  }

  PERIP(XP_CF) &= ~XP_CF_RDBUF_ENA;     // release dsp if (reset do op)
  return;
}

// Write data buffer to nf mem
auto void xp_fifowr(LCC_REGISTER const u_int16 *dbuf, register u_int16 addr,
		    register u_int16 wcnt) { 
  PERIP(XP_CF) |= XP_CF_WRBUF_ENA;    // write fifo
  PERIP(XP_ADDR) = addr;              // addr ->

  dbuf = XP_FifoWr8(dbuf, wcnt>>3);
  wcnt &= 7;

  while (wcnt--) {
    PERIP(XP_ODATA) = *dbuf++;           // write to fifo
  }
  PERIP(XP_CF) &= ~XP_CF_WRBUF_ENA;     // release dsp if (reset do op)
  return;
}


// Write data buffer to nf mem
auto void xp_fifowrx(LCC_REGISTER u_int16 x, register u_int16 addr,
		    register u_int16 wcnt) { 
  PERIP(XP_CF) |= XP_CF_WRBUF_ENA;    // write fifo
  PERIP(XP_ADDR) = addr;              // addr ->

  XP_FifoWr8X(x, wcnt>>3);
  wcnt &= 7;

  while (wcnt--) {
    PERIP(XP_ODATA) = x;           // write to fifo
  }

  PERIP(XP_CF)  &= ~XP_CF_WRBUF_ENA;     // release dsp if (reset do op)
  return;
}
