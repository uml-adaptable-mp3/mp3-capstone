
	
	.sect code,GetLR1
	.export _GetLR1
_GetLR1:
	jr
	mv lr1,a0
	
	.sect code,GetLR0
	.export _GetLR0
_GetLR0:
	jr
	mv lr0,a0
	