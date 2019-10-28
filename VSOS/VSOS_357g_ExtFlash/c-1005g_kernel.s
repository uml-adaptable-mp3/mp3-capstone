#include <vs1005g.h>
        
	.sect code,startup
	.org 0x80
	.import __stack
  
        
        
        ldc 0x200,mr0
        ldc 0x1f,ls
        ldc __stack,a0
        .import OSInit
        call OSInit
        ldc DEBUG_STACK,a1
	.import _main
	j _main
	ldc _exit,lr0

_exit:
	j 0x8000
        nop

	.end
	
