#include <vstypes.h>
#include <vsasm.h>

	/* 3rd order polynomial division maxerr 0.107% */

	// 6 cycles overhead + differentiation abs(re)-abs(im) abs(re)+abs(im)
	// 4 cycles for normalization and final shift calculation
	// 6 cycles for 3rd order polynomial 1/x
	// 10.5 cycles for y*1/x result
	// 6 cycles for angle calc (2 cycles moved to X * 1/Y calc)
	// 9.5 cycles for quadrant fix and register restore
	// 42 cycles total

	.sect data_x,const_x	// 14 words
_phaseconst:
#if 0
	.word 0x200, 0, -23, 23+12, 7366, 21938, -24106, 11574 // fix needed
#else
	.word 0x200, 0, -23, 23+12, 7366, 21942, -24107, 11568 // no fix needed
#endif
	 /* 'optimized' for 3rd order fit +-0.044% */
	.word 14000, 12247+1, 10407+1, 0x000, 24576, 8192


// auto s_int16 Phase(register __c0 s_int16 re, register __c1 s_int16 im);

	.sect code,Phase	// 58 words
	.export _Phase
_Phase:	// 35.5 cycles	// Do not use i5 !!

//     register __b0 s_int16 r;
//     register __b1 s_int16 angle;
//     r = atanfrdiv16(abs( re ) - abs( im ) , abs( re ) + abs( im ) );

	stx MR0,(I6)+1	; sty D1,(I6)
	ldc _phaseconst,i7

	abs C0,A0	; ldx (i7)+1,mr0
	abs C1,A1	; stx A1,(I6)
	sub A0,A1,B0	; sty B0,(I6)+1
	add A0,A1,A0	; ldx (i7)+1,a1	// a1=a2=0, a0 always positive
	// 2 cycles overhead, 4 cycles for differentiation

//     if (D) {	// D is always positive
// 	while (D < 0x8000U) {
// 	    D <<= 1;
// 	    ex--;
// 	}
// 	ex += 12;

	// we need a long register for exp because the input has 16 bits.
	exp a,b1	; stx D0,(i6)	; sty B1,(i6)	// b1 = 23..39 or 0
	ashl a,b1,a	; ldx (i7)+1,d1	//-23->d1	shift data to A2/A1
	ashl a,d1,a	; ldx (i7)+1,d1	//23+12->d1	shift data to A0
	sub b1,d1,b1	; ldx (i7)+1,a1	//7366->a1
	// 4 cycles for normalization and final shift calculation

// 	register __c0 u_int16 x;

//	x = ((s_int16)
//	     (((s_int16)
//	       ((u_int16)(21938U - (u_int16)(7366U * (u_int32)D >> 16)) *
//		(u_int32)D >> 16) - 24106) * (s_int32)D >> 16) + 11574);


	muluu A1,A0	; ldx (i7)+1,a1	//21938->A1
	sub A1,P,D
	mulsu D1,A0	; ldx (i7)+1,a1	//-24106->A1
	add A1,P,D
	mulsu D1,A0	; ldx (i7)+1,a1	//11574->A1
	add A1,P,D
	// 6 cycles for 3rd order polynomial

// 	r = (s_int16)((s_int32)dividend  * x >> ex);

	mulus D1,B0
	add null,p,D
	// maybe is necessary for low signal levels..
	//ldc 0,d2	// does not seem to be necessary, D fits into 31 bits
	ashl D,B1,D
	// 3 cycles for result

	// A1, B1, A0, D1 scratch, D0 input

//     {
// 	register __a0 s_int16 r2 = (s_int32)r*r >> 16;
// 	angle = ((((12246-(s_int16)(14000*(s_int32)r2>>16))*(s_int32)r2>>16) - 10389)*r>>15);

	mulss D0,D0
	add NULL,P,A	; ldx (i7)+1,b0	//14000->B0
	mulss A1,B0	; ldx (i7)+1,a0	//12246+1->A0
	sub A0,P,B	; ldx (i7)+1,a0	//10389+1->A0
	mulss B1,A1	; ldx (i7)+1,mr0	// FRACT mode, negated result
	sub A0,P,A
	mulss D0,A1
	// 7 cycles for angle calc

//     }
//     if (re >= 0) {
// 	angle = 8192 + angle;
//     } else {
// 	angle = 24576 - angle;
//     }
//     return (im < 0) ? -angle : angle;

	add C0,NULL,C0	; ldx (i7)+1,a0	// 24576->A0
	ldx (I6)-1,D0	; ldy (i6),B1
	jlt $2
	add A0,P,A	; ldx (i7)+1,c0	// 8192->c0	// negated back

	sub c0,P,A	// negated back
$2:
	add C1,NULL,C1	; mv a1,a0
	ldx (I6)-1,A1 ; ldy (I6),B0
	jrge
	ldx (I6)-1,MR0 ; ldy (I6),D1
	jr
	sub NULL,A0,A0
	// 9.5 cycles for quadrant fix and register restore




#if 1
// void RdsFilter(struct RDSFILTER *rds) {

	.sect code,RdsFilter
	.export _RdsFilter
_RdsFilter:	// PROLOGUE
	ldx (I6)+1,NULL	// Room for Sp and Fp
	stx MR0,(I6)+2 ; sty I5,(I6)
	stx I6,(I6) ; sty I4,(I6)
	ldx (I6)+1,I4	// FP' = SP, reserve space
	stx A0,(I6)+1 ; sty A1,(I6)
	stx B0,(I6)+1 ; sty B1,(I6)
	stx C0,(I6)+1 ; sty C1,(I6)
	stx D0,(I6)+1 ; sty D1,(I6)

//   register __d s_int32 acci = 0;
//   register __b s_int32 accq = 0;
//   register __c0 s_int16 i;
//   register __i0 __mem_y s_int16 *p = rds->fiford;
//   register __i2 s_int16 *sp = rds->coeffsin;
//   register __i3 s_int16 *cp = rds->coeffcos;

	// indirect field fiford, offset 1 (struct at page 1)
	// indirect field coeffsin, offset 2 (struct at page 1)
	// indirect field coeffcos, offset 3 (struct at page 1)
	// indirect field size, offset 4 (struct at page 1)

	ldy (I4)-3,NULL	  	; stx I0,(I6)
	ldx (I4)+3,I7		; sty I1,(I6)+1
	ldy (I7)+1,NULL		; stx I2,(I6)
	ldx (I7)+1,I0		; sty I3,(I6)	//+1
	and D,NULL,D	; ldx (I7)+2,I2	//+2
	and B,NULL,B	; ldx (I7)-1,A0	//+4

	ldc MR_SAT,mr0	//fractional mode
	ldc 1,i3
	add a0,ones,a0	; ldx (I7)+2,I5	//+3
	ldx (i7)+1,i1	//ldc MAKEMODB(1024),I1
	loop a0,$7-1
	mulss d0,d0	; ldy (I0)*,A1 ; ldx (I2)*,A0	//0->P

//   for (i=0;i<rds->size;i++) {
//     acci += *p * (long long)*sp++;
//     accq += *p * (long long)*cp++;
//     if (--p < &bpmem2[0]) p+=1024;

	macss A1,A0,B	; ldx (I5)+1,A0
	macss A1,A0,D	; ldy (I0)*,A1 ; ldx (I2)*,A0
$7:
	add B,P,B

//   rds->resi = acci >> 15;
//   rds->resq = accq >> 15;

	rnd D,D0		; ldx (I6),I2
	rnd B,B0	; stx D0,(I7)+1	//+5
	stx B0,(I7)		; ldy (I6)-1,I3	//+6

	ldx (I6)-1,I0 ; ldy (I6),I1
	ldx (I6)-1,D0 ; ldy (I6),D1
	ldx (I6)-1,C0 ; ldy (I6),C1
	ldx (I6)-1,B0 ; ldy (I6),B1
	ldx (I6)-1,A0 ; ldy (I6),A1
	ldx (I4),I6 ; ldy (I4),I4
	ldx (I6)-2,NULL
	jr
	ldx (I6)-1,MR0 ; ldy (I6),I5
#endif
	.end
