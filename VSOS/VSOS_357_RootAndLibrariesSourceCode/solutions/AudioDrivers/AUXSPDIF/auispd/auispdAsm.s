/**
	\file auispdAsm.s

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



	.sect code,SPDInterrupt
	.export _SPDInterruptVector
_SPDInterruptVector:
	jmpi _SPDInterrupt,(i6)+1
	.export _SPDInterrupt
_SPDInterrupt:
	stx I2,(I6)+1;	sty MR0,(i6)
	stx I0,(I6)+1;	sty I1,(i6)
	ldc SP_LDATA_LSB,i1
	.import _audioIn, _audioOut
	ldc _audioIn+IFLAGS,i0
	stx A2,(I6)+1;	sty B2,(i6)
	stx A0,(I6)+1;	sty A1,(i6)
	stx B0,(I6)+1;	sty B1,(i6)

	ldc MR_INT,mr0

	ldc AIN_32BIT,b0
	ldx (i0)+(-IFLAGS+IWR),a0; sty i3,(I6)	// a0 = flags
	and a0,b0,a0
	ldx (i0)+(-IWR+IMODIFIER),i2; ldy (i1)+1,b0 // i2 = wr
	jzs $11	// Jump if 16-bit samples
	ldx (i0)+(-IMODIFIER+IERROR0),i3; ldy (i1)+1,b1 // i3=mod

$10:
	// 32-bit samples
	sty b0,(i2)*;	ldx (i0)+(-IERROR0+ICOUNTERL),null
	sty b1,(i2)*;	ldx (i0)+(-ICOUNTERL+ICOUNTERH),b0
	ldy (i1)+1,a0;	ldx (i0)+(-ICOUNTERH+ICOUNTERH),b1
	ldy (i1)-3,a1;	sub b,ones,b
	sty a0,(i2)*;	stx b1,(i0)+(-ICOUNTERH+ICOUNTERL)
	j $12
	sty a1,(i2)*;	stx b0,(i0)+(-ICOUNTERL+IRD)

$11:
	// 16-bit samples
	and a1,null,a1;	ldx (i0),a0	// a0 = error[0]
	add b,a,a;	ldy (i1)+1,b0	// Error propagation
	sat a,a;	ldy (i1)-3,b1	// Needs to saturate result
	sty a1,(i2)*;	stx a0,(i0)+(-IERROR0+IERROR1)
	and a1,null,a1;	ldx (i0)+(-IERROR1+ICOUNTERL),a0	// a0 = error[1]
	add b,a,a;	ldx (i0)+(-ICOUNTERL+ICOUNTERH),b0	// Error propagation
	sat a,a;	ldx (i0)+(-ICOUNTERH+IERROR1),b1		// Needs to saturate result
	sub b,ones,b;	stx a0,(i0)+(-IERROR1+ICOUNTERH)
	sty a1,(i2)*;	stx b1,(i0)+(-ICOUNTERH+ICOUNTERL)
	stx b0,(i0)+(-ICOUNTERL+IRD)

$12:
	ldx (i0)+(-IRD+IWR),a0
	mv i2,b1
	
	sub a0,b1,b1;	ldy (I6)-1,i3
	ldx (I6)-1,B0	; ldy (I6),B1
	jzs $20
	ldx (I6)-1,A0	; ldy (I6),A1

	stx i2,(i0)	// Update audioIn.wr if buffer not full

$13:
	ldx (I6)-1,A2	; ldy (I6),B2
	ldx (I6)-1,I0	; ldy (I6),I1
	ldy (I6),MR0
	ldc INT_GLOB_ENA,i2
	reti
	ldx (i6)-1,I2;	sty i2,(i2)

	// Overflow / Underflow
$20:
	ldx (i6)+1,null;	ldy (i0)+(-IWR+IERROR1),null
	stx a0,(i6);		ldy (i0)+(-IERROR1+IOVERL),null
	ldx (i0)+(-IOVERL+IOVERH),a0;	sty a1,(i6)
	sub a0,ones,a0;			ldx (i0)+(-IOVERH+IOVERL),a1
	addc a1,null,a1;		stx a0,(i0)+(-IOVERL+IOVERH)
	stx a1,(i0)+(-IOVERH+IOVERL)
	j $13
	ldx (i6)-1,a0;	ldy (i6),a1




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
