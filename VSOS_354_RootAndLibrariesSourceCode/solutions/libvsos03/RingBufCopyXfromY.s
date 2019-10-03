#define ASM
#include <vs1005h.h>
#include <usblowlib.h>	

	.sect code,RingBufCopyXfromY
	.export _RingBufCopyXfromY
_RingBufCopyXfromY:
	add a0,ones,a0
	stx le,(i6)+1	; sty ls,(i6)
	jns $3
	stx lc,(i6)	; sty i1,(i6)
	ldc MAKEMODF(RING_BUF_SIZE),i1
	loop a0,$3-1
	ldy (i0)*,a0
	stx a0,(i2)+1	; ldy (i0)*,a0
$3:
	ldx (i6)-1,lc	; ldy (i6),i1
	ldx (i6)-1,le	; ldy (i6),ls
	jr
	nop

.end
