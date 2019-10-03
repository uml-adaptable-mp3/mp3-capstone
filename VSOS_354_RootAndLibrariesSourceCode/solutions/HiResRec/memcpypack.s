/**
	\file hiresAsm.s
	
*/
#include <vs1005h.h>
#include <vstypes.h>


#ifdef ASM

/*
register __i0 *memcpypack(register __i0 *d, const register __i1 *s, size_t register __a0 n);
*/
	.sect code,memcpypack
	.export _memcpypack
_memcpypack:
	ldx (i6)+1,null
	stx b0,(I6)+1;	sty b1,(I6)
	stx c0,(I6)+1;	sty c1,(I6)
	stx a1,(I6)+1;	sty lc,(I6)	// return address

	lsr a0,a1;	stx le,(I6)
	mv i0,i5
	jzs $1
	add a1,ones,a1;	sty ls,(I6)

	ldc 8,b0
	ldc 0xFF,b1

	ldx (i1)+1,c1
	loop a1,$2-1
	ashl c1,b0,c0;	ldx (i1)+1,c1

	and c1,b1,c1;	ldx (i1)+1,a1
	or c1,c0,c0;	ldx (i1)+1,c1
	ashl a1,b0,c0;	stx c0,(i5)+1
	
$2:
	sub null,ones,b0; ldx (i1)-2,null
	and a0,b0,a0
$1:
	and a0,a0,a0;	ldx (I6),le
	ldx (i5),c0;	ldy (I6)-1,ls
	jzs $3
	and c0,b1,c0;	ldx (i1)+1,c1

	ashl c1,b0,c1
	or c1,c0,c0
	stx c0,(i5)+1

$3:
	ldx (I6)-1,a1;	ldy (I6),lc
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
