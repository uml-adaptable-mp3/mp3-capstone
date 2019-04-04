/** \file rtosdefs.h Low-level RTOS settings.
 */
#ifndef RTOS_DEFS_H
#define RTOS_DEFS_H

#define USE_QUANTUM 30

#define INT_BASE 0xfc00
#define INT_ENABLEL  (INT_BASE+0)
#define INT_ENABLEH  (INT_BASE+2)
#define INT_ORIGIN   (INT_BASE+4)
#define INT_VECTOR   (INT_BASE+6)
#define INT_ENCOUNT  (INT_BASE+7)
#define INT_GLOB_DIS (INT_BASE+8)
#define INT_GLOB_EN  (INT_BASE+9)

#define PERIP_MEM	Y

#define STACK_START 0x0040

#ifndef TASK_STACK_SIZE
/* stack space for main() and tasks created with CreateTask() */
#define TASK_STACK_SIZE	0x380 //0x400//0x180
#endif /* TASK_STACK_SIZE */

#ifdef ASM
#macro INT_EXTRA_NOPS /* 3-instruction delay by default */
	nop /* 4 cycles */
	nop /* 5 cycles */
	nop /* 6 cycles */
#endm
#endif/*ASM*/

#endif/*RTOS_DEFS_H*/
