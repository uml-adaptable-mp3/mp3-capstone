/**
	\file hiresAsm.s
	
*/
#include <vs1005h.h>
#include <vstypes.h>
#include "miscAsm.h"


#ifdef ASM

/*
void CombineDryWet16(register __i0 s_int16 *dry, register __d0 u_int16 dryGain, register __i1 s_int16 *wet, register __d1 u_int16 n);
*/
	.sect code,CombineDryWet16
	.export _CombineDryWet16
_CombineDryWet16:
	add d1,ones,d1;	ldx (i6)+1,null
	stx a0,(I6)+1;	sty a1,(I6)
	stx b0,(I6)+1;	sty b1,(I6)
	stx c0,(I6)+1;	sty c1,(I6)
	stx mr0,(I6)+1;	sty lc,(I6)	// return address
	stx le,(I6);	sty ls,(I6)

	jns $1
	and c0,null,c0;	ldx (i0)+1,a0

	ldc 16-10,a1
	ldc MR_INT|MR_SAT,mr0
	loop d1,$2-1
	mulsu a0,d0

	add null,p,b;	ldx (i0)-1,a0
	ashl b,a1,b;	ldx (i1)+1,c1
	add b,c,b
	rnd b,b1
	mulsu a0,d0;	stx b1,(i0)+2
$2:
$1:
	ldx (I6)-1,le;	ldy (I6),ls
	ldx (I6)-1,mr0;	ldy (I6),lc
	ldx (I6)-1,c0;	ldy (I6),c1
	ldx (I6)-1,b0;	ldy (I6),b1
	jr
	ldx (I6)-1,a0;	ldy (I6),a1




/*
void CombineDryWet32(register __i0 s_int16 *dry, register __d0 u_int16 dryGain, register __i1 s_int16 *wet, register __d1 u_int16 n);
*/
	.sect code,CombineDryWet32
	.export _CombineDryWet32
_CombineDryWet32:
	add d1,ones,d1;	ldx (i6)+1,null
	stx a0,(I6)+1;	sty a1,(I6)
	stx b0,(I6)+1;	sty b1,(I6)
	stx c0,(I6)+1;	sty c1,(I6)
	stx le,(I6)+1;	sty ls,(I6)
	stx mr0,(I6);	sty lc,(I6)

	jns $1
	ldc 16-10,c0

	ldc MR_INT|MR_SAT,mr0
	muluu a0,d0;	ldx (i0)+1,a0
	add null,p,b;	ldx (i0)+1,a1	// resLo = ...

	loop d1,$2-1
	ldc 0,i7
	
	add b1,null,b0; mv i7,b1
	mulsu a1,d0;	ldx (i0)+1,a0
	add b,p,b;	ldx (i0)-3,a1	// resHi = ... + resLo
	ashl b,c0,b;	ldx (i1)+1,c1	// resHi <<= 6
	add b,c1,b
	sat b,b
	muluu a0,d0;	stx b0,(i0)+1
	add null,p,b;	stx b1,(i0)+3	// resLo = ...
		
$2:
$1:
	ldx (I6)-1,mr0;	ldy (I6),lc
	ldx (I6)-1,le;	ldy (I6),ls
	ldx (I6)-1,c0;	ldy (I6),c1
	ldx (I6)-1,b0;	ldy (I6),b1
	jr
	ldx (I6)-1,a0;	ldy (I6),a1


#endif

	.end



#if 0
	/********* EVERYTHING FROM NOW ON IS PURELY DECORATIONAL!!! **********/

/*
	void Generic(void) - 2017 edition
*/
	.sect code,Generic
	.export _Generic
_Generic:
	ldx (i6)+1,null
	stx i0,(I6)+1;	sty i1,(I6)
	stx i2,(I6)+1;	sty i3,(I6)
	stx a0,(I6)+1;	sty a1,(I6)
	stx b0,(I6)+1;	sty b1,(I6)
	stx c0,(I6)+1;	sty c1,(I6)
	stx d0,(I6)+1;	sty d1,(I6)
	stx lr0,(I6)+1;	sty lc,(I6)	// return address
	stx le,(I6)+1;	sty ls,(I6)
	stx i4,(I6)

	ldx (I6)-1,i4
	ldx (I6)-1,le;	ldy (I6),ls
	ldx (I6)-1,lr0;	ldy (I6),lc
	ldx (I6)-1,d0;	ldy (I6),d1
	ldx (I6)-1,c0;	ldy (I6),c1
	ldx (I6)-1,b0;	ldy (I6),b1
	ldx (I6)-1,a0;	ldy (I6),a1
	ldx (I6)-1,i2;	ldy (I6),i3
	jr
	ldx (I6)-1,i0;	ldy (I6),i1
#endif
