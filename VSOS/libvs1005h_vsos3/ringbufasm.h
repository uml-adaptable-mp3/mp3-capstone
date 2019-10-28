#ifndef RING_BUF_ASM_H
#define RING_BUF_ASM_H

#include <vstypes.h>

#ifdef ASM

/* stp = -128 .. 127, bufsz 64,128,...,4096 */
#macro MAKEMOD64 stp,bufsz
        (0x4000|(((bufsz)/64-1)&31)|(((stp)&255)<<6))
#endm


#macro MAKEMODLIN stp
	  (stp)
#endm

/* stp = -64 .. 63, bufsz 1..64 */
#macro MAKEMOD stp,bufsz
	(0x2000|(((stp)&127)<<6)|(((bufsz)-1)&63))
#endm

/* stp = 1, bufsz 1..8192 */
#macro MAKEMODF bufsz
	(0x8000|(((bufsz)-1)&8191))
#endm

/* stp = -1, bufsz 1..8192 */
#macro MAKEMODB bufsz
	(0xA000|(((bufsz)-1)&8191))
#endm

#endif /* ASM */



#endif /* !RING_BUF_H */
