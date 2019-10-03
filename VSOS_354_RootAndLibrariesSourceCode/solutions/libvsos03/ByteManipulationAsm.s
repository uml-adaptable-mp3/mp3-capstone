/**
	\file bytemanipulation.s
	
*/
#include <vs1005h.h>
#include <vstypes.h>
#include <ByteManipulation.h>


#ifdef ASM



#if 0
//auto u_int16 GetLE8(register __i0 u_int16 *p, register u_int16 __b0 byteOffset);
	.sect code,GetLE8
	.export _GetLE8
_GetLE8:
	asr b0,a0;	sty i1,(i6)
	mv a0,i1
	sub null,ones,a0;	ldx (i0)*,null
	and b0,a0,b0;		ldx (i0),a0
	ldc -8,b0
	jzs $1
	ldy (i6)-1,i1

	ashl a0,b0,a0
$1:
	ldc 0xff,b0
	jr
	and a0,b0,a0
#endif



//auto u_int16 GetBE8(register __i0 u_int16 *p, register u_int16 __b0 byteOffset);
	.sect code,GetBE8
	.export _GetBE8
_GetBE8:
	asr b0,a0;	sty i1,(i6)
	mv a0,i1
	sub null,ones,a0;	ldx (i0)*,null
	and b0,a0,b0;		ldx (i0),a0
	ldc -8,b0
	jzc $1
	ldy (i6)-1,i1

	ashl a0,b0,a0
$1:
	ldc 0xff,b0
	jr
	and a0,b0,a0



//auto u_int16 GetLE16(register __i0 u_int16 *p, register u_int16 __b0 byteOffset);
	.sect code,GetLE16
	.export _GetLE16
_GetLE16:
	stx i1,(i6)+1;	sty c0,(i6)
	add b0,null,c0;	stx lr0,(i6)+2
	call _GetBE8
	mv i0,i1

	mv i1,i0
	call _GetBE8
	mv a0,c0; sub c0,ones,b0

	ldc 8,b0
	ashl a0,b0,a0
	or a0,c0,a0;	ldx (i6)-1,lr0
	jr
	ldx (i6)-1,i1;	ldy (i6),c0



//auto u_int16 GetBE16(register __i0 u_int16 *p, register u_int16 __b0 byteOffset);
	.sect code,GetBE16
	.export _GetBE16
_GetBE16:
	stx i1,(i6)+1;	sty c0,(i6)
	add b0,null,c0;	stx lr0,(i6)+2
	call _GetBE8
	mv i0,i1

	ldc 8,b0
	mv i1,i0;	ashl a0,b0,a0
	call _GetBE8
	mv a0,c0;	sub c0,ones,b0
	
	or a0,c0,a0;	ldx (i6)-1,lr0
	jr
	ldx (i6)-1,i1;	ldy (i6),c0



//auto u_int32 GetLE32(register __i0 u_int16 *p, register u_int16 __b0 byteOffset);
	.sect code,GetLE32
	.export _GetLE32
_GetLE32:
	stx i1,(i6)+2;	sty lr0,(i6)
	call _GetLE16
	mv i0,i1;	add b0,null,a1

	mv i1,i0;	add a1,null,b0
	call _GetLE16
	add a0,null,a1;	ldx (i0)+1,null

	ldx (i6)-1,i1;	ldy (i6),lr0
	jr
	add a1,null,a0;	mv a0,a1



//auto u_int32 GetBE32(register __i0 u_int16 *p, register u_int16 __b0 byteOffset);
	.sect code,GetBE32
	.export _GetBE32
_GetBE32:
	stx i1,(i6)+2;	sty lr0,(i6)
	call _GetBE16
	mv i0,i1;	add b0,null,a1

	mv i1,i0;	add a1,null,b0
	call _GetBE16
	add a0,null,a1;	ldx (i0)+1,null

	ldx (i6)-1,i1;	ldy (i6),lr0
	jr
	nop



//void SetBE8(register __i0 u_int16 *p, register u_int16 __a0 byteOffset, register u_int16 __b0 data);
	.sect code,SetBE8
	.export _SetBE8
_SetBE8:
#if 1
	ldx (i6)+1,null
	stx c0,(i6)+1;	sty c1,(i6)

	asr a0,c0;	sty i1,(i6)
	mv c0,i1
	sub null,ones,c0;	ldx (i0)*,null
	and a0,c0,c0;		ldx (i0),c1
	ldc 8,c0
	jzc $1
	ldc 0x00ff,a0

	j $2
	ashl b0,c0,b0
$1:
	and b0,a0,b0
	ldc 0xff00,a0
$2:
	and c1,a0,c1;	ldy (i6)-1,i1
	or c1,b0,b0;	ldx (i6),c0
	jr
	stx b0,(i0);	ldy (i6)-1,c1
#else
	ldx (i6)+1,null
	stx i1,(i6)+1;	sty a1,(i6)
	stx b0,(i6);	sty lr0,(i6)
	sub null,ones,a1	// a1 = srcIdx

	.import _MemCopyPackedBigEndian
	call _MemCopyPackedBigEndian
	mv i6,i1;	sub null,ones,b0 // b0 = byteSize, i1 = src

	ldy (i6)-1,lr0
	jr
	ldx (i6)-1,i1;	ldy (i6),a1
#endif


//void SetBE16(register __i0 u_int16 *p, register u_int16 __a0 byteOffset, register u_int16 __b0 data);
	.sect code,SetBE16
	.export _SetBE16
_SetBE16:
#if 1
	ldx (i6)+1,null
	stx c0,(i6)+1;	sty c1,(i6)

	asr a0,c0;	sty i1,(i6)
	mv c0,i1
	sub null,ones,c0;	ldx (i0)*,null
	and a0,c0,c0;		ldx (i0),c1
	ldc -8,c0
	jzs $1
	ldc 0xff00,a0

	and c1,a0,c1;	stx d0,(i6)
	ashl b0,c0,c0;	ldx (i0)+1,null
	ldc 0x00ff,a0
	and c0,a0,c0;	ldx (i0)-1,d0
	or c1,c0,c1
	ldc 8,c0
	ashl b0,c0,b0;	stx c1,(i0)+1
	and d0,a0,c1
	or c1,b0,b0;	ldx (i6),d0
$1:
	stx b0,(i0);	ldy (i6)-1,i1
	jr
	ldx (i6),c0;	ldy (i6)-1,c1
#else
	ldx (i6)+1,null
	stx i1,(i6)+1;	sty a1,(i6)
	sub null,ones,b0;	stx b0,(i6)
	and a1,null,a1;		sty lr0,(i6);	// a1 = srcIdx

	.import _MemCopyPackedBigEndian
	call _MemCopyPackedBigEndian
	mv i6,i1;	sub b0,ones,b0 // b0 = byteSize, i1 = src

	ldy (i6)-1,lr0
	jr
	ldx (i6)-1,i1;	ldy (i6),a1
#endif


//void SetLE16(register __i0 u_int16 *p, register u_int16 __a0 byteOffset, register u_int16 __b0 data);
	.sect code,SetLE16
	.export _SetLE16
_SetLE16:
	ldx (i6)+1,null
	.import _Swap16
	stx a0,(i6)+1;	sty lr0,(i6)
	call _Swap16
	nop

	add a0,null,b0
	j _SetBE16
	ldx (i6)-1,a0; ldy (i6),lr0



//void SetBE32(register __i0 u_int16 *p, register u_int16 __a0 byteOffset, register u_int32 __reg_b data);
	.sect code,SetBE32
	.export _SetBE32
_SetBE32:
#if 1
	ldx (i6)+1,null
	stx lr0,(i6)+1;	sty b0,(i6)
	stx i0,(i6);	sty a0,(i6)
	call _SetBE16
	add b1,null,b0

	ldx (i6)-1,i0;	ldy (i6),a0
	ldx (i6)-1,lr0;	ldy (i6),b0
	j _SetBE16
	ldx (i0)+1,null
#else
	ldx (i6)+1,null
	stx i1,(i6)+1;	sty a1,(i6)
	mv i6,i1	// i1 = src
	and a1,null,a1;	stx b1,(i6)+1	// a1 = srcIdx
	stx b0,(i6);	sty lr0,(i6)

	.import _MemCopyPackedBigEndian
	call _MemCopyPackedBigEndian
	ldc 4,b0

	ldy (i6)-2,lr0
	jr
	ldx (i6)-1,i1;	ldy (i6),a1
#endif


//void SetLE32(register __i0 u_int16 *p, register u_int16 __a0 byteOffset, register u_int32 __reg_b data);
	.sect code,SetLE32
	.export _SetLE32
_SetLE32:
	ldx (i6)+1,null
	stx d0,(i6)+1;	sty d1,(i6)
	stx a0,(i6)+1;	sty lr0,(i6)
	.import _Swap32
	call _Swap32
	add b,null,d;	stx a1,(i6)+1

	ldx (i6)-1,a1;	add a,null,b
	ldx (i6)-1,a0;	ldy (i6),lr0
	j _SetBE32
	ldx (i6)-1,d0; ldy (i6),d1

//auto u_int16 BitReverse8(register __d0 u_int16 x);
	.sect code,BitReverse8
	.export _BitReverse8
_BitReverse8:
	stx c0,(i6);	sty b0,(i6)
	stx ls,(i6)+1;	sty lc,(i6)
	ldc _bitReverseConstants+12,i7
	stx le,(i6);	ldy (i7)+1,b0
	j _BitReverse16_8BitEntryPoint
	and d0,b0,d0;	ldy (i7)-4,lc
	



//auto u_int16 BitReverse16(register __d0 u_int16 x);
	.sect code,BitReverse16
	.export _BitReverse16
_BitReverse16:
	stx c0,(i6)+1;	sty b0,(i6)
	stx ls,(i6)+1;	sty lc,(i6)
	ldc _bitReverseConstants+14,i7
	stx le,(i6);	ldy (i7)-2,lc

_BitReverse16_8BitEntryPoint:
	loop lc,$1-1
	ldy (i7)-2,b0

	and d0,b0,a0;	ldy (i7)+1,c0
	ashl a0,c0,a0;	ldy (i7)-2,c0
	ashl d0,c0,d0
	and d0,b0,d0
	or d0,a0,d0;	ldy (i7)-2,b0

$1:
	add d0,null,a0;	ldx (i6)-1,le
	ldx (i6)-1,ls;	ldy (i6),lc
	jr
	ldx (i6)-1,c0;	ldy (i6),b0



	.sect data_y,const_y
	.export _bitReverseConstants
_bitReverseConstants:
	.word 4-1
	.word 1,-1,0x5555
	.word 2,-2,0x3333
	.word 4,-4,0x0f0f
	.word 8,-8,0x00ff,3-1,4-1

//auto u_int32 BitReverse32(register __reg_d x);
	.sect code,BitReverse32
	.export _BitReverse32
_BitReverse32:
	stx b0,(i6)+1;	sty b1,(i6)
	stx c0,(i6)+1;	sty c1,(i6)
	stx ls,(i6)+1;	sty lc,(i6)

	ldc _bitReverseConstants,i7
	ldy (i7)+3,lc;	stx le,(i6)
	add null,ones,c0;	ldy (i7)+1,b0
	loop lc,$1-1
	sub null,ones,c1; mv b0,b1

	and d,b,a
	ashl a,c1,a;	ldy (i7)+1,c1
	ashl d,c0,d;	ldy (i7)+1,c0
	and d,b,d;	ldy (i7),b0
	or d,a,d;	ldy (i7)+1,b1
$1:

	add d0,null,a1;	mv d1,a0
	ldx (i6)-1,le
	ldx (i6)-1,ls;	ldy (i6),lc
	ldx (i6)-1,c0;	ldy (i6),c1
	jr
	ldx (i6)-1,b0;	ldy (i6),b1



//u_int32 BitReverseN32(register __reg_d u_int32 x, register __c0 u_int16 n);
	.sect code,BitReverseN32
	.export _BitReverseN32
_BitReverseN32:
	ldc 32,a0
	sub a0,c0,a0
	j _BitReverse32
	ashl d,a0,d;	ldx (i6)+1,null


//u_int16 BitReverseN16(register __d0 u_int16 x, register __c0 u_int16 n);
	.sect code,BitReverseN16
	.export _BitReverseN16
_BitReverseN16:
	ldc 16,a0
	sub a0,c0,a0
	j _BitReverse16
	ashl d0,a0,d0;	ldx (i6)+1,null

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
