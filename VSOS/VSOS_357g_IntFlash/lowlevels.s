#define ASM
#include <vs1005g.h>


#if 0
	.sect code,GetI6
	.export _GetI6
_GetI6:
	jr
	mv i6,a0
#endif

#if 0
	.sect code,Halt
	.export _Halt
_Halt:
	nop
	halt
	nop
	jr
	nop	
#endif
   .sect code,WriteToProgramRam    // 4+1 words
   .export _WriteToProgramRam
   .export _WriteToProgramRamSwapped
   .export _WriteToProgramRam16
   .export _WriteToProgramRam16Swapped
_WriteToProgramRam16Swapped:
_WriteToProgramRamSwapped:   
	or a0,null,a1; mv a1,a0
_WriteToProgramRam16:
_WriteToProgramRam:
   sti a,(i0)
   nop
   jr
   nop

   .sect code,ReadFromProgramRam    // 4 words
   .export _ReadFromProgramRam
_ReadFromProgramRam:
   ldi (i0),a
   nop
   jr
   nop

#if 0
//void *SetJmpiHookFunction(register __i0 u_int16 hook, register __a0 void *newFunc);
        .sect code,SetJmpiHookFunction
        .export _SetJmpiHookFunction
_SetJmpiHookFunction:
        ldx (i6)+1,null
        stx b0,(i6)+1        ; sty b1,(i6)
        stx a1,(i6)
        ldi (i0),b
        ldc -6,a1
        ashl b,a1,a        ; mv a0,b0
        ldc 0xa8,b1 //JMPI //ldc 0xa0,b1 //J
        ldc 6,a1
        ashl b,a1,b
        sti b,(i0)
        ldx (i6)-1,a1
        jr
        ldx (i6)-1,b0        ; ldy (i6),b1
        
	.sect code,Disable
	.export _Disable,_Enable,_Halt
_Disable:
	j _Enable+1
	ldc INT_GLOB_DIS,i7
_Enable:
	ldc INT_GLOB_ENA,i7
	STP i7,(i7)
	nop
	nop
	nop
	jr
	nop
#endif

	.end
