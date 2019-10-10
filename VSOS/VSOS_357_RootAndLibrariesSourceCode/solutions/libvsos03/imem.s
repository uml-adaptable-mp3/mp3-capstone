	.sect code,ReadWriteIMem

	.export _ReadIMem
_ReadIMem:
	ldi (i0),a
	nop
	jr
	nop

	.export _WriteIMem
_WriteIMem:
	sti a,(i0)
	nop
	jr
	nop
	
	.end
