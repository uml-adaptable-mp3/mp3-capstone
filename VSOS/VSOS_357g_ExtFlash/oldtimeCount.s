/** \file timeCount.s Add time counter, then call schedule handler. */
#define ASM
#include <vs1005g.h>

	.sect code,TimeCountAndScheduleHandler
	.export _TimeCountAndScheduleHandler
_TimeCountAndScheduleHandler:
	ldx (i6)+1,null
	stx mr0,(i6)+1;	sty i7,(i6)
	stx b0,(i6);	sty a0,(i6)

#if 1
	ldc 0x200,mr0
	.import _timeCount
	ldc _timeCount,i7
	ldy (i7)+1,a0
	sub a0,ones,a0;	ldy (i7)-1,b0
	subc b0,ones,b0;	sty a0,(i7)+1
	sty b0,(i7)

	ldx (i6)-1,b0;	ldy (i6),a0
	.import _ScheduleHandler
	j _ScheduleHandler
	ldx (i6)-1,mr0;	ldy (i6),i7

	.end
