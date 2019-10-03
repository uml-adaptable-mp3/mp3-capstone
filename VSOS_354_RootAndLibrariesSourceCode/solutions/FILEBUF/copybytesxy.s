#if COREVERSION .ge. 3
// void MemCopyPackedBigEndianXY(register __i0 __mem_y u_int16 *dst,
// 			    register __a0 u_int16 dstidx,
// 			    register __i1 u_int16 *src,
// 			    register __a1 u_int16 srcidx,
// 			    register __b0 u_int16 byteSize) {

#define __FAR__
	
	.sect code,MemCopyPackedBigEndianXY	// 113 words
	.export _MemCopyPackedBigEndianXY
_MemCopyPackedBigEndianXY:
	add b0,null,b0	; ldx (I6)+1,NULL
	stx MR0,(I6)+1 ; sty I7,(I6)
	stx B1,(I6)+1 ; sty C0,(I6)
	stx C1,(I6)+1 ; sty D0,(I6)
	stx D1,(I6)+1 ; sty LC,(I6)
	jzs $16	//byteSize == 0
	stx LE,(I6) ; sty LS,(I6)

	ldc 512,MR0
	// register parameter to function: I0
	// register parameter to function: A0
	// register parameter to function: I1
	// register parameter to function: A1
	// register parameter to function: B0

//     dst += dstidx >> 1;
//     src += srcidx >> 1;

	lsr A0,B1	; mv I0,C0
	add C0,B1,C0
	lsr A1,B1	; mv C0,I0
	mv I1,C0
	add C0,B1,C0
	xor a0,a1,c0	; mv C0,I1

	lsr c0,c0
	ldc 1,B1
	jcc $6	// ==
	lsr a0,c0

//     if ((srcidx & 1) != (dstidx & 1)) {

	ldc 256,C1
	jcc $2
	ldc 0xff00,C0

// 	if (dstidx & 1) {
// 	    *dst = (*dst & 0xff00U) | (*src >> 8);
// 	    dst++;
// 	    byteSize--;

	ldy (I0),B1 ; add B0,ONES,B0
	and B1,C0,B1 ; ldx (I1),C0
	muluu C1,C0
	add NULL,P,C
	or B1,C1,B1
	sty B1,(I0)+1

$2:

// 	}
// 	{
// 	    register __c u_int32 t = *src++;
// 	    int i;
// 	    int j = byteSize >> 1;

// 	    for (i=0; i<j; i++)

	lsr B0,D1	; ldx (I1)+1,C1
	ldc -8,B1
	jzs $1	// >=
	add D1,ONES,D1	; ldx (I1)+1,C0

	loop D1,$1-1
	ashl C,B1,D

//  {
// 		t = (t << 16) | *src++;
// 		*dst++ = t >> 8;
	add c0,null,c1	; ldx (i1)+1,c0
	ashl C,B1,D	; sty D0,(I0)+1
$1:
// 	    }

	lsr B0,B0
	ldc 0x00ff,B0
	jcc $16
	ashl C,B1,D	; ldy (I0),c0	//ashl has not been done if no loops

// 	    if (byteSize & 1) {
// 		*dst = (*dst & 0x00ffU) | ((t << 8) & 0xff00U);

	and c0,B0,c0
	ldc 0xff00,B0
	and d0,B0,d0
	or c0,d0,c0
	j $16
	sty c0,(I0)

// 	    }
// 	}
// 	return;

// jumped away
$6:

//     }

	lsr a1,b1
	ldc 65280,C0
	jcc $13
	ldc 255,C1

//     if (srcidx & 1) {
// 	*dst = (*dst & 0xff00U) | (*src & 0x00ffU);
// 	dst++;
// 	src++;
// 	byteSize--;

	ldy (I0),B1 ; add B0,ONES,B0
	and B1,C0,B1 ; ldx (I1)+1,C0
	and C0,C1,C0
	or B1,C0,B1
	sty B1,(I0)+1


//     }
//     memcpy(dst, src, (byteSize>>1));

$13:
	lsr B0,A0
	ldc 0xff00,C1
	jzs $0
	ADD A0,ONES,A0

	loop A0,$3-1
	ldx (I1)+1,A0	
	ldx (I1)+1,A0	; sty A0,(I0)+1	//does one extra (i1)+1
$3:
	ldx (i1)-1,null	//compensate for that one extra
$0:

//     if ((byteSize & 1))

	lsr B0,B0	; ldy (I0),B1
	ldc 0x00ff,C0
	jcc $16
	and B1,C0,B1 ; ldx (I1),C0

// jumped away
//  {
// 	dst += (byteSize>>1);	//already done by copy loop..
// 	src += (byteSize>>1);
// 	*dst = (*dst & 0x00ffU) | (*src & 0xff00U);

	and C0,C1,C0
	or B1,C0,B1
	sty B1,(I0)

$16:
	ldx (I6)-1,LE ; ldy (I6),LS
	ldx (I6)-1,D1 ; ldy (I6),LC
	ldx (I6)-1,C1 ; ldy (I6),D0
	ldx (I6)-1,B1 ; ldy (I6),C0
	jr __FAR__
	ldx (I6)-1,MR0 ; ldy (I6),I7
#endif

	.end
