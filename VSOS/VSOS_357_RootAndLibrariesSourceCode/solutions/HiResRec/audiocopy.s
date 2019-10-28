/**
	\file hiresAsm.s
	
*/
#include <vs1005h.h>
#include <vstypes.h>


#ifdef ASM

/*
void FindAudioLevels2Ch16I(register __i0 s_int16 *d,
     		       register __i2 u_int16 __mem_y *max,
		       register __a0 s_int16 stereoSamples);
*/
	.sect code,FindAudioLevels2Ch16I
	.export _FindAudioLevels2Ch16I
_FindAudioLevels2Ch16I:
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
void FindAudioLevels2Ch32I(register __i0 s_int32 *d,
     		       register __i2 u_int32 __mem_y *max,
		       register __a0 s_int16 stereoSamples);
*/
	.sect code,FindAudioLevels2Ch32I
	.export _FindAudioLevels2Ch32I
_FindAudioLevels2Ch32I:
	add a0,ones,a0;	ldx (i6)+1,null
	stx b0,(I6)+1;	sty b1,(I6)
	stx i2,(I6)+1;	sty lc,(I6)
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




/*
void FindAudioLevels4Ch16I(register __i0 s_int16 *d,
     		       register __i2 u_int16 __mem_y *max,
		       register __a0 s_int16 stereoSamples);
*/
	.sect code,FindAudioLevels4Ch16I
	.export _FindAudioLevels4Ch16I
_FindAudioLevels4Ch16I:
	add a0,ones,a0;	ldx (i6)+1,null
	stx i2,(I6)+1;	sty b0,(I6)
	stx a1,(I6)+1;	sty lc,(I6)
	stx c0,(I6)+1;	sty c1,(I6)
	
	jns $9
	stx d0,(I6);	sty d1,(I6)

	ldy (i2)+1,d0			// d0 = max[0]
	ldy (i2)+1,d1;	ldx (i6)+1,null	// d1 = max[1]
	ldy (i2)+1,c0	    		// c0 = max[2]
	ldy (i2)-3,c1	    		// c1 = max[3]

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
	sub c0,a1,b0;	ldx (i0)+1,a0
	nop
	jcs $3
	abs a0,a0

	add a1,null,c0
$3:
	sub c1,a0,b0;	ldx (i0)+1,a1
	nop
	jcs $4
	abs a1,a1

	add a0,null,c1
$4:
	sub d0,a1,b0;	ldx (i0)+1,a0

$8:
	sty d0,(i2)+1	// Store max[0]
	sty d1,(i2)+1	// Store max[1]
	sty c0,(i2)+1	// Store max[2]
	sty c1,(i2)+1	// Store max[3]

	ldx (I6)-1,le;	ldy (I6),ls
$9:
	ldx (I6)-1,d0;	ldy (I6),d1
	ldx (I6)-1,c0;	ldy (I6),c1
	ldx (I6)-1,a1;	ldy (I6),lc
	jr
	ldx (I6)-1,i2;	ldy (I6),b0



/*
void FindAudioLevels4Ch32I(register __i0 s_int32 *d,
     		       register __i2 u_int32 __mem_y *max,
		       register __a0 s_int16 stereoSamples);
*/
	.sect code,FindAudioLevels4Ch32I
	.export _FindAudioLevels4Ch32I
_FindAudioLevels4Ch32I:
	add a0,ones,a0;	ldx (i6)+1,null
	stx b0,(I6)+1;	sty b1,(I6)
	stx i2,(I6)+1;	sty lc,(I6)
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
	ldx (i0)+5,a1;	ldy (i6),c0

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



/*
void ApplySoftVol32(register __i0 s_int32 *d, register __a0 u_int16 volume,
		    register __a1 s_int16 n);
*/
	.sect code,ApplySoftVol32
	.export _ApplySoftVol32
_ApplySoftVol32:
	add a1,ones,a1;	ldx (i6)+1,null
	stx mr0,(I6)+1;	sty lc,(I6)
	stx b0,(I6)+1;	sty b1,(I6)
	stx c0,(I6)+1;	sty c1,(I6)
	stx d0,(I6)+1;	sty d1,(I6)
	stx le,(I6)+1;	sty ls,(I6)

	ldx (i0)+1,b0
	jns $1
	ldc MR_SAT,mr0

	and d,null,d;	ldx (i0)+1,b1
	muluu a0,b0;	stx d0,(i6);	sty d1,(i6)

	loop a1,$1-1
	macus a0,b1,d

	rnd d,c0;	ldx (i0)+1,b0
	and c1,null,c1;	ldx (i0)-3,b1
	add c,p,c;	ldx (i6),d0;	ldy (i6),d1
	muluu a0,b0;	stx c0,(i0)+1
	macus a0,b1,d;	stx c1,(i0)+3
	
$1:
	ldx (i6)-1,null
	ldx (I6)-1,le;	ldy (I6),ls
	ldx (I6)-1,d0;	ldy (I6),d1
	ldx (I6)-1,c0;	ldy (I6),c1
	ldx (I6)-1,b0;	ldy (I6),b1
	jr
	ldx (I6)-1,mr0;	ldy (I6),lc


/*
void ApplySoftVol32Shift(register __i0 s_int32 *d, register __a0 u_int16 volume,
			 register __a1 s_int16 shiftLeft,
			 register __d0 s_int16 n);
*/
	.sect code,ApplySoftVol32Shift
	.export _ApplySoftVol32Shift
_ApplySoftVol32Shift:
	add a1,null,a1;	ldx (i6)+1,null
	stx mr0,(I6)+1;	sty lc,(I6)
	jzs $30
	stx b0,(I6)+1;	sty b1,(I6)

	stx c0,(I6)+1;	sty c1,(I6)
	stx d0,(I6)+1;	sty d1,(I6)
	ldc MR_SAT,mr0
	add d0,ones,d0;	ldx (i0)+1,b0
	stx le,(I6)+1;	sty ls,(I6)

	jns $9		// n == 0?
	mv d0,ls

	/* shiftLeft > 0 */
	jns $20
	and d,null,d;	ldx (i0)+1,b1

	muluu a0,b0;	stx d0,(i6);	sty d1,(i6)

	loop ls,$1-1
	macus a0,b1,d

	rnd d,c0;	ldx (i0)+1,b0
	and c1,null,c1;	ldx (i0)-3,b1
	add c,p,c;	ldx (i6),d0;	ldy (i6),d1
	ashl c,a1,c
	sat c,c		// Required because shifting up
	muluu a0,b0;	stx c0,(i0)+1
	macus a0,b1,d;	stx c1,(i0)+3

$1:
	j $9
	nop

$20:
	/* shiftLeft < 0 */
	muluu a0,b0;	stx d0,(i6);	sty d1,(i6)

	loop ls,$2-1
	macus a0,b1,d

	rnd d,c0;	ldx (i0)+1,b0
	and c1,null,c1;	ldx (i0)-3,b1
	add c,p,c;	ldx (i6),d0;	ldy (i6),d1
	ashl c,a1,c
	addc c,null,c	// Gives rounding after shifting down
	muluu a0,b0;	stx c0,(i0)+1
	macus a0,b1,d;	stx c1,(i0)+3
$2:

$9:
	ldx (i6)-1,null
	ldx (I6)-1,le;	ldy (I6),ls
	ldx (I6)-1,d0;	ldy (I6),d1
	ldx (I6)-1,c0;	ldy (I6),c1
	ldx (I6)-1,b0;	ldy (I6),b1
	jr
	ldx (I6)-1,mr0;	ldy (I6),lc

$30:
	/* shiftLeft = 0 */
	j _ApplySoftVol32
	add d0,null,a1; ldx (i6)-3,null




/*
void Convert16BitVSDSPTo16BitLE(register __i3 s_int16 *d, register __i0 const s_int16 *s, register __d0 u_int16 stereoSamples);
void Convert16BitLETo16BitVSDSP(register __i3 s_int16 *d, register __i0 const s_int16 *s, register __d0 u_int16 stereoSamples);
*/
	.sect code,Convert16BitVSDSPTo16BitLE
	.export _Convert16BitVSDSPTo16BitLE
	.export _Convert16BitLETo16BitVSDSP
_Convert16BitLETo16BitVSDSP:
_Convert16BitVSDSPTo16BitLE:
	add d0,d0,d0;	ldx (i6)+1,null
	stx a0,(I6)+1;	sty a1,(I6)
	stx d1,(I6)+1;	sty lc,(I6)
	stx le,(I6)+1;	sty ls,(I6)
	add d0,ones,d0;	stx b0,(I6);	sty b1,(I6)
	
	ldc 0,a1
	jns $9
	ldc 8,d1

	ldx (i0)+1,a0
	loop d0,$9-1
	ashl a,d1,b
$1:
	or b1,b0,b0;	ldx (i0)+1,a0
	ashl a,d1,b;	stx b0,(i3)+1

$9:
	ldx (I6)-1,b0;	ldy (I6),b1
	ldx (I6)-1,le;	ldy (I6),ls
	ldx (I6)-1,d1;	ldy (I6),lc
	jr
	ldx (I6)-1,a0;	ldy (I6),a1



/*
void Convert16BitVSDSPTo32BitLE(register __i3 s_int32 *d, register __i0 const s_int16 *s, register __d0 u_int16 stereoSamples);
void Convert16BitLETo32BitVSDSP(register __i3 s_int32 *d, register __i0 const s_int16 *s, register __d0 u_int16 stereoSamples);
*/
	.sect code,Convert16BitVSDSPTo32BitLE
	.export _Convert16BitVSDSPTo32BitLE
	.export _Convert16BitLETo32BitVSDSP
_Convert16BitLETo32BitVSDSP:
_Convert16BitVSDSPTo32BitLE:
	add d0,d0,d0;	ldx (i6)+1,null
	stx a0,(I6)+1;	sty a1,(I6)
	stx d1,(I6)+1;	sty lc,(I6)
	mv i3,b1
	stx le,(I6)+1;	sty ls,(I6)
	add b1,d0,b1;	stx b0,(I6);	sty b1,(I6)
	
	add b1,d0,b1;	mv i0,b0
	add b0,d0,b0;	mv b1,i3

	add d0,ones,d0; mv b0,i0
	ldc 0,a1
	jns $9
	ldc 8,d1

	ldx (i0)-1,null; ldy (i3)-1,null
	ldx (i0)-1,a0
	loop d0,$9-1
	ashl a,d1,b
$1:
	or b1,b0,b0;	ldx (i0)-1,a0
	ashl a,d1,b;	stx b0,(i3)-1
	stx a1,(i3)-1

$9:
	ldx (I6)-1,b0;	ldy (I6),b1
	ldx (I6)-1,le;	ldy (I6),ls
	ldx (I6)-1,d1;	ldy (I6),lc
	jr
	ldx (I6)-1,a0;	ldy (I6),a1




/*
void Convert32BitVSDSPTo16BitLE(register __i3 s_int16 *d, register __i0 const s_int32 *s, register __d0 u_int16 stereoSamples);
void Convert32BitLETo16BitVSDSP(register __i3 s_int16 *d, register __i0 const s_int32 *s, register __d0 u_int16 stereoSamples);
*/
	.sect code,Convert32BitVSDSPTo16BitLE
	.export _Convert32BitVSDSPTo16BitLE
	.export _Convert32BitLETo16BitVSDSP
_Convert32BitVSDSPTo16BitLE:
_Convert32BitLETo16BitVSDSP:
	add d0,d0,d0;	ldx (i6)+1,null
	stx a0,(I6)+1;	sty a1,(I6)
	stx d1,(I6)+1;	sty lc,(I6)
	stx le,(I6)+1;	sty ls,(I6)
	add d0,ones,d0;	stx b0,(I6);	sty b1,(I6)

	ldc 0,a1
	jns $9
	ldc 8,d1

	ldx (i0)+1,null
	ldx (i0)+2,a0
	loop d0,$9-1
	ashl a,d1,b
$1:
	or b1,b0,b0;	ldx (i0)+2,a0
	ashl a,d1,b;	stx b0,(i3)+1

$9:
	ldx (I6)-1,b0;	ldy (I6),b1
	ldx (I6)-1,le;	ldy (I6),ls
	ldx (I6)-1,d1;	ldy (I6),lc
	jr
	ldx (I6)-1,a0;	ldy (I6),a1


/*
void Convert32BitVSDSPTo24BitLE(register __i3 void *d, register __i0 const s_int32 *s, register __d0 u_int16 stereoSamples);
*/
	.sect code,Convert32BitVSDSPTo24BitLE
	.export _Convert32BitVSDSPTo24BitLE
_Convert32BitVSDSPTo24BitLE:
	add d0,ones,d0;	ldx (i6)+1,null
	stx a0,(I6)+1;	sty a1,(I6)
	stx d1,(I6)+1;	sty lc,(I6)
	stx b0,(I6)+1;	sty b1,(I6)
	stx c0,(I6)+1;	sty c1,(I6)
	stx le,(I6)+1;	sty ls,(I6)
	
	jns $9
	and a1,null,a1;	stx i2,(I6)+1

	ldc 0xff00,b1
	ldc 0x00ff,b0
	stx b1,(i6);	sty b0,(i6)
	ldc 8,c1

	ldx (i0)+1,a0
	ldc 1,i2
	and a0,b1,c0;	ldy (i6),b1

	loop d0,$1-1
	ldx (i0)+1,b0
	
	and b0,b1,d1;	ldx (i6),b1
	or d1,c0,c0;	ldx (i0)+1,a0
	and b0,b1,c0;	stx c0,(i3)+1
	ashl a,c1,d;	ldx (i0)+2,a0
	or d1,c0,c0;	ldx (i0)-1,b0
	ashl a,c1,d;	stx c0,(i3)+1
	or d1,d0,c0;	ldx (i0)+2,a0
	and a0,b1,c0;	stx c0,(i3)*;	ldy (i6),b1
$1:

$9:
	ldx (I6)-1,null
	ldx (I6)-1,i2
	ldx (I6)-1,le;	ldy (I6),ls
	ldx (I6)-1,c0;	ldy (I6),c1
	ldx (I6)-1,b0;	ldy (I6),b1
	ldx (I6)-1,d1;	ldy (I6),lc
	jr
	ldx (I6)-1,a0;	ldy (I6),a1


/*
void Convert24BitLETo32BitVSDSP(register __i3 s_int32 *d, register __i0 const void *s, register __d0 u_int16 stereoSamples);
*/
	.sect code,Convert24BitLETo32BitVSDSP
	.export _Convert24BitLETo32BitVSDSP
_Convert24BitLETo32BitVSDSP:
	ldx (i6)+1,null
	stx a0,(I6)+1;	sty a1,(I6)
	stx d1,(I6)+1;	sty lc,(I6)
	stx b0,(I6)+1;	sty b1,(I6)
	stx le,(I6)+1;	sty ls,(I6)
	add d0,d0,a0;	stx c0,(I6);	sty c1,(I6)

	add a0,a0,a0;	mv i3,a1
	add a1,a0,a1;	mv i0,b1
	sub a0,d0,a0;	mv a1,i3
	add b1,a0,b1
	add d0,ones,d0;	mv b1,i0	// i0 = inP = inP+stereoSamples*3
	ldx (i3)-1,null;   ldy (i0)-1,null
	jns $9
	and a1,null,a1; ldx (i0)-1,a0

	ldc 0xff00,b1
	ldc 0x00ff,b0
	ldc 8,c1

	loop d0,$1-1
	ashl a,c1,d

	or d1,d0,c0;	ldx (i0)-1,a0
	ashl a,c1,d;	stx c0,(i3)-1
	and a0,b1,c0;	ldx (i0)-1,d1
	and d1,b0,d0;	stx d0,(i3)-1
	or d0,c0,c0;	ldx (i0)-1,a0
	and d1,b1,c0;	stx c0,(i3)-1
	ashl a,c1,d;	stx c0,(i3)-1
$1:

$9:
	ldx (I6)-1,c0;	ldy (I6),c1
	ldx (I6)-1,le;	ldy (I6),ls
	ldx (I6)-1,b0;	ldy (I6),b1
	ldx (I6)-1,d1;	ldy (I6),lc
	jr
	ldx (I6)-1,a0;	ldy (I6),a1



/******************************/



/*
void Convert16BitVSDSPTo16BitLEInPlace(register __i3 u_int16 *d,
				register __d0 u_int16 stereoSamples);
void Convert16BitLETo16BitVSDSPInPlace(register __i3 u_int16 *d,
				register __d0 u_int16 stereoSamples);
*/
	.sect code,Convert16BitVSDSPTo16BitLEInPlace
	.export _Convert16BitVSDSPTo16BitLEInPlace
	.export _Convert16BitLETo16BitVSDSPInPlace
_Convert16BitLETo16BitVSDSPInPlace:
_Convert16BitVSDSPTo16BitLEInPlace:
	ldx (i6)+1,null
	stx lr0,(i6); sty i0,(i6)
	call _Convert16BitLETo16BitVSDSP
	mv i3,i0

	ldx (i6)-1,lr0; ldy (i6),i0
	jr
	nop

/*
void Convert16BitVSDSPTo32BitLEInPlace(register __i3 u_int16 *d,
				register __d0 u_int16 stereoSamples);
void Convert16BitLETo32BitVSDSPInPlace(register __i3 u_int16 *d,
				register __d0 u_int16 stereoSamples);
*/
	.sect code,Convert16BitVSDSPTo32BitLEInPlace
	.export _Convert16BitVSDSPTo32BitLEInPlace
	.export _Convert16BitLETo32BitVSDSPInPlace
_Convert16BitLETo32BitVSDSPInPlace:
_Convert16BitVSDSPTo32BitLEInPlace:
	ldx (i6)+1,null
	stx lr0,(i6); sty i0,(i6)
	call _Convert16BitLETo32BitVSDSP
	mv i3,i0

	ldx (i6)-1,lr0; ldy (i6),i0
	jr
	nop



/*
void Convert32BitVSDSPTo16BitLEInPlace(register __i3 u_int16 *d,
				register __d0 u_int16 stereoSamples);
void Convert32BitLETo16BitVSDSPInPlace(register __i3 u_int16 *d,
				register __d0 u_int16 stereoSamples);
*/
	.sect code,Convert32BitVSDSPTo16BitLEInPlace
	.export _Convert32BitVSDSPTo16BitLEInPlace
	.export _Convert32BitLETo16BitVSDSPInPlace
_Convert32BitVSDSPTo16BitLEInPlace:
_Convert32BitLETo16BitVSDSPInPlace:
	ldx (i6)+1,null
	stx lr0,(i6); sty i0,(i6)
	call _Convert32BitLETo16BitVSDSP
	mv i3,i0

	ldx (i6)-1,lr0; ldy (i6),i0
	jr
	nop


/*
void Convert32BitVSDSPTo24BitLEInPlace(register __i3 u_int16 *d,
				register __d0 u_int16 stereoSamples);
*/
	.sect code,Convert32BitVSDSPTo24BitLEInPlace
	.export _Convert32BitVSDSPTo24BitLEInPlace
_Convert32BitVSDSPTo24BitLEInPlace:
	ldx (i6)+1,null
	stx lr0,(i6); sty i0,(i6)
	call _Convert32BitVSDSPTo24BitLE
	mv i3,i0

	ldx (i6)-1,lr0; ldy (i6),i0
	jr
	nop


/*
void Convert24BitLETo32BitVSDSPInPlace(register __i3 u_int16 *d,
				register __d0 u_int16 stereoSamples);
*/
	.sect code,Convert24BitLETo32BitVSDSPInPlace
	.export _Convert24BitLETo32BitVSDSPInPlace
_Convert24BitLETo32BitVSDSPInPlace:
	ldx (i6)+1,null
	stx lr0,(i6); sty i0,(i6)
	call _Convert24BitLETo32BitVSDSP
	mv i3,i0

	ldx (i6)-1,lr0; ldy (i6),i0
	jr
	nop


/******************************/



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




/*
void Convert32ITo16IInPlace(register __i1 s_int16 *d, register __d0 stereoSamples);
*/
	.sect code,Convert32ITo16IInPlace
	.export _Convert32ITo16IInPlace
_Convert32ITo16IInPlace:
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
void Convert16ITo32IInPlace(register __i1 s_int16 *d, register __d0 stereoSamples);
*/
	.sect code,Convert16ITo32IInPlace
	.export _Convert16ITo32IInPlace
_Convert16ITo32IInPlace:
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






/*
void Matrix4ChTo2Ch16Bit(register __i0 s_int16 *d, register __i2 s_int16 *s, register __d0 s_int16 stereoSamples);
*/
	.sect code,Matrix4ChTo2Ch16Bit
	.export _Matrix4ChTo2Ch16Bit
_Matrix4ChTo2Ch16Bit:
	add d0,ones,d0; ldx (i6)+1,null
	stx b0,(I6)+1;	sty b1,(I6)
	stx le,(I6)+1;	sty ls,(I6)
	stx mr0,(I6);	sty lc,(I6)

	jns $1
	ldc MR_SAT|MR_INT,mr0

	loop d0,$1-1
	ldx (i2)+2,b0

	ldx (i2)-1,b1
	add b0,b1,b1;	ldx (i2)+2,b0
	stx b1,(i0)+1
	ldx (i2)+1,b1
	add b0,b1,b1;	ldx (i2)+2,b0
	stx b1,(i0)+1
$1:
	ldx (I6)-1,mr0;	ldy (I6),lc
	ldx (I6)-1,le;	ldy (I6),ls
	jr
	ldx (I6)-1,b0;	ldy (I6),b1



/*
void Matrix4ChTo2Ch32Bit(register __i0 s_int16 *d, register __i2 s_int16 *s, register __d0 s_int16 stereoSamples);
*/
	.sect code,Matrix4ChTo2Ch32Bit
	.export _Matrix4ChTo2Ch32Bit
_Matrix4ChTo2Ch32Bit:
	add d0,ones,d0; ldx (i6)+1,null
	stx b0,(I6)+1;	sty b1,(I6)
	stx c0,(I6)+1;	sty c1,(I6)
	stx le,(I6)+1;	sty ls,(I6)
	stx mr0,(I6);	sty lc,(I6)

	jns $1
	ldc MR_SAT|MR_INT,mr0

	ldx (i2)+1,b0
	loop d0,$1-1
	ldx (i2)+3,b1

	ldx (i2)+1,c0
	ldx (i2)-3,c1
	add b,c,c;	ldx (i2)+1,b0
	sat c,c;	ldx (i2)+3,b1
	stx c0,(i0)+1
	stx c1,(i0)+1

	ldx (i2)+1,c0
	ldx (i2)+1,c1
	add b,c,c;	ldx (i2)+1,b0
	sat c,c;	ldx (i2)+3,b1
	stx c0,(i0)+1
	stx c1,(i0)+1

$1:
	ldx (I6)-1,mr0;	ldy (I6),lc
	ldx (I6)-1,le;	ldy (I6),ls
	ldx (I6)-1,c0;	ldy (I6),c1
	jr
	ldx (I6)-1,b0;	ldy (I6),b1



#endif /* ASM */


	.end



#if 0
	/********* EVERYTHING FROM NOW ON IS PURELY DECORATIONAL!!! **********/

/*
	void Generic(void) - 2018 edition
*/
	.sect code,Generic
	.export _Generic
_Generic:
	ldx (i6)+1,null
	stx i0,(I6)+1;	sty i1,(I6)
	stx i2,(I6)+1;	sty i3,(I6)
	stx le,(I6)+1;	sty ls,(I6)
	stx lr0,(I6)+1;	sty lc,(I6)
	stx a0,(I6)+1;	sty a1,(I6)
	stx b0,(I6)+1;	sty b1,(I6)
	stx c0,(I6)+1;	sty c1,(I6)
	stx d0,(I6)+1;	sty d1,(I6)
	stx i4,(I6)

	ldx (I6)-1,i4
	ldx (I6)-1,d0;	ldy (I6),d1
	ldx (I6)-1,c0;	ldy (I6),c1
	ldx (I6)-1,b0;	ldy (I6),b1
	ldx (I6)-1,a0;	ldy (I6),a1
	ldx (I6)-1,lr0;	ldy (I6),lc
	ldx (I6)-1,le;	ldy (I6),ls
	ldx (I6)-1,i2;	ldy (I6),i3
	jr
	ldx (I6)-1,i0;	ldy (I6),i1
#endif
