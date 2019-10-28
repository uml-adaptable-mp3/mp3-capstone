/** \file hwLocksAsm.s Release hardware lock associated with XPERIP. */

#include <vs1005g.h>
#include <hwLocks.h>


/*
   XPERIP interrupt routine.
   Never call explicitly!
   Set up an interrupt vector INT_XPERIP to this by writing
   WriteIMem((void *)(0x20+INTV_XPERIP),
       0x2a000000e+((u_int32)((u_int16)XPeripIntAsm) << 6));
   This routine calls auto void XPeripIntC(register __a0 u_int16 intNumber);
   where intNumber is the number of the bit in XP_ST
*/
	.sect code,XPeripIntAsm
	.export _XPeripIntAsm
_XPeripIntAsm:
	stx i7,(i6)+1;	sty lr0,(i6)
	stx lr1,(i6)+1;	sty mr0,(i6)
	stx a2,(i6)+1;	sty b2,(i6)
	stx c2,(i6)+1;	sty d2,(i6)
	stx a1,(i6)+1;	sty a0,(i6)
	.import _Forbid
	call _Forbid			// void Forbid(void)
	add null,p,a;	stx i5,(i6)+1

	stx a1,(i6)+1;	sty a0,(i6)
	stx b1,(i6);	sty b0,(i6)

	ldc MR_INT,mr0

	// Disable XPERIP interrupt at priority 2
	ldc INT_ENABLE0_HP,i5
	ldc ~INTF_XPERIP,a0
	ldy (i5)+(INT_ENABLE0_LP-INT_ENABLE0_HP),b1
	and b1,a0,b1;	ldy (i5)+(INT_ENABLE0_HP-INT_ENABLE0_LP),b0
	and b0,a0,b0;	sty b1,(i5)+(INT_ENABLE0_LP-INT_ENABLE0_HP)
	sty b0,(i5)+(INT_ENABLE0_HP-INT_ENABLE0_LP)

	ldc INT_GLOB_ENA,i5
	j $2
	sty i5,(i5)		// Enable all other interrupts except XPERIP

$1:
	// All calls in Forbid state but with all other interrupts allowed
	// except XPERIP
	.import _XPeripIntC
	sub null,ones,a1
	ashl a1,a0,a1
	or a1,b0,a1
	call _XPeripIntC	// auto void XPeripIntC(register __a0 intNo)
	sty a1,(i5);	ldx (i6)+1,null

$2:
	ldc XP_ST,i5
	ldc XP_ST_ALL_INTS,a1
	ldc XP_ST_INT_ENA,b1
	ldy (i5),a0
	and a0,b1,b0
	and a0,a1,a0
	exp a0,a0
	ldc 14,a1
	jzc $1
	sub a1,a0,a0

	.import _Permit
	call _Permit			// void Permit(void)
	ldc INTF_XPERIP,a0

	// Re-enable XPERIP interrupt at priority 2
	ldc INT_ENABLE0_HP,i5
	ldy (i5),b0
	or b0,a0,b0
	sty b0,(i5)

	ldx (i6)-1,b1;	ldy (i6),b0
	ldx (i6)-1,a1;	ldy (i6),a0
	resp a0,a1
	ldx (i6)-1,i5
	ldx (i6)-1,a1;	ldy (i6),a0
	ldx (i6)-1,c2;	ldy (i6),d2
	ldx (i6)-1,a2;	ldy (i6),b2
	ldx (i6)-1,lr0;	ldy (i6),mr0
	jr
	ldx (i6)-1,i7;	ldy (i6),lr0







#if 0
	.sect code,ReleaseXPeripHwLockInt
	.export _ReleaseXPeripHwLockInt
_ReleaseXPeripHwLockInt:
	stx i7,(i6)+1;	sty lr0,(i6)
	stx lr1,(i6)+1;	sty mr0,(i6)
	stx a2,(i6)+1;	sty b2,(i6)
	stx c2,(i6)+1;	sty d2,(i6)
	stx a1,(i6)+1;	sty a0,(i6)
	.import _Forbid
	call _Forbid			// void Forbid(void)
	add null,p,a;	stx i5,(i6)+1

	stx a1,(i6)+1;	sty a0,(i6)
	stx b1,(i6)+1;	sty b0,(i6)
	stx c1,(i6)+1;	sty c0,(i6)
	stx d1,(i6)+1;	sty d0,(i6)
	ldc INT_GLOB_ENA,i5
	and b,null,b;	sty i5,(i5)	// slb; interrupts allowed from here on
	and c,null,c			// slio

#if 1
	// Debug
	.import _xPeripCounter
	ldc _xPeripCounter,i5
	ldx (i5),d0
	sub d0,ones,d0
	stx d0,(i5)
#endif

	ldc hi(HLP_SD),d1	// hi(slp)
	
	.import _ReleaseHwLocksBIP
	call _ReleaseHwLocksBIP	// auto, (i6) added to earlier
	ldc lo(HLP_SD),d0	// lo(slp)

	.import _Permit
	call _Permit			// void Permit(void)
	ldx (i6)-1,d1;	ldy (i6),d0
	ldx (i6)-1,c1;	ldy (i6),c0
	ldx (i6)-1,b1;	ldy (i6),b0
	ldx (i6)-1,a1;	ldy (i6),a0
	resp a0,a1
	ldx (i6)-1,i5
	ldx (i6)-1,a1;	ldy (i6),a0
	ldx (i6)-1,c2;	ldy (i6),d2
	ldx (i6)-1,a2;	ldy (i6),b2
	ldx (i6)-1,lr0;	ldy (i6),mr0
	jr
	ldx (i6)-1,i7;	ldy (i6),lr0
#endif





	.end
