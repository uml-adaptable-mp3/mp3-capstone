	.sect code,GetI6
	.export _GetI6
	.export _GetLR0
	.export _GetMR0
	
_GetI6:
	jr
	mv i6,a0


_GetLR0:
	jr
	mv lr0,a0

_GetMR0:
	jr
	mv mr0,a0
	
#if 0
	.sect code,ToggleIo
	.export _ToggleIo
_ToggleIo:
	ldc 1<<0,a0
	ldc GPIO0_SET_MASK,i0
	ldc GPIO0_CLEAR_MASK,i1

$1:
	sty a0,(i0)
	nop
	nop
	nop
	nop
	sty a0,(i1)
	nop
	nop
	j $1
	nop
#endif

