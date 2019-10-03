/**
	\file hiresAsm.s
	
*/
#include <vs1005h.h>
#include <vstypes.h>


#ifdef ASM

/*
void FindAudioLevels16(register __i0 s_int16 *d,
     		       register __i2 u_int16 __mem_y *max,
		       register __a0 s_int16 stereoSamples);
*/
	.sect code,FindAudioLevels16
	.export _FindAudioLevels16
_FindAudioLevels16:
	add a0,ones,a0;	ldx (i6)+1,null
	stx i2,(I6)+1;	sty b0,(I6)
	stx a1,(I6)+1;	sty lc,(I6)
	
	jns $9
	stx d0,(I6);	sty d1,(I6)

	ldy (i2)+1,d0			// d0 = max[0]
	ldy (i2)-1,d1;	ldx (i6)+1,null	// d1 = max[1]

	ldx (i0)+1,a1;	sty ls,(I6)
	abs a1,a1;	stx le,(I6)

	loop a0,$8-1
	sub d0,a1,b0;	ldx (i0)+1,a0

	nop
	jcs $1
	abs a0,a0

	add a1,null,d0
$1:
	sub d1,a0,b0;	ldx (i0)+1,a1
	nop
	jcs $2
	abs a1,a1

	add a0,null,d1
$2:
	sub d0,a1,b0;	ldx (i0)+1,a0

$8:
	sty d0,(i2)+1	// Store max[0]
	sty d1,(i2)-1	// Store max[1]

	ldx (I6)-1,le;	ldy (I6),ls
$9:
	ldx (I6)-1,d0;	ldy (I6),d1
	ldx (I6)-1,a1;	ldy (I6),lc
	jr
	ldx (I6)-1,i2;	ldy (I6),b0


/*
void FindAudioLevels32(register __i0 s_int32 *d,
     		       register __i2 u_int32 __mem_y *max,
		       register __a0 s_int16 stereoSamples);
*/
	.sect code,FindAudioLevels32
	.export _FindAudioLevels32
_FindAudioLevels32:
	add a0,ones,a0;	ldx (i6)+1,null
	stx b0,(I6)+1;	sty b1,(I6)
	stx i2,(I6)+1;	sty lc,(I6)	// return address
	stx c0,(I6)+1;	sty c1,(I6)
	stx d0,(I6)+1;	sty d1,(I6)

	jns $9
	stx le,(I6);	sty ls,(I6)

	ldy (i2)+1,c0;	ldx (i6)+1,null	// c = max[0]
	ldy (i2)+1,c1;	stx a1,(i6)+1
	ldy (i2)+1,d0;	ldx (i0)+1,b0	// d = max[1]
	ldy (i2)-3,d1;	ldx (i0)+1,b1

	loop a0,$8-1
	abs b,b;	sty c0,(i6)+1

	sub c,b,c;	sty c1,(i6)
	ldx (i0)+1,a0;	ldy (i6)-1,c1
	jcs $1
	ldx (i0)+1,a1;	ldy (i6),c0

	add b,null,c
$1:
	abs a,a;	sty d0,(i6)+1
	sub d,a,d;	sty d1,(i6)
	ldx (i0)+1,b0;	ldy (i6)-1,d1
	jcs $2
	ldx (i0)+1,b1;	ldy (i6),d0

	add a,null,d
$2:
	abs b,b;	sty c0,(i6)+1

$8:
	sty c0,(i2)+1	// Store max[0]
	sty c1,(i2)+1;	ldx (i6)-2,null
	sty d0,(i2)+1	// Store max[1]
	sty d1,(i2)-3;	ldx (i6)-1,a1

$9:
	ldx (I6)-1,le;	ldy (I6),ls
	ldx (I6)-1,d0;	ldy (I6),d1
	ldx (I6)-1,c0;	ldy (I6),c1
	ldx (I6)-1,i2;	ldy (I6),lc
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
