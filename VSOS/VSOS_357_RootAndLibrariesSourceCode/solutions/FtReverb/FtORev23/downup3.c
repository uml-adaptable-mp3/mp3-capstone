/*
   This is an automatically generated file by VLSI Solution's
   Octave program fir2vsdsp.m.

   DO NOT MODIFY unless you ABSOLUTELY know what you are doing!
*/

#include <vstypes.h>
#include <fir.h>
#include "downup3.h"

/* Filter structures for FIR filter "down3L" */

const s_int16 down3LCoeff[121] = {
      -9,    -35,    -74,   -105,    -90,     -3,    141,    271,
     293,    157,    -90,   -306,   -341,   -144,    171,    386,
     330,     19,   -328,   -444,   -215,    202,    484,    391,
     -32,   -458,   -532,   -162,    373,    630,    367,   -234,
    -681,   -572,     47,    680,    770,    186,   -621,   -956,
    -468,    496,   1125,    807,   -290,  -1273,  -1225,    -30,
    1398,   1771,    534,  -1497,  -2582,  -1421,   1569,   4128,
    3493,  -1613,  -9731, -17197, -20218, -17197,  -9731,  -1613,
    3493,   4128,   1569,  -1421,  -2582,  -1497,    534,   1771,
    1398,    -30,  -1225,  -1273,   -290,    807,   1125,    496,
    -468,   -956,   -621,    186,    770,    680,     47,   -572,
    -681,   -234,    367,    630,    373,   -162,   -532,   -458,
     -32,    391,    484,    202,   -215,   -444,   -328,     19,
     330,    386,    171,   -144,   -341,   -306,    -90,    157,
     293,    271,    141,     -3,    -90,   -105,    -74,    -35,
      -9,
};
s_int16 __align __mem_y down3LMem[121] = {0};
struct FirFilter __mem_y down3L = {
  FirFilter16x16NUpDn, 0x0000/*flags*/, 121/*len*/,
  down3LCoeff, down3LMem, down3LMem+61,
  0x8078/*mod1*/, 0x0000/*mod2*/, -1/*shL*/,
  1/*up*/, 3/*down*/, 3/*dnPhase*/, 0x2aab/*1/dn*/
};
s_int16 __align __mem_y down3RMem[121] = {0};
struct FirFilter __mem_y down3R = {
  FirFilter16x16NUpDn, 0x0000/*flags*/, 121/*len*/,
  down3LCoeff, down3RMem, down3RMem+61,
  0x8078/*mod1*/, 0x0000/*mod2*/, -1/*shL*/,
  1/*up*/, 3/*down*/, 3/*dnPhase*/, 0x2aab/*1/dn*/
};


/* Filter structures for FIR filter "up3L" */

const s_int16 up3LCoeff[129] = {
       4,     32,    112,    261,    447,    580,    545,    284,
    -126,   -479,   -548,   -258,    223,    563,    498,     47,
    -462,   -625,   -286,    307,    673,    487,   -128,   -667,
    -653,    -62,    619,    789,    259,   -540,   -901,   -462,
     434,    993,    673,   -300,  -1070,   -898,    135,   1132,
    1145,     67,  -1184,  -1428,   -320,   1226,   1767,    649,
   -1260,  -2204,  -1105,   1287,   2824,   1799,  -1307,  -3835,
   -3030,   1321,   5952,   6007,  -1329, -14163, -26407, -31436,
  -26407, -14163,  -1329,   6007,   5952,   1321,  -3030,  -3835,
   -1307,   1799,   2824,   1287,  -1105,  -2204,  -1260,    649,
    1767,   1226,   -320,  -1428,  -1184,     67,   1145,   1132,
     135,   -898,  -1070,   -300,    673,    993,    434,   -462,
    -901,   -540,    259,    789,    619,    -62,   -653,   -667,
    -128,    487,    673,    307,   -286,   -625,   -462,     47,
     498,    563,    223,   -258,   -548,   -479,   -126,    284,
     545,    580,    447,    261,    112,     32,      4,      0,
       0,
};
s_int16 __align __mem_y up3LMem[43] = {0};
struct FirFilter __mem_y up3L = {
  FirFilter16x16NUpDn, 0x0000/*flags*/, 43/*len*/,
  up3LCoeff, up3LMem, up3LMem+64,
  0x802a/*mod1*/, 0x0000/*mod2*/, 0/*shL*/,
  3/*up*/, 1/*down*/, 1/*dnPhase*/, 0x8000/*1/dn*/
};
s_int16 __align __mem_y up3RMem[43] = {0};
struct FirFilter __mem_y up3R = {
  FirFilter16x16NUpDn, 0x0000/*flags*/, 43/*len*/,
  up3LCoeff, up3RMem, up3RMem+64,
  0x802a/*mod1*/, 0x0000/*mod2*/, 0/*shL*/,
  3/*up*/, 1/*down*/, 1/*dnPhase*/, 0x8000/*1/dn*/
};

