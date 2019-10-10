/**
	\file copy.s
	
*/
#include <vstypes.h>
#include "copy.h"



#ifdef ASM

#define SAFETY_BITS 3


	// Copies from 16-bit interleaved buffer to 32-bit buffer
	.sect code,Copy16iTo32Scale
	.export _Copy16iTo32Scale
_Copy16iTo32Scale:
	stx b0,(I6)+1 ; sty b1,(I6)
	and a0,a0,a0
	stx le,(I6)+1 ; sty ls,(I6)
	jzs $1
	add a0,ones,a0;	stx lc,(I6)

	sub null,c0,c0

	loop a0,$2-1
	and b0,null,b0; ldx (i2)+2,b1

	ashl b,c0,b
	and b0,null,b0; stx b0,(i0)+1
	stx b1,(i0)+1
	ldx (i2)+2,b1
$2:

$1:
	ldx (I6)-1,lc
	ldx (I6)-1,le ; ldy (I6),ls
	jr
	ldx (I6)-1,b0 ; ldy (I6),b1






	// Copies from 32-bit buffer to 16-bit interleaved buffer
	.sect code,Copy32To16iScale
	.export _Copy32To16iScale
_Copy32To16iScale:
	stx b0,(I6)+1 ; sty b1,(I6)
	stx c0,(I6)+1 ; sty c1,(I6)
	stx mr0,(I6)+1 ; sty d0,(I6)
	and a0,a0,a0
	stx le,(I6)+1 ; sty ls,(I6)
	jzs $1
	add a0,ones,a0;	stx lc,(I6)
	
	and d0,null,d0;	ldx (i2)+1,c0
	sub null,a1,a1
	ashl d,a1,d;	ldx (i2)+1,c1
	sub null,a1,a1
	loop a0,$2-1
	add c,d,c

	ashl c,a1,b;	ldx (i2)+1,c0
	sat b,b;	ldx (i2)+1,c1
	add c,d,c;	stx b1,(i0)+2
$2:
$1:
	ldx (I6)-1,lc
	ldx (I6)-1,le ; ldy (I6),ls
	ldx (I6)-1,mr0 ; ldy (I6),d0
	ldx (I6)-1,c0 ; ldy (I6),c1
	jr
	ldx (I6)-1,b0 ; ldy (I6),b1




	.sect code,ScaleUp32
	.export _ScaleUp32
_ScaleUp32:
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

	add a0,ones,a0
	nop
	jns $10		// Zero or negative samples
	ldx (i0)+1,b0

	loop a0,$2-1
	ldx (i0)+1,b1

	// Shift upwards; SATuration necessary


	ashl b,c0,a;	ldx (i0)+1,b0
	sat a,a;	ldx (i0)-3,b1
	stx a0,(i0)+1
	stx a1,(i0)+3

$2:
$10:
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



	.sect code,Copy32iTo32Scale
	.export _Copy32iTo32Scale
_Copy32iTo32Scale:
	stx b0,(I6)+1 ; sty b1,(I6)

	add a0,ones,a0;	ldx (i2)+1,b0
	stx a1,(I6)+1 ; sty lc,(I6)
	jns $10		// Zero or negative samples
	stx le,(I6) ; sty ls,(I6)

	loop a0,$2-1
	sub null,c0,c0;	ldx (i2)+3,b1

	ashl b,c0,a;	ldx (i2)+1,b0
	sat a,a;	ldx (i2)+3,b1
	stx a0,(i0)+1
	stx a1,(i0)+1

$2:
$10:
	ldx (I6)-1,le ; ldy (I6),ls
	ldx (I6)-1,a1 ; ldy (I6),lc
	jr
	ldx (I6)-1,b0 ; ldy (I6),b1



	.sect code,Copy32To32iScale
	.export _Copy32To32iScale
_Copy32To32iScale:
	stx b0,(I6)+1 ; sty b1,(I6)
	stx a1,(I6)+1 ; sty lc,(I6)

	add a0,ones,a0;	ldx (i2)+1,b0
	jns $10		// Zero or negative samples
	stx le,(I6) ; sty ls,(I6)

	loop a0,$2-1
	ldx (i2)+1,b1

	ashl b,c0,a;	ldx (i2)+1,b0
	sat a,a;	ldx (i2)+1,b1
	stx a0,(i0)+1
	stx a1,(i0)+3

$2:
$10:
	ldx (I6)-1,le ; ldy (I6),ls
	ldx (I6)-1,a1 ; ldy (I6),lc
	jr
	ldx (I6)-1,b0 ; ldy (I6),b1




#endif /* ASM */

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
