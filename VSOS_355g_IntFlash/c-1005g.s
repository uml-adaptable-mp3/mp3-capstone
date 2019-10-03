	.sect code,startup
	.org 0x80
	.import __stack
	ldc __stack,i6
	ldc 0x200,mr0
	.import _main
	j _main
	ldc _exit,lr0

_exit:
	halt
	nop
	nop
	nop
	jmpi _exit

	.end
	