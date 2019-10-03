/**
	\file auoosetAsm.s

*/
#include <vs1005h.h>
#include <vstypes.h>
#include <aucommon.h>

#ifdef ASM


#define IWR AIN_WR_OFFSET
#define IRD AIN_RD_OFFSET
#define IBUF AIN_BUF_OFFSET
#define IBUFSIZE AIN_BUFSIZE_OFFSET
#define IMODIFIER AIN_MODIFIER_OFFSET
#define IFLAGS AIN_FLAGS_OFFSET
#define IERROR0 AIN_ERROR0_OFFSET
#define IERROR1 AIN_ERROR1_OFFSET
#define IRATEL AIN_SAMPLE_RATE_OFFSET
#define IRATEH AIN_SAMPLE_RATE_OFFSET
#define ICOUNTERL AIN_SAMPLE_COUNTERL_OFFSET
#define ICOUNTERH AIN_SAMPLE_COUNTERH_OFFSET
#define IOVERL AIN_OVERFLOWL_OFFSET
#define IOVERH AIN_OVERFLOWH_OFFSET
#define IVOLL (AIN_STRUCT_SIZE+0)
#define IVOLR (AIN_STRUCT_SIZE+1)
#define IVOLVAL (AIN_STRUCT_SIZE+2)


#if 0
	.sect data_x,auxi_x
	.export _auxi
_auxi:
	.word 0,0

	.sect code,GetI6
	.export _GetI6
_GetI6:
	jr
	mv i6,a0
#endif



	.sect code,StxInterrupt
	.export _StxInterruptVector
_StxInterruptVector:
	jmpi _StxInterrupt,(i6)+1
	.export _StxInterrupt
_StxInterrupt:
	stx I1,(I6)+1;	sty MR0,(i6)
	stx I4,(I6)+1;	sty I7,(i6)
	ldc SP_LDATA_LSB,i1
	.import _audioOutSPDif
	ldc _audioOutSPDif+IRD,i7
	stx A2,(I6)+1;	sty B2,(i6)
	stx A0,(I6)+1;	sty A1,(i6)
	stx B0,(I6)+1;	sty B1,(i6)

	ldc MR_INT,mr0

	ldx (i7)+(-IRD+IMODIFIER),i4;	sty i5,(i6)
	ldx (i7)+(-IMODIFIER+ICOUNTERL),i5
	ldx (i7)+(-ICOUNTERL+ICOUNTERH),b0;	ldy (i4)*,a0

	// 32-bit samples
	ldy (i4)*,a1
	sty a0,(i1)+1;	ldx (i7)+(-ICOUNTERH+ICOUNTERH),b1
	sty a1,(i1)+1;	sub b,ones,b
	ldy (i4)*,a0;	stx b1,(i7)+(-ICOUNTERH+ICOUNTERL)
	ldy (i4)*,a1;	stx b0,(i7)+(-ICOUNTERL+IRD)
	sty a0,(i1)+1;	ldx (i7)+(-IRD+IWR),null
	sty a1,(i1)+1;	ldx (i7)+(-IWR+IRD),a1

	mv i4,b1
	sub a1,b1,a0;	ldy (I6)-1,I5
	ldx (I6)-1,B0	; ldy (I6),B1
	jzs $14
	ldx (I6)-1,A0	; ldy (I6),A1

	stx i4,(i7)	// Update audioOut.rd if buffer not full

$14:
	ldx (I6)-1,A2	; ldy (I6),B2
	ldx (I6)-1,I4	; ldy (I6),I7
	ldy (I6),MR0
	ldc INT_GLOB_ENA,i1
	reti
	ldx (i6)-1,I1;	sty i1,(i1)







	.sect code,SrcInterrupt
	.export _SrcInterruptVector
_SrcInterruptVector:
	jmpi _SrcInterrupt,(i6)+1
	.export _SrcInterrupt
_SrcInterrupt:
	stx I2,(I6)+1;	sty MR0,(i6)
	stx I0,(I6)+1;	sty I1,(i6)
	ldc SRC_LEFT_LSB,i1
	.import _audioOutSPDif
	ldc _audioOutSPDif+IWR,i0
	stx A2,(I6)+1;	sty B2,(i6)
	stx A0,(I6)+1;	sty A1,(i6)
	stx B0,(I6)+1;	sty B1,(i6)

	ldc MR_INT,mr0

	sty i3,(I6)	// a0 = flags
	ldx (i0)+(-IWR+IMODIFIER),i2; ldy (i1)+1,b0 // i2 = wr
	ldx (i0)+(-IMODIFIER+IERROR0),i3; ldy (i1)+1,b1 // i3=mod

	// 32-bit samples
	sty b0,(i2)*;	ldx (i0)+(-IERROR0+ICOUNTERL),null
	sty b1,(i2)*;	ldx (i0)+(-ICOUNTERL+ICOUNTERH),b0
	ldy (i1)+1,a0;	ldx (i0)+(-ICOUNTERH+ICOUNTERH),b1
	ldy (i1)-3,a1;	sub b,ones,b
	sty a0,(i2)*;	stx b1,(i0)+(-ICOUNTERH+ICOUNTERL)
	sty a1,(i2)*;	stx b0,(i0)+(-ICOUNTERL+IRD)

	ldx (i0)+(-IRD+IWR),a0
	mv i2,b1
	
	sub a0,b1,b1;	ldy (I6)-1,i3
	ldx (I6)-1,B0	; ldy (I6),B1
	jzs $13
	ldx (I6)-1,A0	; ldy (I6),A1

	stx i2,(i0)	// Update audioOut.wr if buffer not full

$13:
	ldx (I6)-1,A2	; ldy (I6),B2
	ldx (I6)-1,I0	; ldy (I6),I1
	ldy (I6),MR0
	ldc INT_GLOB_ENA,i2
	reti
	ldx (i6)-1,I2;	sty i2,(i2)




	.sect code,SrcVolInterrupt
	.export _SrcVolInterruptVector
_SrcVolInterruptVector:
	jmpi _SrcVolInterrupt,(i6)+1
	.export _SrcVolInterrupt
_SrcVolInterrupt:
	stx I2,(I6)+1;	sty MR0,(i6)
	stx I0,(I6)+1;	sty I1,(i6)
	ldc SRC_LEFT_LSB,i1
	.import _audioOutSPDif
	ldc _audioOutSPDif+IVOLR,i0
	stx A2,(I6)+1;	sty B2,(i6)
	stx A0,(I6)+1;	sty A1,(i6)
	add null,p,a
	stx A0,(I6)+1;	sty A1,(i6)
	stx B0,(I6)+1;	sty B1,(i6)
	stx C0,(I6)+1;	sty D0,(i6)

	ldc MR_INT,mr0

	sty i3,(I6)	// a0 = flags
	ldx (i0)+(-IVOLR+IVOLL),d0; ldy (i1)+1,a0
	ldx (i0)+(-IVOLL+IERROR1),c0; ldy (i1)+1,a1

	// 32-bit samples
	muluu c0,a0;	ldx (i0)+(-IERROR1+IWR),null
	add null,p,b;	ldx (i0)+(-IWR+IMODIFIER),i2 // i2 = wr
	mulus c0,a1;	ldx (i0)+(-IMODIFIER+ICOUNTERL),i3 // i3=mod
	ldc 4,c0
	and b1,null,b1; mv b1,b0
	add b,p,b;	ldy (i1)+1,a0
	ashl b,c0,b;	ldy (i1)-3,a1
	sat b,b
	muluu d0,a0;	sty b0,(i2)*
	add null,p,b;	sty b1,(i2)*
	mulus d0,a1
	and b1,null,a1; mv b1,a0
	add a,p,a;	ldx (i0)+(-ICOUNTERL+ICOUNTERH),b0
	ashl a,c0,a;	ldx (i0)+(-ICOUNTERH+ICOUNTERH),b1
	sat a,a	

	sub b,ones,b
	sty a0,(i2)*;	stx b1,(i0)+(-ICOUNTERH+ICOUNTERL)
	sty a1,(i2)*;	stx b0,(i0)+(-ICOUNTERL+IRD)

	ldx (i0)+(-IRD+IWR),a0
	mv i2,b1
	
	sub a0,b1,b1;	ldy (I6)-1,i3
	ldx (I6)-1,C0	; ldy (I6),D0
	ldx (I6)-1,B0	; ldy (I6),B1
	jzs $13
	ldx (I6)-1,A0	; ldy (I6),A1

	stx i2,(i0)	// Update audioOut.wr if buffer not full

$13:
	resp a0,a1
	ldx (I6)-1,A0	; ldy (I6),A1
	ldx (I6)-1,A2	; ldy (I6),B2
	ldx (I6)-1,I0	; ldy (I6),I1
	ldy (I6),MR0
	ldc INT_GLOB_ENA,i2
	reti
	ldx (i6)-1,I2;	sty i2,(i2)




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
