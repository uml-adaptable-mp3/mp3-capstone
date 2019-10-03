/// \file console.h This file demonstrates the simplicity of defining a custom console under VSOS03.
#include "vsos.h"
#include "consoleops.h"
#include <vs1005g.h>
#include <stdio.h>


#if 1 // TIMER 1 SOFTWARE UART AT PIN CS0 (GPIO1.0) SEND
volatile u_int16 t1sBuf = 0;
#pragma interrupt y 0x2f
void Timer1Interrupt(void) {
	if (!t1sBuf) return;
	if (t1sBuf & 1) {
		PERIP(GPIO1_SET_MASK) = 1;
	} else {
		PERIP(GPIO1_CLEAR_MASK) = 1;
	}
	t1sBuf >>= 1;	
}

void SetTimer1(u_int32 ck) {
	PERIP(TIMER_T1H) = (ck-1) >> 16;
	PERIP(TIMER_T1L) = (ck-1);
	PERIP(TIMER_ENA) |= TIMER_ENA_T1;
	PERIP(INT_ENABLE0_LP) |= INTF_TIMER1;	
}

void Timer1Send(unsigned char ch) {
	u_int16 intSave[4];
	u_int16 i;
	
	Disable();	
	for (i=0; i<4; i++) { // Save int_enable state
		intSave[i] = PERIP(INT_ENABLE0_LP+i);
		PERIP(INT_ENABLE0_LP+i) = 0;
	}
	PERIP(INT_ENABLE0_LP) = INTF_TIMER1;	
	Enable();
	t1sBuf = (ch<<1) | (3<<9); // Start bit, Data, 2 stop bits
	while(t1sBuf){
		//Wait for transmission to be finished
	}
	Disable();
	for (i=0; i<4; i++) { // Restore int_enable state
		PERIP(INT_ENABLE0_LP+i) = intSave[i];
	}
	Enable();	
}

u_int16 ConsoleWrite(register __i0 VO_FILE *self, void *buf, u_int16 sourceIndex, u_int16 bytes) {
	static u_int16 doInit = 1;
	if (doInit) {
		SetTimer1(53); //115200bps=53 (12288000/2/115200 = 53.333, 0.63% speed error, within spec)
		doInit = 0;
		Timer1Send(' ');
	}

	Timer1Send(((char*)buf)[0]>>8); //Send character to Timer1 software UART
};


#endif



#if 0
// UART send
u_int16 ConsoleWrite(register __i0 VO_FILE *self, void *buf, u_int16 sourceIndex, u_int16 bytes) {
	u_int16 oldRxInt;
	
	// Save RX interrupt state - needed for compatibility with legacy routines
	Disable();
	oldRxInt = PERIP(INT_ENABLE0_LP) & INTF_UART_RX;
	PERIP(INT_ENABLE0_LP) &= ~INTF_UART_RX;
	Enable();
	
	//since console is a console, only characters are written to it. Hence bytes is always 1.	
	//putchar(((char*)buf)[0]>>8); //Send character to VS3EMU Console 
	putch(((char*)buf)[0]>>8); //Send character to raw UART port

	// Restore RX interrupt state
	Disable();
	PERIP(INT_ENABLE0_LP) |= oldRxInt;
	Enable();
};
#endif


const FILEOPS ConsoleFileOperations = {
	NULL, NULL, ConsoleIoctl, ConsoleWrite, ConsoleWrite
};

const SIMPLE_FILE consoleFile = {__MASK_PRESENT | __MASK_OPEN | __MASK_WRITABLE | __MASK_FILE, NULL, &ConsoleFileOperations};

