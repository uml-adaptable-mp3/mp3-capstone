#include <vs1005h.h>
#include "spireceivedreq.h"

#if 1
//#define SPIFIFOY_SIZE_WORDS (4*512/2)
//#define SPIFIFOY_DREQ_THRESHOLD_BYTES 512
//#define GPIO_DREQ_BIT 0 //GPIO2
//#define GPIO_DREQ_PORT 2 //The GPIO port to be used for DREQ

	.sect data_y,fifo_y
	.align SPIFIFOY_SIZE_WORDS
	.export _SpiFifoY
_SpiFifoY:
	.bss SPIFIFOY_SIZE_WORDS

	.sect data_y,fifo_vars
	.export _SpiFIFOYVars
_SpiFIFOYVars:
	.word _SpiFifoY	//wr
	.word _SpiFifoY	//rd
	.word MAKEMODF(SPIFIFOY_SIZE_WORDS)
	.word INT_GLOB_ENA

	.sect code,spififoyint
	.export _SpiFifoYInt
	jmpi _SpiFifoYInt,(i6)+1
#ifdef GPIO_DREQ_BIT
//with flow control (DREQ)
_SpiFifoYInt:
	stx i0,(i6)+1
	stx mr0,(i6)+1	; sty i2,(i6)

	ldc SPI1_DATA,i2
	LDP (i2),a0	; stx a0,(i6)+1
	ldc _SpiFIFOYVars,i2
	ldy (i2)+2,i1	; stx i1,(i6)+1		//read wrptr
	ldy (i2)-1,i0	; stx b0,(i6)		//modulo specifier
	ldy (i2)-1,b0	//read rdptr
	STY a0,(i1)*
	mv i1,a0
	sub b0,a0,a0	; sty i1,(i2)+3 	//save wrptr
	ldc SPIFIFOY_SIZE_WORDS,b0
	jnc $1
	nop
	add a0,b0,a0
$1:
	// +1 because FIFO can not be filled completely, because then it would
	// become empty
	ldc (SPIFIFOY_DREQ_THRESHOLD_BYTES/2)+1,b0
	sub a0,b0,a0	; ldx (i6)-1,b0
	ldx (i6)-1,i1
	jgt $0
	ldc (1<<GPIO_DREQ_BIT),a0

#if GPIO_DREQ_PORT .eq. 2
	ldc GPIO2_CLEAR_MASK,i0	//DREQ down -> no room
#else
#if GPIO_DREQ_PORT .eq. 1
	ldc GPIO1_CLEAR_MASK,i0	//DREQ down -> no room
#else
#if GPIO_DREQ_PORT .eq. 0
	ldc GPIO0_CLEAR_MASK,i0	//DREQ down -> no room
#else
	err; No GPIO port defined
#endif
#endif
#endif
	STP a0,(i0)

$0:	ldx (i6)-1,a0	; ldy (i2),i0	//INT_GLOB_ENA -> i0
	ldx (i6)-1,mr0	; ldy (i6),i2
	reti
	STP i0,(i0)	; ldx (i6)-1,i0
	//2+23 cycles = 0.5MHz @ 320kbit/sec, 9.6MHz @ 96kHz 32-bit stereo PCM

#else

_SpiFifoYInt:
	stx i0,(i6)+1	; sty i2,(i6)	//do not save mr0 here
	ldc SPI1_DATA,i2
	LDP (i2),a0	; stx a0,(i6)+1
	ldc _SpiFIFOYVars,i2
	ldy (i2)+2,i1	; stx i1,(i6)
	ldy (i2)-2,i0			//modulo specifier
	STY a0,(i1)*
	sty i1,(i2)+3	; ldx (i6)-1,i1	//save wrptr

	ldx (i6)-1,a0	; ldy (i2),i0	//INT_GLOB_ENA -> i0
	ldy (i6),i2
	reti
	STP i0,(i0)	; ldx (i6)-1,i0
	//2+12 cycles = 10.8MHz @ 192k stereo 32-bit (14.112MHz with multirate)

#endif

// s_int16 SpiFifoYFill(void);

	.sect code,SpiFifoYFill
	.export _SpiFifoYFill
_SpiFifoYFill:
	ldc _SpiFIFOYVars,i7
	ldy (i7)+1,a0	; ldx (i6)+1,null	//wr
	ldy (i7),b0	; stx b0,(i6)	//rd
	sub a0,b0,a0
	ldc SPIFIFOY_SIZE_WORDS,b0
	jnc $0
	nop
	add a0,b0,a0
$0:	jr
	ldx (i6)-1,b0


// void SpiFifoGet(register __i3 u_int16 *buf, register __a0 s_int16 words);

	.sect code,SpiFifoYGet
	.export _SpiFifoYGet
_SpiFifoYGet:
	ldx (i6)+1,null
	stx i0,(i6)+1	; sty i1,(i6)
	stx ls,(i6)+1	; sty le,(i6)
	stx lc,(i6)	; add a0,ones,a0
	ldc _SpiFIFOYVars+1,i0
	ldy (i0)+1,i1
	loop a0,$0-1
	ldy (i0),i0

	LDY (i1)*,a0
	stx a0,(i3)+1
$0:
	ldc _SpiFIFOYVars+1,i0
	sty i1,(i0)-1	; ldx (i6)-1,lc
#ifdef GPIO_DREQ_BIT
	ldy (i0),a0	//wrptr
	mv b0,i7	; mv i1,b0
	sub b0,a0,a0
	ldc SPIFIFOY_SIZE_WORDS,b0
	jnc $2
	nop
	add a0,b0,a0
$2:
	ldc (SPIFIFOY_DREQ_THRESHOLD_BYTES+16)/2,b0	//a little hysteresis
	sub a0,b0,a0	; mv i7,b0	//restore b0
	ldx (i6)-1,ls	; ldy (i6),le
	jle $1
	ldc (1<<GPIO_DREQ_BIT),a0

#if GPIO_DREQ_PORT .eq. 2
	ldc GPIO2_SET_MASK,i0	//DREQ up -> room
#else
#if GPIO_DREQ_PORT .eq. 1
	ldc GPIO1_SET_MASK,i0	//DREQ up -> room
#else
#if GPIO_DREQ_PORT .eq. 0
	ldc GPIO0_SET_MASK,i0	//DREQ up -> room
#else
	err; No GPIO port defined
#endif
#endif
#endif
	STP a0,(i0)
$1:
#else
	ldx (i6)-1,ls	; ldy (i6),le
#endif
	jr
	ldx (i6)-1,i0	; ldy (i6),i1
#endif

	.end
