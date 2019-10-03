	.sect code,LongLongMac
	.export _LongLongMac
_LongLongMac:	//21->18
	//ldx (i6)+1,null
	stx c0,(i6)+1	; sty c1,(i6)			//00000000 7DE07509
	and d1,null,d1	; stx d0,(i6)	; sty d1,(i6)

	muluu a0,b0	; ldx (i0)+2,null		//$1751*$1751
	add null,p,c	; ldx (i0),d0	// lo0->
	ldc 0,c2					//$021FA7A1
	//ldc 0,d1
	add c,d,c	//7509+021fa7a1 = 02201CAA

	mulsu a1,b0	; stx c0,(i0)+1	// ->lo0	//0000 * 1751  ->1CAA
	add c1,null,c0	; mv c2,c1		//00000220
	macsu b1,a0,c	; ldx (i0)-3,d0	// lo1->	//0000 * 1751  7de0->
	macss a1,b1,c	; ldx (i0)+1,a0	//hi0		//0000 * 0000  0000->
	//ldc 0,d1
	add c,d,c	; ldx (i0)+2,a1	//hi1		//7de0+0220=00008000
	stx c0,(i0)-3	; ldy (i6),d1	// ->lo1	//->8000
	add c1,null,c0	; mv c2,c1

	add c,p,c	; ldx (i6)-1,d0
	add c,a,a	; ldx (i6),c0

	stx a0,(i0)+1	; ldy (i6)-1,c1	//hi0
	jr
	stx a1,(i0)-1			//hi1







	.sect code,LongLongToDouble
	.export _LongLongToDouble
_LongLongToDouble:
	//ldx (i6)+1,null
	ldx (i0)+1,a0	; sty c0,(i6)+1
	ldx (i0)+1,a1	; sty c1,(i6)
	sub a,ones,a	; ldx (i0)+1,c0	// check -1
	ldx (i0)+1,c1	// guard bits = sign bit
	jzs $1
	add a,ones,a	// check 0
	ldc 32+31+1,i0
	jzc $0
	nop
 
$1:	mv a2,c2	// fix the sign
	add c,null,a
	ldc 32768,b0
	jzs $3		// hi=0, lo=0 -> return 0.0 (asr at $3 does not matter)
	ldc 31+1,i0
	jes $3		// asr at $3 required!
	ldc 31+1,b0

	//only A contains information here! exp can be used
	//ldc 31,b0
	exp a,c0	; ldx (i0)-1,null
	ldc 8,c1
	sub c0,c1,c0	; mv i0,b0
	sub b0,c0,b0	; ldy (i6)-1,c1
	jr
	ashl a,c0,a	; ldy (i6)-1,c0


$0:	exp a,b0	; mv d0,i7	// 8..38
	ldc 8,d0
	sub b0,d0,b0			// 0..30
	ashl a,b0,a	// <<= b0
	ldc 32,d0
	sub b0,d0,b0	; mv i7,d0	//-(32-(ex-8))
	ldc 0,c2
	ashl c,b0,c	// >>= 32-(ex-8)
	or a,c,a	; ldy (i6)-1,c1
	ldc 31,c0
	jr
	sub c0,b0,b0	; ldy (i6)-1,c0	//b0 = 31+32-(ex-8)

$3:	asr a,a	; ldy (i6)-1,c1
	jr
	ldy (i6)-1,c0

	.end
