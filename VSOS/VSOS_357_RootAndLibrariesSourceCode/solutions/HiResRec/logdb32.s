#if COREVERSION .ge. 3
//See /users/albert/proj/math/log.c and logs.s

	.import _logdb

	.sect code,logdb32
	.export _logdb32
_logdb32:
	ldc 0x00ff,a0
	sub a0,b1,a0	; ldx (i6)+1,null
	nop
	jcs $0
	add b1,null,b1	; stx lr0,(i6)

	mv b1,b0
	j $2
	ldc 96+96,b1	//96+logdb(uval>>16)

$0:	nop
	jzs $1
	ldc 0,b2
	
	ldc -8,a0
	ashl b,a0,b	//48+logdb(uval>>8)
	j $2
	ldc 96+48,b1

$1:	ldc 96,b1	//logdb(uval)

$2:	call _logdb
	nop
	ldx (i6)-1,lr0
	jr
	add b1,a0,a0
#endif

	.end
