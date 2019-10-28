/**
	\file devAudioAsm.s
	
*/
#include <vs1005h.h>
#include <vstypes.h>


#ifdef ASM



// auto u_int16 *XP_FifoRd8(register u_int16 __i0 *d, register __a0 u_int16 n8x);
	.sect code,XP_FifoRd8
	.export _XP_FifoRd8
_XP_FifoRd8:
	add a0,ones,a0
	stx le,(i6)+1;	sty ls,(i6)
	jns $1
	stx lc,(i6)

	loop a0,$2-1
	ldc XP_IDATA,i7

	// Leaves one out of 9 clock cycles free on purpose
			ldy (i7),a0
	stx a0,(i0)+1;	ldy (i7),a0
	stx a0,(i0)+1;	ldy (i7),a0
	stx a0,(i0)+1;	ldy (i7),a0
	stx a0,(i0)+1;	ldy (i7),a0
	stx a0,(i0)+1;	ldy (i7),a0
	stx a0,(i0)+1;	ldy (i7),a0
	stx a0,(i0)+1;	ldy (i7),a0
	stx a0,(i0)+1
$2:
$1:
	mv i0,a0
	ldx (i6)-1,lc
	jr
	ldx (i6)-1,le;	ldy (i6),ls




// auto u_int16 *XP_FifoWr8(register const u_int16 __i0 *s, register __a0 u_int16 n8x);
	.sect code,XP_FifoWr8
	.export _XP_FifoWr8
_XP_FifoWr8:
	add a0,ones,a0
	stx le,(i6)+1;	sty ls,(i6)
	jns $1
	stx lc,(i6)

	loop a0,$2-1
	ldc XP_ODATA,i7

	// Leaves one out of 9 clock cycles free on purpose
	ldx (i0)+1,a0
	ldx (i0)+1,a0;	sty a0,(i7)
	ldx (i0)+1,a0;	sty a0,(i7)
	ldx (i0)+1,a0;	sty a0,(i7)
	ldx (i0)+1,a0;	sty a0,(i7)
	ldx (i0)+1,a0;	sty a0,(i7)
	ldx (i0)+1,a0;	sty a0,(i7)
	ldx (i0)+1,a0;	sty a0,(i7)
			sty a0,(i7)
$2:
$1:
	mv i0,a0
	ldx (i6)-1,lc
	jr
	ldx (i6)-1,le;	ldy (i6),ls


// auto void XP_FifoWr8X(register u_int16 __b0 x, register __a0 u_int16 n8x);
	.sect code,XP_FifoWr8X
	.export _XP_FifoWr8X
_XP_FifoWr8X:
	add a0,ones,a0
	stx le,(i6)+1;	sty ls,(i6)
	jns $1
	stx lc,(i6)

	loop a0,$2-1
	ldc XP_ODATA,i7

	// Leaves one out of 9 clock cycles free on purpose
	nop
	sty b0,(i7)
	sty b0,(i7)
	sty b0,(i7)
	sty b0,(i7)
	sty b0,(i7)
	sty b0,(i7)
	sty b0,(i7)
	sty b0,(i7)
$2:
$1:
	ldx (i6)-1,lc
	jr
	ldx (i6)-1,le;	ldy (i6),ls

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
