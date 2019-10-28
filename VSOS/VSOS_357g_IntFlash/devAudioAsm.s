/**
	\file devAudioAsm.s
	
*/
#include <vs1005g.h>
#include <vstypes.h>
#include <devAudio.h>


#ifdef ASM


	.import _adc	// X memory

#if 1
	.sect data_x,init_x
	.export _huuhaa
_huuhaa:
	.uword 0x1234,0x5678
#endif

	.sect code,SrcInterrupt
	.export _SrcInterruptVector
_SrcInterruptVector:
	jmpi _SrcInterrupt,(i6)+1
	.export _SrcInterrupt
_SrcInterrupt:
	stx i0,(I6)+1;	sty i1,(i6)
	stx i2,(I6)+1;	sty i3,(i6)

	.import _src
	ldc _src+SRC_WR_OFFSET,i0
	ldc SRC_LEFT_LSB,i1
	ldx (i0)+(SRC_MODIFIER_OFFSET-SRC_WR_OFFSET),i2;	sty i5,(I6)
	ldy (i1)+1,i5;	ldx (i0)-(SRC_MODIFIER_OFFSET-SRC_WR_OFFSET),i3
#if 1
	ldy (i1)+1,i5;	stx i5,(i2)*
	ldy (i1)+1,i5;	stx i5,(i2)*
	ldy (i1)+1,i5;	stx i5,(i2)*
	stx i5,(i2)*
#else
	ldx (i2)+1,null;
#endif
	stx i2,(i0);	ldy (I6)-1,I5

	ldx (I6)-1,I2	; ldy (I6),I3
	ldy (I6),i1
	ldc INT_GLOB_ENA,i0
	reti
	ldx (i6)-1,i0;	sty i0,(i0)





	.sect code,SPDifNoVolInterrupt
	.export _SPDifNoVolInterruptVector
_SPDifNoVolInterruptVector:
	jmpi _SPDifNoVolInterrupt,(i6)+1
	.export _SPDifNoVolInterrupt
_SPDifNoVolInterrupt:
	stx i0,(I6)+1;	sty i1,(i6)
	stx i2,(I6)+1;	sty i3,(i6)

	.import _src
	ldc _src+SRC_RD_OFFSET,i0
	ldc SP_LDATA_LSB,i1
	ldx (i0)+(SRC_MODIFIER_OFFSET-SRC_RD_OFFSET),i2;	sty i5,(I6)
	ldx (i0)-(SRC_MODIFIER_OFFSET-SRC_RD_OFFSET),i3
	ldx (i2)*,i5
	ldx (i2)*,i5;	sty i5,(i1)+1
	ldx (i2)*,i5;	sty i5,(i1)+1
	ldx (i2)*,i5;	sty i5,(i1)+1
	    		sty i5,(i1)+1
	stx i2,(i0);	ldy (I6)-1,I5

	ldx (I6)-1,I2	; ldy (I6),I3
	ldy (I6),i1
	ldc INT_GLOB_ENA,i0
	reti
	ldx (i6)-1,i0;	sty i0,(i0)



	.sect code,SPDifVolInterrupt
	.export _SPDifVolInterruptVector
_SPDifVolInterruptVector:
	jmpi _SPDifVolInterrupt,(i6)+1
	.export _SPDifVolInterrupt
_SPDifVolInterrupt:
	stx i0,(I6)+1;	sty i1,(i6)
	stx i2,(i6)+1;	sty i3,(i6)
	stx i5,(i6)+1;	sty mr0,(i6)
	stx a2,(i6)+1;	sty b2,(i6)
	stx a1,(i6)+1;	sty a0,(i6)
	add null,p,a
	stx a1,(i6)+1;	sty a0,(i6)
	stx b1,(i6)+1;	sty b0,(i6)

	ldc MR_INT,mr0

	.import _src
	ldc _src+SRC_RD_OFFSET,i0
	ldc SP_LDATA_LSB,i1
	ldx (i0)+(SRC_MODIFIER_OFFSET-SRC_RD_OFFSET),i2;	sty c0,(I6)
	ldx (i0)+(SRC_VOLUMELIN_OFFSET-SRC_MODIFIER_OFFSET),i3
	ldx (i0)+(SRC_RD_OFFSET-SRC_VOLUMELIN_OFFSET),c0

	and null,a,a;	ldx (i2)*,b0
	muluu c0,b0;	ldx (i2)*,b1
	add null,p,a
	add a1,null,a0;	mv a0,i5
	mv i5,b0;	and null,null,a1
	mulsu b1,c0
	add a,p,a
	add b0,b0,b0
	addc a,a,a
	sat a,b
	and null,a,a;	sty b0,(i1)+1
	ldx (i2)*,b0;	sty b1,(i1)+1
	
	muluu c0,b0;	ldx (i2)*,b1
	add null,p,a;	stx i2,(i0)
	add a1,null,a0;	mv a0,i5
	mv i5,b0;	and null,null,a1
	mulsu b1,c0;	ldy (i6)-1,c0
	add a,p,a
	add b0,b0,b0
	addc a,a,a;	ldy (i6),b0
	sat a,a;	ldx (i6)-1,b1
	sty a0,(i1)+1
	sty a1,(i1)+1

	ldx (i6)-1,a1;	ldy (i6),a0
	resp a0,a1
	ldx (i6)-1,a1;	ldy (i6),a0
	ldx (i6)-1,a2;	ldy (i6),b2
	ldx (i6)-1,i5;	ldy (i6),mr0
	ldx (i6)-1,i2;	ldy (i6),i3
	ldy (I6),i1
	ldc INT_GLOB_ENA,i0
	reti
	ldx (i6)-1,i0;	sty i0,(i0)


	.sect code,AdcInterrupt
	.export _AdcInterruptVector
_AdcInterruptVector:
	jmpi _AdcInterrupt,(i6)+1
	.export _AdcInterrupt
_AdcInterrupt:
	stx I2,(I6)+1;	sty MR0,(i6)
	stx I0,(I6)+1;	sty I1,(i6)
	ldc AD_LEFT_LSB,i1
	ldc _adc+(AD_INT_MAC0*ADCCHAN_SIZE+ADCPTR_ADCCHAN+ADCCHAN_FLAGS),i0
	stx A2,(I6)+1;	sty B2,(i6)
	stx A0,(I6)+1;	sty A1,(i6)
	stx B0,(I6)+1;	sty B1,(i6)

	ldc MR_INT,mr0
	
	ldx (i0)+(-ADCCHAN_FLAGS+ADCCHAN_WR),a0; sty i3,(I6)	// a0 = flags
	lsr a0,a0
	ldx (i0)+(-ADCCHAN_WR+ADCCHAN_MODIFIER),i2; ldy (i1)+1,b0 // i2 = wr
	jzs $11	// 16-bit samples
	ldx (i0)+(-ADCCHAN_MODIFIER+ADCCHAN_ERROR0),i3; ldy (i1)+1,b1 // i3=mod
$10:
	sty b0,(i2)*
	sty b1,(i2)*
	ldy (i1)+1,b0
	ldy (i1)+1,b1
	sty b0,(i2)*
	j $12
	sty b1,(i2)*; ldx (i0)+(-ADCCHAN_ERROR0+ADCCHAN_WR),null

$11:
	// 16-bit samples
	and a1,null,a1;	ldx (i0),a0	// a0 = error[0]
	add b,a,a;	ldy (i1)+1,b0	// Error propagation
	sat a,a;	ldy (i1),b1	// Needs to saturate result
	sty a1,(i2)*;	stx a0,(i0)+(-ADCCHAN_ERROR0+ADCCHAN_ERROR1)
	and a1,null,a1;	ldx (i0),a0	// a0 = error[1]
	add b,a,a			// Error propagation
	sat a,a				// Needs to saturate result
	sty a1,(i2)*;	stx a0,(i0)+(-ADCCHAN_ERROR1+ADCCHAN_WR)
	
$12:
	stx i2,(i0)	; ldy (I6)-1,i3
	ldx (I6)-1,B0	; ldy (I6),B1
	ldx (I6)-1,A0	; ldy (I6),A1
	ldx (I6)-1,A2	; ldy (I6),B2
	ldx (I6)-1,I0	; ldy (I6),I1
	ldy (I6),MR0
	ldc INT_GLOB_ENA,i2
	reti
	ldx (i6)-1,I2;	sty i2,(i2)






	.sect code,Dec6Interrupt
	.export _Dec6InterruptVector
_Dec6InterruptVector:
	jmpi _Dec6Interrupt,(i6)+1
	.export _Dec6Interrupt
_Dec6Interrupt:
	stx I2,(I6)+1;	sty MR0,(i6)
	stx I0,(I6)+1;	sty I1,(i6)
	ldc DEC6_LEFT_LSB,i1
	j _AdcInterrupt+4
	ldc _adc+(AD_INT_MAC1*ADCCHAN_SIZE+ADCPTR_ADCCHAN+ADCCHAN_FLAGS),i0








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
