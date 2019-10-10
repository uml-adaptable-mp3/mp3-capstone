/**
	\file hiresAsm.s
	
*/
#include <vs1005h.h>
#include <vstypes.h>
#include "interleave.h"


#ifdef ASM


/*
void Deinterleave16To16Simple(register __i1 s_int16 *d, register __i0 const s_int16 *s, register __d0 samples);
*/
	.sect code,Deinterleave16To16Simple
	.export _Deinterleave16To16Simple
_Deinterleave16To16Simple:
	ldx (i6)+1,null
	stx i2,(I6)+1;	sty i3,(I6)
	stx lr0,(i6)

	mv d0,i3
	mv i1,i2

	call _Deinterleave16To16
	ldx (i2)*,null

	ldx (i6)-1,lr0
	jr
	ldx (I6)-1,i2;	ldy (I6),i3



/*
void Deinterleave16To32Simple(register __i1 s_int32 *d, register __i0 const s_int16 *s, register __d0 samples);
*/
	.sect code,Deinterleave16To32Simple
	.export _Deinterleave16To32Simple
_Deinterleave16To32Simple:
	ldx (i6)+1,null
	stx i2,(I6)+1;	sty i3,(I6)
	stx lr0,(i6)

	mv d0,i3
	mv i1,i2

	ldx (i2)*,null
	call _Deinterleave16To32
	ldx (i2)*,null

	ldx (i6)-1,lr0
	jr
	ldx (I6)-1,i2;	ldy (I6),i3



/*
void Deinterleave32To16Simple(register __i1 s_int16 *d, register __i0 const s_int32 *s, register __d0 samples);
*/
	.sect code,Deinterleave32To16Simple
	.export _Deinterleave32To16Simple
_Deinterleave32To16Simple:
	ldx (i6)+1,null
	stx i2,(I6)+1;	sty i3,(I6)
	stx lr0,(i6)

	mv d0,i3
	mv i1,i2

	call _Deinterleave32To16
	ldx (i2)*,null

	ldx (i6)-1,lr0
	jr
	ldx (I6)-1,i2;	ldy (I6),i3



/*
void Deinterleave32To32Simple(register __i1 s_int32 *d, register __i0 const s_int32 *s, register __d0 samples);
*/
	.sect code,Deinterleave32To32Simple
	.export _Deinterleave32To32Simple
_Deinterleave32To32Simple:
	ldx (i6)+1,null
	stx i2,(I6)+1;	sty i3,(I6)
	stx lr0,(i6)

	mv d0,i3
	mv i1,i2

	ldx (i2)*,null
	call _Deinterleave32To32
	ldx (i2)*,null

	ldx (i6)-1,lr0
	jr
	ldx (I6)-1,i2;	ldy (I6),i3



/*
void Interleave16To16Simple(register __i1 s_int16 *d, register __i0 const s_int16 *s, register __d0 samples);
*/
	.sect code,Interleave16To16Simple
	.export _Interleave16To16Simple
_Interleave16To16Simple:
	ldx (i6)+1,null
	stx i2,(I6)+1;	sty i3,(I6)
	stx lr0,(i6)

	mv d0,i3
	mv i0,i2

	call _Interleave16To16
	ldx (i2)*,null

	ldx (i6)-1,lr0
	jr
	ldx (I6)-1,i2;	ldy (I6),i3



/*
void Interleave16To32Simple(register __i1 s_int32 *d, register __i0 const s_int16 *s, register __d0 samples);
*/
	.sect code,Interleave16To32Simple
	.export _Interleave16To32Simple
_Interleave16To32Simple:
	ldx (i6)+1,null
	stx i2,(I6)+1;	sty i3,(I6)
	stx lr0,(i6)

	mv d0,i3
	mv i0,i2

	call _Interleave16To32
	ldx (i2)*,null

	ldx (i6)-1,lr0
	jr
	ldx (I6)-1,i2;	ldy (I6),i3



/*
void Interleave32To16Simple(register __i1 s_int16 *d, register __i0 const s_int32 *s, register __d0 samples);
*/
	.sect code,Interleave32To16Simple
	.export _Interleave32To16Simple
_Interleave32To16Simple:
	ldx (i6)+1,null
	stx i2,(I6)+1;	sty i3,(I6)
	stx lr0,(i6)

	mv d0,i3
	mv i0,i2

	ldx (i2)*,null
	call _Interleave32To16
	ldx (i2)*,null

	ldx (i6)-1,lr0
	jr
	ldx (I6)-1,i2;	ldy (I6),i3



/*
void Interleave32To32Simple(register __i1 s_int32 *d, register __i0 const s_int32 *s, register __d0 samples);
*/
	.sect code,Interleave32To32Simple
	.export _Interleave32To32Simple
_Interleave32To32Simple:
	ldx (i6)+1,null
	stx i2,(I6)+1;	sty i3,(I6)
	stx lr0,(i6)

	mv d0,i3
	mv i0,i2

	ldx (i2)*,null
	call _Interleave32To32
	ldx (i2)*,null

	ldx (i6)-1,lr0
	jr
	ldx (I6)-1,i2;	ldy (I6),i3



/*
void Deinterleave16To16(register __i1 s_int16 *d1, register __i2 s_int16 *d2, register __i0 const s_int16 *s, register __d0 samples);
*/
	.sect code,Deinterleave16To16
	.export _Deinterleave16To16
_Deinterleave16To16:
	add d0,ones,d0; ldx (i6)+1,null
	stx lr0,(I6)+1;	sty lc,(I6)	// return address
	jns $1
	stx le,(I6);	sty ls,(I6)

	loop d0,$2-1
	nop

	ldx (i0)++,d0
	stx d0,(i1)++
	ldx (i0)++,d0
	stx d0,(i2)++
$2:
$1:
	ldx (I6)-1,le;	ldy (I6),ls
	ldx (I6)-1,lr0;	ldy (I6),lc
	jr
	nop



/*
void Deinterleave16To32(register __i1 s_int32 *d1, register __i2 s_int32 *d2, register __i0 const s_int16 *s, register __d0 samples);
*/
	.sect code,Deinterleave16To32
	.export _Deinterleave16To32
_Deinterleave16To32:
	add d0,ones,d0; ldx (i6)+1,null
	stx lr0,(I6)+1;	sty lc,(I6)	// return address
	jns $1
	stx le,(I6);	sty ls,(I6)

	loop d0,$2-1
	ldc 0,i7

	stx i7,(i1)++
	ldx (i0)++,d0
	stx d0,(i1)++
	stx i7,(i2)++
	ldx (i0)++,d0
	stx d0,(i2)++
$2:
$1:
	ldx (I6)-1,le;	ldy (I6),ls
	ldx (I6)-1,lr0;	ldy (I6),lc
	jr
	nop



/*
void Deinterleave32To16(register __i1 s_int16 *d1, register __i2 s_int16 *d2, register __i0 const s_int32 *s, register __d0 samples);
*/
	.sect code,Deinterleave32To16
	.export _Deinterleave32To16
_Deinterleave32To16:
	add d0,ones,d0; ldx (i6)+1,null
	stx lr0,(I6)+1;	sty lc,(I6)	// return address
	jns $1
	stx le,(I6);	sty ls,(I6)

	loop d0,$2-1
	ldx (i0)+1,null

	ldx (i0)+2,d0
	stx d0,(i1)++
	ldx (i0)+2,d0
	stx d0,(i2)++
$2:
$1:
	ldx (I6)-1,le;	ldy (I6),ls
	ldx (I6)-1,lr0;	ldy (I6),lc
	jr
	nop



/*
void Deinterleave32To32(register __i1 s_int32 *d1, register __i2 s_int32 *d2, register __i0 const s_int32 *s, register __d0 samples);
*/
	.sect code,Deinterleave32To32
	.export _Deinterleave32To32
_Deinterleave32To32:
	add d0,ones,d0; ldx (i6)+1,null
	stx lr0,(I6)+1;	sty lc,(I6)	// return address
	jns $1
	stx le,(I6);	sty ls,(I6)

	loop d0,$2-1
	nop

	ldx (i0)++,d0
	stx d0,(i1)++
	ldx (i0)++,d0
	stx d0,(i1)++
	ldx (i0)++,d0
	stx d0,(i2)++
	ldx (i0)++,d0
	stx d0,(i2)++
$2:
$1:
	ldx (I6)-1,le;	ldy (I6),ls
	ldx (I6)-1,lr0;	ldy (I6),lc
	jr
	nop



/*
void Interleave16To16(register __i1 s_int16 *d, register __i0 const s_int16 *s1, register __i2 const s_int16 *s2, register __d0 samples);
*/
	.sect code,Interleave16To16
	.export _Interleave16To16
_Interleave16To16:
	add d0,ones,d0;	ldx (i6)+1,null
	stx lr0,(I6)+1;	sty lc,(I6)	// return address
	jns $1
	stx le,(I6);	sty ls,(I6)

	loop d0,$2-1
	nop

	ldx (i0)++,d0
	stx d0,(i1)++
	ldx (i2)++,d0
	stx d0,(i1)++
$2:
$1:
	ldx (I6)-1,le;	ldy (I6),ls
	ldx (I6)-1,lr0;	ldy (I6),lc
	jr
	nop



/*
void Interleave16To32(register __i1 s_int32 *d, register __i0 const s_int16 *s1, register __i2 const s_int16 *s2, register __d0 samples);
*/
	.sect code,Interleave16To32
	.export _Interleave16To32
_Interleave16To32:
	add d0,ones,d0;	ldx (i6)+1,null
	stx lr0,(I6)+1;	sty lc,(I6)	// return address
	jns $1
	stx le,(I6);	sty ls,(I6)

	loop d0,$2-1
	ldc 0,i7

	stx i7,(i1)++
	ldx (i0)++,d0
	stx d0,(i1)++
	stx i7,(i1)++
	ldx (i2)++,d0
	stx d0,(i1)++
$2:
$1:
	ldx (I6)-1,le;	ldy (I6),ls
	ldx (I6)-1,lr0;	ldy (I6),lc
	jr
	nop



/*
void Interleave32To16(register __i1 s_int16 *d, register __i0 const s_int32 *s1, register __i2 const s_int32 *s2, register __d0 samples);
*/
	.sect code,Interleave32To16
	.export _Interleave32To16
_Interleave32To16:
	add d0,ones,d0;	ldx (i6)+1,null
	stx lr0,(I6)+1;	sty lc,(I6)	// return address
	jns $1
	stx le,(I6);	sty ls,(I6)

	loop d0,$2-1
	ldx (i0)++,null; ldy (i2)++,null

	ldx (i0)+2,d0
	stx d0,(i1)++
	ldx (i2)+2,d0
	stx d0,(i1)++
$2:
$1:
	ldx (I6)-1,le;	ldy (I6),ls
	ldx (I6)-1,lr0;	ldy (I6),lc
	jr
	nop



/*
void Interleave32To32(register __i1 s_int32 *d, register __i0 const s_int32 *s1, register __i2 const s_int32 *s2, register __d0 samples);
*/
	.sect code,Interleave32To32
	.export _Interleave32To32
_Interleave32To32:
	add d0,ones,d0;	ldx (i6)+1,null
	stx lr0,(I6)+1;	sty lc,(I6)	// return address
	jns $1
	stx le,(I6);	sty ls,(I6)

	loop d0,$2-1
	nop

	ldx (i0)++,d0
	stx d0,(i1)++
	ldx (i0)++,d0
	stx d0,(i1)++
	ldx (i2)++,d0
	stx d0,(i1)++
	ldx (i2)++,d0
	stx d0,(i1)++
$2:
$1:
	ldx (I6)-1,le;	ldy (I6),ls
	ldx (I6)-1,lr0;	ldy (I6),lc
	jr
	nop


#endif

/*
void Convert32ITo16I(register __i1 s_int16 *d, register __d0 stereoSamples);
*/
	.sect code,Convert32ITo16I
	.export _Convert32ITo16I
_Convert32ITo16I:
	add d0,ones,d0;	ldx (i6)+1,null
	stx lr0,(I6)+1;	sty lc,(I6)	// return address
	jns $1
	stx le,(I6);	sty ls,(I6)

	add d0,d0,d0;	mv i1,i7
	sub d0,ones,d0;	ldx (i7)+1,null
	loop d0,$2-1
	nop

	ldx (i7)+2,d0
	stx d0,(i1)+1
$2:
$1:
	ldx (I6)-1,le;	ldy (I6),ls
	ldx (I6)-1,lr0;	ldy (I6),lc
	jr
	nop

/*
void Convert16ITo32I(register __i1 s_int16 *d, register __d0 stereoSamples);
*/
	.sect code,Convert16ITo32I
	.export _Convert16ITo32I
_Convert16ITo32I:
	ldx (i6)+1,null
	add d0,ones,a0;	stx a0,(I6)+1
	stx i0,(I6)+1;	sty lc,(I6)
	jns $1
	stx le,(I6);	sty ls,(I6)

	add d0,d0,d0;	mv i1,i7
	add d0,ones,a0;	mv d0,i0
	and d0,null,d0;	ldx (i1)*,null
	mv i1,i5			// i5 = s16 pointer
	ldx (i1)*,null
	mv i1,i7	   		// i7 = s32 pointer


	loop a0,$2-1
	ldx (i7)-1,null; ldy (i5)-1,null

	ldx (i5)-1,a0
	stx a0,(i7)-1
	stx d0,(i7)-1
$2:
$1:
	ldx (I6)-1,le;	ldy (I6),ls
	ldx (I6)-1,i0;	ldy (I6),lc
	jr
	ldx (I6)-1,a0


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
