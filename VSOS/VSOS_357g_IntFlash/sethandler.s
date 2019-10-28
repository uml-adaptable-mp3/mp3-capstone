	.sect code, SetHandler
//void *SetHandler(register __i0 u_int16 hook, register __a0 void *newFunc);
    .export _SetHandler
_SetHandler:
    ldx (i6)+1,null
    stx b0,(i6)+1    ; sty b1,(i6)
    stx a1,(i6)
    ldi (i0),b
    ldc -6,a1
    ashl b,a1,a    ; mv a0,b0
    ldc 0xa8,b1
    ldc 6,a1
    ashl b,a1,b
    sti b,(i0)
    ldx (i6)-1,a1
    jr
    ldx (i6)-1,b0    ; ldy (i6),b1

    .end
