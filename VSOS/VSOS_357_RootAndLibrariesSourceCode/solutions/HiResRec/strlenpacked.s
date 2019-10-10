/**
	\file hiresAsm.s
	
*/
#include <vs1005h.h>
#include <vstypes.h>


#ifdef ASM

/*
size_t register __a0 strlenpacked(const char register __i0 *s);
*/
	.sect code,strlenpacked
	.export _strlenpacked
_strlenpacked:
	add null,ones,a0;	ldx (i6)+1,null
	stx b0,(I6)+1;	sty b1,(I6)
	stx c0,(I6);	sty c1,(I6)

	ldc 0xff,b0
	ldc -8,b1

	ldx (i0)+1,c0
	ashl c0,b1,c1;	mv a0,i7
$1:
	ldy (i7)+1,null
	jzs $3
	and c0,b0,c1
	
	ldx (i0)+1,c0; ldy (i7)+1,null
	jzc $1
	ashl c0,b1,c1

$3:
	mv i7,a0
	ldx (I6)-1,c0;	ldy (I6),c1
	jr
	ldx (I6)-1,b0;	ldy (I6),b1


#endif /* ASM */


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
	stx lr0,(I6)+1;	sty lc,(I6)	// return address
	stx i2,(I6)+1;	sty i3,(I6)
	stx a0,(I6)+1;	sty a1,(I6)
	stx b0,(I6)+1;	sty b1,(I6)
	stx c0,(I6)+1;	sty c1,(I6)
	stx d0,(I6)+1;	sty d1,(I6)
	stx le,(I6)+1;	sty ls,(I6)
	stx i4,(I6)

	ldx (I6)-1,i4
	ldx (I6)-1,le;	ldy (I6),ls
	ldx (I6)-1,d0;	ldy (I6),d1
	ldx (I6)-1,c0;	ldy (I6),c1
	ldx (I6)-1,b0;	ldy (I6),b1
	ldx (I6)-1,a0;	ldy (I6),a1
	ldx (I6)-1,i2;	ldy (I6),i3
	ldx (I6)-1,lr0;	ldy (I6),lc
	jr
	ldx (I6)-1,i0;	ldy (I6),i1
#endif
