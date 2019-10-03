/** \file timeCount.s Add time counter, then call schedule handler. */
#define ASM
#include <vs1005g.h>

	.sect data_x,init_x
	.export _timeCountAdd
_timeCountAdd:
	.word 1

	.sect code,TimeCountAndScheduleHandler
	.export _TimeCountAndScheduleHandler
_TimeCountAndScheduleHandler:
	ldx (i6)+1,null
	stx mr0,(i6)+1;	sty i7,(i6)
	stx b0,(i6)+1;	sty a0,(i6)
	stx c0,(i6);	sty i5,(i6)

//	TimeCount += timeCountAdd;
	ldc MR_INT,mr0	// Make sure saturation isn't on
	ldc _timeCountAdd,i5
	.import _timeCount
	ldc _timeCount,i7
	ldy (i7)+1,a0;	ldx (i5),c0
	add a0,c0,a0;	ldy (i7)-1,b0
	subc b0,ones,b0;sty a0,(i7)+1
	sty b0,(i7);	ldx (i6),c0

//	PERIP(ANA_CF1) |= ANA_CF1_BTNDIS; //Disable power button reset
	ldc ANA_CF1,i7
	ldc ANA_CF1_BTNDIS,a0
	ldy (i7),b0
	or b0,a0,b0;	ldy (i6)-1,i5
	sty b0,(i7)

	ldx (i6)-1,b0;	ldy (i6),a0
	.import _ScheduleHandler
	j _ScheduleHandler
	ldx (i6)-1,mr0;	ldy (i6),i7

	.end
