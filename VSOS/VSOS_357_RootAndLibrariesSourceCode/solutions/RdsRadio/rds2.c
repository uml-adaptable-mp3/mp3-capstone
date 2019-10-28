#include <vo_stdio.h>
#include <math.h>
#include <stdlib.h>
#include <vstypes.h>
#include "rds.h"



s_int16 biphasecoeff2[] = {
  321,-454,-191,989,-494,-1112,1520,454,
-2344,1003,2320,-2795,-1033,4082,-1389,-3980,4182,2014,
-6139,1534,6085,-5551,-3449,8405,-1327,-8574,6754,5349,
-10728,681,11328,-7640,-7667,12925,450,-14179,8074,10297,
-14801,-2055,16921,-7960,-13074,16175,4060,-19328,7260,15789,
-16895,-6327,21177,-5998,-18202,16868,8663,-22276,4273,20070,
-16071,-10835,22486,-2249,-21174,14559,12595,-21740,143,21341,
-12467,-13707,20060,1798,-20473,9995,13973,-17553,-3324,18559,
-7393,-13262,14410,4207,-15681,4933,11527,-10883,-4275,12015,
-2877,-8815,7266,3431,-7809,1449,5267,-3859,-1668,3367,
-809,-1110,937,-927,989,1035,-3368,1274,4179,-4945,
-2110,7848,-2629,-7842,8230,3926,-12005,3072,11624,-10642,
-6298,15548,-2638,-15221,12067,8983,-18245,1453,18343,-12484,
-11707,19943,289,-20750,11968,14200,-20586,-2351,22270,-10672,
-16219,20209,4477,-22819,8807,17580,-18934,-6424,22399,-6617,
-18169,16948,7990,-21101,4348,17956,-14483,-9029,19083,-2224,
-16986,11784,9466,-16551,421,15377,-9085,-9297,13735,940,
-13294,6586,8586,-10861,-1808,10931,-4435,-7445,8132,2193,
-8488,2724,6023,-5705,-2156,6147,-1480,-4478,3686,1799,
  -4055,679,2962,-2122,-1240,2314,-256,-1600,1010,602,
  -976,118,484,-302,7,43,-164,338,-75,-507,519,298,-858,215,849,-779,-442,1103,-205,-1016,814,540,-1126,125,1022,-709,-567,993,-33,-901,537,518,-772,-33,700,-355,-410,526,60,-470,201,271,-302,-51,255,-93,-132,130,19,-87,32,23,-22,13,-14,-6,38,-25,-29,48,-1,-44,28,15,-25,9,0
};
s_int16 biphasecoeffcos2[] = {
-181,-325,743,-86,-1131,993,893,-1999,
192,2474,-1903,-1842,3561,-69,-4249,2808,3233,-5346,
-385,6419,-3580,-5089,7230,1257,-8896,4085,7382,-9058,
-2593,11538,-4200,-10026,10657,4394,-14165,3834,12878,-11859,
-6596,16568,-2945,-15747,12521,9075,-18537,1550,18407,-12543,
-11653,19880,265,-20618,11893,14104,-20448,-2350,22156,-10608,
-16184,20158,4500,-22836,8801,17651,-19004,-6475,22535,-6654,
-18295,17066,8027,-21214,4397,17963,-14504,-8924,18921,-2288,
-16583,11545,8984,-15800,583,14173,-8457,-8092,12070,492,
-10850,5529,6225,-8011,-775,6815,-3030,-3453,3936,179,
-2342,1186,-62,-153,1289,-2252,-151,4080,-3058,-3532,
6641,-6,-8309,5483,6367,-10516,-687,12431,-6986,-9547,
13621,2118,-16136,7528,12789,-15778,-4096,19156,-7161,-15804,
16897,6382,-21288,6022,18328,-16985,-8713,22413,-4313,-20147,
16139,10834,-22506,2274,21123,-14531,-12524,21631,-155,-21198,
12383,13620,-19931,-1811,20404,-9941,-14031,17606,3437,-18846,
7446,13742,-14891,-4589,16690,-5113,-12812,12025,5206,-14140,
3105,11358,-9229,-5290,11416,-1529,-9539,6684,4905,-8723,
427,7533,-4520,-4159,6238,218,-5517,2807,3185,-4092,
-469,3645,-1559,-2122,2364,421,-2034,739,1097,-1077,
-182,758,-279,-212,211,-138,155,86,-465,291,451,-720,-66,906,-510,-690,986,132,-1115,533,818,-1019,-214,1126,-443,-828,897,269,-992,310,735,-694,-275,771,-182,-573,471,232,-523,88,381,-273,-156,293,-34,-197,125,70,-115,15,54,-33,0,4,-14,30,-8,-38,40,15,-50,17,31,-30,0,13,-5
};


__align __mem_y s_int16 bpmem2[RDS_FIFOSIZE];
s_int16 bpptr = 161;

struct RDSFILTER rdsfilt = {
  bpmem2, bpmem2, biphasecoeff2, biphasecoeffcos2, 198, (0xa000+RDS_FIFOSIZE-1)
};



void RdsPutFifo(struct RDSFILTER *rds, s_int16 val) {
  rds->fifowr[0] = val;
  rds->fifowr++;
  if (rds->fifowr >= &bpmem2[RDS_FIFOSIZE])
    rds->fifowr -= RDS_FIFOSIZE;
}

#if 0
u_int16 RdsGetFifo(struct RDSFILTER *rds) {
  val = rds->fiford[0];
  rds->fiford++;
  if (rds->fiford >= &bpmem2[RDS_FIFOSIZE])
    rds->fiford -= RDS_FIFOSIZE;
  
}
#endif

s_int16 RdsFill(struct RDSFILTER *rds) {
  s_int16 f = rds->fifowr-rds->fiford;
  if (f < 0) {
    f += RDS_FIFOSIZE;
  }
  return f;
}

void RdsDiscard(struct RDSFILTER *rds, s_int16 skip) {
  rds->fiford += skip;
  if (rds->fiford >= &bpmem2[RDS_FIFOSIZE])
    rds->fiford -= RDS_FIFOSIZE;
}

#ifdef __VSDSP__
void RdsFilter(struct RDSFILTER *rds);
#else
void RdsFilter(struct RDSFILTER *rds) {
  register __reg_d s_int32 acci = 0;
  register __reg_b s_int32 accq = 0;
  register __c0 s_int16 i;
  register __i0 __mem_y s_int16 *p = rds->fiford;
  register __i2 s_int16 *sp = rds->coeffsin;
  register __i3 s_int16 *cp = rds->coeffcos;
  for (i=0;i<rds->size;i++) {
    //printf("  sin%d %d %d\n", i, *p, *sp);
    acci += *p * (long long)*sp++;
    accq += *p * (long long)*cp++;
    --p;
    if (p < &bpmem2[0]) {
      p += RDS_FIFOSIZE;
    }
  }
  acci >>= 15;
  accq >>= 15;
  if (labs(acci) > 32767 || labs(accq) > 32767) {
  //    printf("RdsFilter Over %d %d!\n", acci, accq);
  }
  if (acci > 32767)
    acci = 32767;
  if (acci < -32767)
    acci = -32767;
  if (accq > 32767)
    accq = 32767;
  if (accq < -32767)
    accq = -32767;
  rds->resi = acci;
  rds->resq = accq;
}
#endif

#if 0
long FilterBiphaseBit(int val, long *q) {
  //u_int32 phaseAdd = 0x2600000; //57kHz
    long long acci = 0, accq = 0;
    int i, p; //,sgn;
    if (++bpptr >= RDS_FIFOSIZE)
      bpptr -= RDS_FIFOSIZE;
    p = bpptr;
    bpmem[bpptr] = val;
    for (
#if 0
	 i=0;i<161+161;i++
#else
	 i=62;i<2*161-62;i++
#endif
	 ) {
      //printf("sin%d %d %d\n", i, val, biphasecoeff[i]);
      acci += val * (long long)biphasecoeff[i] >> 15;
      accq += val * (long long)biphasecoeffcos[i] >> 15;
      if (--p < 0) {
	p = RDS_FIFOSIZE-1;
      }
      val = bpmem[p];
    }
    if (q) {
      *q = accq;
    }
    return acci;
}
#endif

#if 0
void PrintFilt() {
  u_int32 phaseAdd = 0x2600000; //57kHz
  u_int32 phaseAcc = 0;
  int i, sgn;
  FILE *fp = fopen("biphasecoeff.dat", "wb");
  for (i=0;i<161;i++) {
    sgn = (i < 80) ? 1 : -1;
    if (fp) {
      fprintf(fp, "%f %f\n",
	      sgn * sin(phaseAcc * 2.0 * M_PI / 0x08000000),
	      sgn * cos(phaseAcc * 2.0 * M_PI / 0x08000000));
    }
    phaseAcc -= phaseAdd;
    if ((s_int32)phaseAcc < 0) {
      phaseAcc += 0x08000000;
    }
  }
  if (fp) {
    fclose(fp);
  }
}
#endif

#ifdef __VSDSP__
auto s_int16 Phase(register __c0 s_int16 re, register __c1 s_int16 im);
#endif



int RdsGetBit(void) {
  static int bcnt = 0;
  int thisbit = -1;
  static int resyncCount = 0;
// System samplerate 192kHz
// Bitrate 57000Hz/48 = 1187.5Hz
// 192000/1187.5 = 3072/19 = 161.6842 samples/bit, i.e. 19 bits / 3072 samples

// 12.288: 161.6842   pll_factor 53127850 0x32AAAAA ???? 0x2600000
// 13.000: 171.0526   pll_factor          0x2FE445F ???? 0x23EB347.5D6B99
// 12.000: 157.8947   pll_factor          0x33E1F67 ???? 0x26E978D.4FDF3B
  static u_int32 phaseAcc = 0x0, phaseAdd = 0x2600000;//-0x460; //57kHz

  while (thisbit < 0) {
    int d;
    long i1 = 0, i2 = 0, aa, arg;

    if (!RdsFill(&rdsfilt)) {
      return -1;
    }
    phaseAcc += phaseAdd;
    if (phaseAcc >= 0x08000000) {
      phaseAcc -= 0x08000000;
    }

    ++bcnt;
    if (bcnt==1 || bcnt==40 || bcnt==80 || bcnt>=159) {
      RdsFilter(&rdsfilt);
      RdsDiscard(&rdsfilt, 1);
      i1 = rdsfilt.resi;
      i2 = rdsfilt.resq;

      arg = Phase(i2,i1) - (s_int16)((phaseAcc) >> 11);
      aa = ((s_int32)i2*i2>>16)+((s_int32)i1*i1>>16);
    } else {
      RdsDiscard(&rdsfilt, 1);
      continue;
    }

    if (1) {
      static long previous = 0;
      static int phS = 0, phM = 0, phM2 = 0, phE = 0;
      if (bcnt == 1) {
	phS = arg; /* Previous bit last half */
      }
      if (bcnt == 40) { /* current bit first half */
	phM = arg;
      }
      if (bcnt == 80) {
	phM2 = arg;
      }
      if (bcnt == 160) { /* current bit second half */
	int diff1;
	phE = arg;

	diff1 = phE - phS;
	if (diff1 > 32767)
	  diff1 -= 65536;
	if (diff1 < -32768)
	  diff1 += 65536;

	if (abs(diff1) < 16384) { /*Close enough to have two phase shifts*/
	  /* Two phase-shifts -> 0*/
	  thisbit = 0;
	} else {
	  /* One phase shift -> 1 */
	  thisbit = 1;
	      
	  if (abs(phE - phM) > 10000 || abs(phE - phM2) > 10000) {
	    /* See phase shift in the middle -> ok */
	    resyncCount = 0;
	  } else {
	    /* No phase shift in the middle -> wrong sync. */
	    if (++resyncCount > 1) {
	      //	      printf("*");
	      resyncCount = 0;
	      /* No phase shift in the middle -> find sync. */
	      bcnt -= 80;
	    }
	  }
	}
      }
      if ((bcnt >= 161 && labs(previous) > labs(aa)) || bcnt >= 162) {
	bcnt = 0;
      }
      previous = aa;
    } /* if (1) */
  } /* while (thisbit < 0) */
  return thisbit;
}



#if 0
int main(int argc, char *argv[]) {
    char line[100];
    long bitnum = 0;
    FILE *out = fopen("rds2.out", "wb");
    int resyncCount = 0;

#ifdef __VSDSP__
    FILE *in = fopen("fm16_2.dat", "rb");
#else
    FILE *in = NULL;
    if (argc > 1) {
      in = fopen(argv[1], "rb");
    }
    if (!in)
      in = stdin;
#endif

// System samplerate 192kHz
// Bitrate 57000Hz/48 = 1187.5Hz
// 192000/1187.5 = 3072/19 = 161.6842 samples/bit, i.e. 19 bits / 3072 samples

// 12.288: 161.6842   pll_factor 53127850 0x32AAAAA ???? 0x2600000
// 13.000: 171.0526   pll_factor          0x2FE445F ???? 0x23EB347.5D6B99
// 12.000: 157.8947   pll_factor          0x33E1F67 ???? 0x26E978D.4FDF3B
    u_int32 phaseAcc = 0x0, phaseAdd = 0x2600000;//-0x460; //57kHz

    //PrintFilt();

    while (1) {
	int wrap;
	static int bcnt = 0, thisbit = 0;
	static long linenum = 1;
	int s, d;
	long i1 = 0, i2 = 0, aa, arg;

	wrap = 0;
	phaseAcc += phaseAdd;
	if (phaseAcc >= 0x08000000) {
	    phaseAcc -= 0x08000000;
	    wrap = 1;
	}

	if (fgets(line, 100, in) == 0)
	    break;
	linenum++;
	s = strtol(line, NULL, 0);
	if (s & 32768) {
	    s |= ~32767; // sign extend input value
	}

	RdsPutFifo(&rdsfilt, s); //NEW

	++bcnt;
	if (bcnt==1 || bcnt==40 || bcnt==80 || bcnt>=159) {

	  RdsFilter(&rdsfilt);
	  RdsDiscard(&rdsfilt, 1);
	  i1 = rdsfilt.resi;
	  i2 = rdsfilt.resq;

	//printf("iq %d %d\n", rdsfilt.resi, rdsfilt.resq);

	//d = s;
	//i1 = FilterBiphaseBit(d, &i2);
	



#ifdef __VSDSP__
#if 0
	  printf("Phase %5d %5d %5d  %5d\n",
		 Phase(i2,i1),
		 (s_int16)(atan2(i1,i2) / M_PI * 32767),
		 (s_int16)((phaseAcc) >> 11),
		 Phase(i2,i1) - (s_int16)((phaseAcc) >> 11));
#endif
	  arg = Phase(i2,i1) - (s_int16)((phaseAcc) >> 11);
	  aa = ((s_int32)i2*i2>>16)+((s_int32)i1*i1>>16);
#else
	  arg = (atan2(i1, i2) - (s_int32)phaseAcc * (2.0 * M_PI / 0x08000000)) / M_PI * 32767;
	  if (arg > 32767L) {
	    arg -= 65536L;
	  }
	  if (arg < -32768L) {
	    arg += 65536L;
	  }
	  //aa = sqrt(i2*i2+i1*i1); //no need for sqrt in actual code
	  aa = ((s_int32)i2*i2>>16)+((s_int32)i1*i1>>16);
#endif
	} else {
	  RdsDiscard(&rdsfilt, 1);
	  continue;
	}
#if 0
	printf("12121212 %5d %3d %5ld %5ld %6ld %6ld %d\n",
	       s, bcnt*100, i1, i2, aa, arg, thisbit*-2000);
#endif
	//if (out) {  fprintf(out, "%ld\n", i1);	}

	if (1) {
	  static long previous = 0;
	  static int phS = 0, phM = 0, phM2 = 0, phE = 0;
	  if (bcnt == 1) {
	    phS = arg; /* Previous bit last half */
	  }
	  if (bcnt == 40) { /* current bit first half */
	    phM = arg;
	  }
	  if (bcnt == 80) {
	    phM2 = arg;
	  }
	  if (bcnt == 160) { /* current bit second half */
	    int diff1;
	    phE = arg;

	    diff1 = phE - phS;
	    if (diff1 > 32767)
	      diff1 -= 65536;
	    if (diff1 < -32768)
	      diff1 += 65536;

	    bitnum++;
	    //fprintf(stderr, "bit %5d %5d %5d %5d  d%5d @%5ld (%ld)\n", phS, phM, phM2, phE, diff1, linenum, bitnum);

	    if (abs(diff1) < 18000) { /*Close enough to have two phase shifts*/
	      /* Two phase-shifts -> 0*/
	  printf("0 %6ld    \r", linenum);
	      if (out) {  fprintf(out, "0\n");	}
	      thisbit = 0;
	    } else {
	      /* One phase shift -> 1 */
	  printf("1 %6ld    \r", linenum);
	      if (out) {  fprintf(out, "1\n");	}
	      thisbit = 1;
	      
	      if (abs(phE - phM) > 10000 || abs(phE - phM2) > 10000) {
		/* See phase shift in the middle -> ok */
		resyncCount = 0;
	      } else {
		/* No phase shift in the middle -> wrong sync. */
		if (++resyncCount > 1) {
		  resyncCount = 0;
		  /* No phase shift in the middle -> find sync. */
		  printf("bit sync! %5d %5d %5d @%5ld\n", phS, phM, phE, linenum);
		  bcnt -= 80;
		}
	      }
	    }
	  }
	  if ((bcnt >= 160 && labs(previous) > labs(aa)) || bcnt >= 164) {
	    bcnt = 0;
	  }
	  previous = aa;
	}
	if (out && (linenum & 0xffff) == 0) fflush(out);
    }
    if (out) {      fclose(out);    }
    if (in != stdin)
      fclose(in);
    return 0;
}
#endif
