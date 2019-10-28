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

/*
u_int32 TestSpeedLoopAsm(register __reg_d u_int32 startTime);
*/
	.sect code,TestSpeedLoopAsm
	.export _TestSpeedLoopAsm
_TestSpeedLoopAsm:
	and a,null,a;	ldx (i6)+1,null
	stx b0,(I6)+1 ; sty c0,(I6)

	// Exactly 10 cycles / loop
$1:
	.import _timeCount
	nop
	nop
	ldc _timeCount,i5
	ldy (i5),b0
	sub b0,d0,b0
	ldc TICKS_PER_SEC,c0	// How many cycles
	sub b0,c0,b0
	nop
	jcc $1
	sub a,ones,a

	jr
	ldx (I6)-1,b0 ; ldy (I6),c0


/*
u_int32 SwapBits0And2(register __reg_d u_int32 d);
*/
	.sect code,SwapBits0And2
	.export _SwapBits0And2
_SwapBits0And2:
	ldc ~5,a0
	and d0,a0,a0; mv d1,a1	// a = d & ~5;
	sub null,ones,d1;	ldx (i6)+1,null
	and d1,d0,c0;		stx c0,(i6)
	add c0,c0,c0
	add c0,c0,c0
	or a0,c0,a0;		ldx (i6)-1,c0
	lsr d0,d0
	lsr d0,d0
	and d0,d1,d0
	jr
	or a0,d0,a0

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
