	.sect code,GetI6
	.export _GetI6
_GetI6:
	jr
	mv i6,a0

	.sect code,GetLR0
	.export _GetLR0
_GetLR0:
	jr
	mv lr0,a0

	.end
