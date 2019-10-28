/**
	\file vs1005h_compatAsm.s

	Provides binary compatibility with VS1005h by offering
	data that is missing from VS1005g ROM
	
*/
#include <vs1005g.h>
#include <vstypes.h>


#ifdef ASM


	/* encVMiscAsm.s */
	.sect code,EncVSampleClear
	.export _EncVSampleClear
_EncVSampleClear:
	stx le,(I6)+1 ; sty ls,(I6)
	stx lc,(I6)

	add a0,ones,a0
	loop a0,$1-1
	and a0,null,a0

	stx a0,(i0)++; sty a0,(i0)
$1:
	ldx (I6)-1,lc
	ldx (I6)-1,le ; ldy (I6),ls
	jr
	nop	// Can't fill with loop register returns



	/* encVMiscAsm.s */
	.sect code,EncVShiftUpAndCopyToSamp
	.export _EncVShiftUpAndCopyToSamp
_EncVShiftUpAndCopyToSamp:
	stx b0,(I6)+1 ; sty b1,(I6)
	stx c0,(I6)+1 ; sty c1,(I6)
	stx lc,(I6)+1 ; sty i1,(I6)

	ldc 16,b0
	sub a1,b0,a1;	stx le,(I6)

	add a0,ones,a0; sty ls,(I6)
	and b0,null,b0;	ldx (i2)+1,b1
	ashl b,a1,c
	loop a0,$1-1
	ldc 1,i1

	and b0,null,b0;	ldx (i2)+1,b1
	ashl b,a1,c;	stx c0,(i0)*; sty c1,(i0)
$1:
	ldx (I6)-1,le ; ldy (I6),ls
	ldx (I6)-1,lc ; ldy (I6),i1
	ldx (I6)-1,c0 ; ldy (I6),c1
	jr
	ldx (I6)-1,b0 ; ldy (I6),b1



	/* encVMiscAsm.s */
	.sect code,MemCpySampleT
	.export _MemCpySampleT
_MemCpySampleT:
	add a0,ones,a0
	stx lc,(I6)+1 ; sty a1,(I6)
	jns $1
	stx le,(I6) ; sty ls,(I6)

	loop a0,$1-1
	nop

	ldx (i2)+1,a0; ldy (i2),a1
	stx a0,(i0)+1; sty a1,(i0)

$1:
	ldx (I6)-1,le ; ldy (I6),ls
	jr
	ldx (I6)-1,lc ; ldy (I6),a1


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
