	.sect code,SwapWord
	.export _SwapWord
_SwapWord:
	and a0,null,a0	; mv b0,i7
	mv a0,a2
	ldc -8,b0
	ashl a,b0,a	// 00 00XX XX00
	jr
	or a1,a0,a0	; mv i7,b0
	
