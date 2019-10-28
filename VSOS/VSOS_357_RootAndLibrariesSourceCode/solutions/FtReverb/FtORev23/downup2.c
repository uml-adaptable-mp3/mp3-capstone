/*
   This is an automatically generated file by VLSI Solution's
   Octave program fir2vsdsp.m.

   DO NOT MODIFY unless you ABSOLUTELY know what you are doing!
*/

#include <vstypes.h>
#include <fir.h>
#include "updown2.h"

/* Filter structures for FIR filter "down2L" */

const s_int16 down2LCoeff[29] = {
      30,    195,    621,   1200,   1380,    505,  -1256,  -2372,
   -1045,   2449,   4685,   1420,  -8015, -18701, -23407, -18701,
   -8015,   1420,   4685,   2449,  -1045,  -2372,  -1256,    505,
    1380,   1200,    621,    195,     30,
};
s_int16 __align __mem_y down2LMem[29] = {0};
struct FirFilter __mem_y down2L = {
  FirFilter16x16NUpDn, 0x0000/*flags*/, 29/*len*/,
  down2LCoeff, down2LMem, down2LMem+15,
  0x801c/*mod1*/, 0x0000/*mod2*/, -1/*shL*/,
  1/*up*/, 2/*down*/, 2/*dnPhase*/, 0x4000/*1/dn*/
};
s_int16 __align __mem_y down2RMem[29] = {0};
struct FirFilter __mem_y down2R = {
  FirFilter16x16NUpDn, 0x0000/*flags*/, 29/*len*/,
  down2LCoeff, down2RMem, down2RMem+15,
  0x801c/*mod1*/, 0x0000/*mod2*/, -1/*shL*/,
  1/*up*/, 2/*down*/, 2/*dnPhase*/, 0x4000/*1/dn*/
};


/* Filter structures for FIR filter "up2L" */

const s_int16 up2LCoeff[24] = {
       2,    -12,     37,    -92,    199,   -389,    704,  -1209,
    2021,  -3429,   6482, -20699, -20699,   6482,  -3429,   2021,
   -1209,    704,   -389,    199,    -92,     37,    -12,      2,
};
s_int16 __align __mem_y up2LMem[24] = {0};
struct FirFilter __mem_y up2L = {
  FirFilter16x16HBUp2Dn1, 0x0001/*flags*/, 24/*len*/,
  up2LCoeff, up2LMem, up2LMem+12,
  0x2057/*mod1*/, 0x2097/*mod2*/, 0/*shL*/,
  2/*up*/, 1/*down*/, 1/*dnPhase*/, 0x8000/*1/dn*/
};
s_int16 __align __mem_y up2RMem[24] = {0};
struct FirFilter __mem_y up2R = {
  FirFilter16x16HBUp2Dn1, 0x0001/*flags*/, 24/*len*/,
  up2LCoeff, up2RMem, up2RMem+12,
  0x2057/*mod1*/, 0x2097/*mod2*/, 0/*shL*/,
  2/*up*/, 1/*down*/, 1/*dnPhase*/, 0x8000/*1/dn*/
};

