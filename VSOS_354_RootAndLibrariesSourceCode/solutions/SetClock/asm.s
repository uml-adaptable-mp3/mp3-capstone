#include <vstypes.h>
#include <vs1005h.h>

#define TICKS_PER_SEC 1000

#if 0
	.sect code,GetMR0
	.export _GetMR0
_GetMR0:
	jr
	mv mr0,a0
#endif

	.sect code,TestSpeedLoopAsm
	.export _TestSpeedLoopAsm
_TestSpeedLoopAsm:
	ldx (i6)+1,null
	stx mr0,(I6)+1 ; sty i7,(I6)
	stx b0,(I6)+1 ; sty b1,(I6)
	stx c0,(I6) ; sty c1,(I6)

	ldc MR_INT,mr0

	ldc INT_GLOB_ENA,i1
	and a,null,a	// Result register
	ldc TICKS_PER_SEC,c0	// How many cycles
	and c1,null,c1

	ldc INT_GLOB_DIS,i7
	sty i7,(i7)+(INT_GLOB_ENA-INT_GLOB_DIS)
	ldc _timeCount,i5
$1:
	.import _timeCount
	nop
	nop
	ldy (i5)+1,b0
	ldy (i5)-1,b1
	sub b,d,b;	sty i7,(i7)+(INT_GLOB_DIS-INT_GLOB_ENA)
	ldc 0,b2
	sub b,c,b;	sty i7,(i7)+(INT_GLOB_ENA-INT_GLOB_DIS)
	nop
	jcc $1
	sub a,ones,a

	sty i7,(i7)+(INT_GLOB_DIS-INT_GLOB_ENA)

	ldx (I6)-1,c0 ; ldy (I6),c1
	ldx (I6)-1,b0 ; ldy (I6),b1
	jr
	ldx (I6)-1,mr0 ; ldy (I6),i7

	.end

#if 0
	/********* EVERYTHING FROM NOW ON IS PURELY DECORATIONAL!!! **********/

	.sect code,Generic
	.export _Generic
_Generic:
	stx mr0,(I6)+1 ; sty i7,(I6)
	stx lr0,(I6)+1 ; sty i5,(I6)	// return address
	stx i0,(I6)+1 ; sty i1,(I6)
	stx i2,(I6)+1 ; sty i3,(I6)
	stx i4,(I6)+1 ; sty i5,(I6)
	stx a0,(I6)+1 ; sty a1,(I6)
	stx b0,(I6)+1 ; sty b1,(I6)
	stx c0,(I6)+1 ; sty c1,(I6)
	stx d0,(I6)+1 ; sty d1,(I6)
	stx le,(I6)+1 ; sty ls,(I6)
	stx lc,(I6)



	ldx (I6)-1,lc
	ldx (I6)-1,le ; ldy (I6),ls
	ldx (I6)-1,d0 ; ldy (I6),d1
	ldx (I6)-1,c0 ; ldy (I6),c1
	ldx (I6)-1,b0 ; ldy (I6),b1
	ldx (I6)-1,a0 ; ldy (I6),a1
	ldx (I6)-1,i4 ; ldy (I6),i5
	ldx (I6)-1,i2 ; ldy (I6),i3
	ldx (I6)-1,i0 ; ldy (I6),i1
	ldx (I6)-1,lr0 ; ldy (I6),i5
	jr
	ldx (I6)-1,mr0 ; ldy (I6),i7
#endif
