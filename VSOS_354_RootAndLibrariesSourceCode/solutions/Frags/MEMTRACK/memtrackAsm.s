/**
	\file auoosetAsm.s

*/
#include <vs1005h.h>
#include <vstypes.h>
#include <aucommon.h>

#ifdef ASM

#define MEM_TRACK_VECTORS 20

	.sect code,memTrackVectors
	.export _memTrackVectors
_memTrackVectors:
#if 1
	jmpi _MyMallocX
	jmpi _MyMallocY
	jmpi _MyCallocX
	jmpi _MyCallocY
	jmpi _MyReAllocX
	jmpi _MyReAllocY
	jmpi _MyFreeX
	jmpi _MyFreeY
#else
	.word 0, 0, 0, 0, 0, 0, 0, 0
#endif
	jmpi _MyAllocMemX
	jmpi _MyAllocMemY
	jmpi _MyAllocMemXY
	jmpi _MyFreeMemX
	jmpi _MyFreeMemY
	jmpi _MyFreeMemXY
	jmpi _MyReAllocMemX
	jmpi _MyReAllocMemY
	jmpi _MyReAllocMemXY
	jmpi _MyAllocMemAbsX
	jmpi _MyAllocMemAbsY
	jmpi _MyAllocMemAbsXY

	.sect code,SwapMemTrackVectors
	.export _SwapMemTrackVectors
_SwapMemTrackVectors:
	stx i0,(I6)+1 ; sty i1,(I6)
	stx a0,(I6)+1 ; sty a1,(I6)
	stx b0,(I6)+1 ; sty b1,(I6)
	stx le,(I6)+1 ; sty ls,(I6)
	stx lc,(I6)

	/* _AllocMemX is first of 21 vectors */
	.import _malloc
	ldc _malloc,i1
	ldc MEM_TRACK_VECTORS-1,lc

	loop lc,$1-1
	ldc _memTrackVectors,i0

	ldi (i0),a
	and a,a,a
	ldi (i1),b
	jzs $2
	sti b,(i0)++
	sti a,(i1)++
$2:
	nop
$1:

	ldx (I6)-1,lc
	ldx (I6)-1,le ; ldy (I6),ls
	ldx (I6)-1,b0 ; ldy (I6),b1
	ldx (I6)-1,a0 ; ldy (I6),a1
	jr
	ldx (I6)-1,i0 ; ldy (I6),i1



	.sect code,MyAsmVectors
_MyMallocX:
	ldx (i6)+1,i7
	ldc 1,i5
	stx i5,(i6)+1
	stx lr0,(i6)+1
	stx i7,(i6)
	j _memTrackVectors+0
	ldc CommonMalloc,lr0
CommonMalloc:
	.import _MyMalloc
	j _MyMalloc
	ldc $1,lr0
$1:
	ldx (i6)-1,null
	ldx (i6)-2,lr0
	jr
	nop
_MyMallocY:
	ldx (i6)+1,i7
	ldc 2,i5
	stx i5,(i6)+1
	stx lr0,(i6)+1
	stx i7,(i6)
	j _memTrackVectors+1
	ldc CommonMalloc,lr0

_MyCallocX:
	mv i6,i5
	sty lr0,(i6);	ldx (i5)-1,null
	ldc 1,i7
	sty i7,(i5)
	j _memTrackVectors+2
	ldc CommonCalloc,lr0
CommonCalloc:
	.import _MyCalloc
	j _MyCalloc
	ldy (i6),lr0
_MyCallocY:
	mv i6,i5
	sty lr0,(i6);	ldx (i5)-1,null
	ldc 2,i7
	sty i7,(i5)
	j _memTrackVectors+3
	ldc CommonCalloc,lr0

_MyReAllocX:
	mv i6,i5
	sty lr0,(i6);	ldx (i5)-1,null
	ldc 1,i7
	sty i7,(i5)
	j _memTrackVectors+4
	ldc CommonReAlloc,lr0
CommonReAlloc:
	.import _MyReAlloc
	j _MyReAlloc
	ldy (i6),lr0
_MyReAllocY:
	mv i6,i5
	sty lr0,(i6);	ldx (i5)-1,null
	ldc 1,i7
	sty i7,(i5)
	j _memTrackVectors+5
	ldc CommonReAlloc,lr0
_MyFreeX:
	ldx (i6)+1,i7
	ldc 1,i5
	stx i5,(i6)+1
	stx lr0,(i6)+1
	stx i7,(i6)
	j _memTrackVectors+6
	ldc CommonFree,lr0
CommonFree:
	.import _MyFree
	ldx (i6)-1,null
	j _MyFree
	ldc $1,lr0
$1:
	ldx (i6)-2,lr0
	jr
	nop
_MyFreeY:
	ldx (i6)+1,i7
	ldc 2,i5
	stx i5,(i6)+1
	stx lr0,(i6)+1
	stx i7,(i6)
	j _memTrackVectors+7
	ldc CommonFree,lr0

_MyAllocMemX:
	sty lr0,(i6)+1
	sty a0,(i6);	stx i5,(i6)
	sub null,ones,a0;   mv i6,i5
	ldx (i5)-2,null
	sty a0,(i5);	ldx (i6),i5
	ldy (i6)-1,a0
	j _memTrackVectors+8
	ldc CommonAllocMem,lr0
CommonAllocMem:
	.import _MyAllocMem
	j _MyAllocMem
	ldy (i6),lr0
_MyAllocMemY:
	sty lr0,(i6)+1
	sty a0,(i6);	stx i5,(i6)
	sub null,ones,a0;   mv i6,i5
	sub a0,ones,a0;	    ldx (i5)-2,null
	sty a0,(i5);	ldx (i6),i5
	ldy (i6)-1,a0
	j _memTrackVectors+9
	ldc CommonAllocMem,lr0
_MyAllocMemXY:
	sty lr0,(i6)+1
	sty a0,(i6);	stx i5,(i6)
	mv i6,i5
	ldx (i5)-2,null
	ldc 3,a0
	sty a0,(i5);	ldx (i6),i5
	ldy (i6)-1,a0
	j _memTrackVectors+10
	ldc CommonAllocMem,lr0
_MyFreeMemX:
	sty lr0,(i6)+1
	sty a0,(i6);	stx i5,(i6)
	mv i6,i5
	ldx (i5)-2,null
	ldc 5,a0
	sty a0,(i5);	ldx (i6),i5
	ldy (i6)-1,a0
	j _memTrackVectors+11
	ldc CommonFreeMem,lr0
CommonFreeMem:
	.import _MyFreeMem
	j _MyFreeMem
	ldy (i6),lr0
_MyFreeMemY:
	sty lr0,(i6)+1
	sty a0,(i6);	stx i5,(i6)
	mv i6,i5
	ldx (i5)-2,null
	ldc 6,a0
	sty a0,(i5);	ldx (i6),i5
	ldy (i6)-1,a0
	j _memTrackVectors+12
	ldc CommonFreeMem,lr0
_MyFreeMemXY:
	sty lr0,(i6)+1
	sty a0,(i6);	stx i5,(i6)
	mv i6,i5
	ldx (i5)-2,null
	ldc 7,a0
	sty a0,(i5);	ldx (i6),i5
	ldy (i6)-1,a0
	j _memTrackVectors+13
	ldc CommonFreeMem,lr0
_MyReAllocMemX:
	sty lr0,(i6)+1
	sty a0,(i6);	stx i5,(i6)
	mv i6,i5
	ldx (i5)-2,null
	ldc 9,a0
	sty a0,(i5);	ldx (i6),i5
	ldy (i6)-1,a0
	j _memTrackVectors+14
	ldc CommonAllocMem,lr0
_MyReAllocMemY:
	sty lr0,(i6)+1
	sty a0,(i6);	stx i5,(i6)
	mv i6,i5
	ldx (i5)-2,null
	ldc 10,a0
	sty a0,(i5);	ldx (i6),i5
	ldy (i6)-1,a0
	j _memTrackVectors+15
	ldc CommonAllocMem,lr0
_MyReAllocMemXY:
	sty lr0,(i6)+1
	sty a0,(i6);	stx i5,(i6)
	mv i6,i5
	ldx (i5)-2,null
	ldc 11,a0
	sty a0,(i5);	ldx (i6),i5
	ldy (i6)-1,a0
	j _memTrackVectors+16
	ldc CommonAllocMem,lr0
_MyAllocMemAbsX:
	sty lr0,(i6)+1
	sty a0,(i6);	stx i5,(i6)
	mv i6,i5
	ldx (i5)-2,null
	ldc 13,a0
	sty a0,(i5);	ldx (i6),i5
	ldy (i6)-1,a0
	j _memTrackVectors+17
	ldc CommonAllocMem,lr0
_MyAllocMemAbsY:
	sty lr0,(i6)+1
	sty a0,(i6);	stx i5,(i6)
	mv i6,i5
	ldx (i5)-2,null
	ldc 14,a0
	sty a0,(i5);	ldx (i6),i5
	ldy (i6)-1,a0
	j _memTrackVectors+18
	ldc CommonAllocMem,lr0
_MyAllocMemAbsXY:
	sty lr0,(i6)+1
	sty a0,(i6);	stx i5,(i6)
	mv i6,i5
	ldx (i5)-2,null
	ldc 15,a0
	sty a0,(i5);	ldx (i6),i5
	ldy (i6)-1,a0
	j _memTrackVectors+19
	ldc CommonAllocMem,lr0


#endif /* ASM */

	.end

#if 0
	/********* EVERYTHING FROM NOW ON IS PURELY DECORATIONAL!!! **********/

/*
	Generic()

	Inputs:
	  i0: data (16-bit)
	  a0: chan

	Outputs:
	  -
*/
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

	ldc MR_INT,mr0

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
