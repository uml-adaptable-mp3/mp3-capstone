#include <vstypes.h>
#include <vs1005h.h>


/*
   SAR interrupt routine.
   Never call explicitly!
   This routine calls void IntSarC(void);

   Set the interrupt vector from C as follows:

   extern u_int16 IntSarVector;
   [...]
   WriteIMem((void *)(0x20+INTV_SAR), ReadIMem((void *)(&IntSarVector)));

   After setting the interrupt vector, activate the interrupt as follows
     (example for interrupt priority 1):
   PERIP(INT_ENABLE1_LP) |= INTF1_SAR;
   PERIP(INT_ENABLE1_HP) &= ~INTF1_SAR;

   Before exiting your application, deactivate the interrupt as follows:
   PERIP(INT_ENABLE1_LP) &= ~INTF1_SAR;
   PERIP(INT_ENABLE1_HP) &= ~INTF1_SAR;
*/
	.sect code,IntSarVector
	.export _IntSarVector
_IntSarVector:
	jmpi _IntSarAsm,(i6)+1

	.sect code,IntSarAsm
	.export _IntSarAsm
_IntSarAsm:
	stx i7,(i6)+1;	sty mr0,(i6)
	stx lr0,(i6)+1;	sty i5,(i6)
	stx a2,(i6)+1;	sty b2,(i6)
	stx c2,(i6)+1;	sty d2,(i6)
	stx a1,(i6)+1;	sty a0,(i6)
	add null,p,a
	stx a1,(i6);	sty a0,(i6)

	.import _IntSarC
	call _IntSarC
	ldc MR_INT,mr0

	ldx (i6)-1,a1;	ldy (i6),a0
	resp a0,a1
	ldx (i6)-1,a1;	ldy (i6),a0
	ldx (i6)-1,c2;	ldy (i6),d2
	ldx (i6)-1,a2;	ldy (i6),b2
	ldx (i6)-1,lr0;	ldy (i6),i5
	ldy (I6),mr0
	ldc INT_GLOB_ENA,i7
	reti
	ldx (i6)-1,I7;	sty i7,(i7)



	.end
