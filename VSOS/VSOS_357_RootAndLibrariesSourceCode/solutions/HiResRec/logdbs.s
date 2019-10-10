#if COREVERSION .ge. 3
//See /users/albert/proj/math/log.c and logs.s
	.sect code,logdb
	.export _logdb
_logdb:
	and b1,null,b1	; mv b1,i7
	add b0,null,b0	; ldx (i6)+1,null
	ldc 39-16,a0
	jzc $0
	exp b,c0	; stx c0,(i6)
	ldc 38,c0
$0:	sub c0,a0,a0	; sty mr0,(i6)+1
	stx le,(i6)+1	; sty lc,(i6)
	ashl b0,a0,b0	; stx ls,(i6)
	ldc 512,mr0
	ldc 4-1,ls
	loop ls,$1-1
	muluu b0,b0

	add null,p,b
	nop
	jns $2
	lsl a0,a0	; mv b1,b0

	lsl b0,b0
	sub a0,ones,a0
$2:
	muluu b0,b0
$1:
	ldc -6,b1
	mulsu b1,a0	; ldx (i6)-1,ls
	ldx (i6)-1,le	; ldy (i6),lc
	add null,p,b
	ldc -4,b1
	ashl b0,b1,a0	; mv i7,b1
	jr
	ldx (i6)-1,c0	; ldy (i6),mr0
	//5 cycles lost because of using loop HW, but the code is shorter
#endif

	.end
