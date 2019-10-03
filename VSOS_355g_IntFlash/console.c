/// \file console.h This file demonstrates the simplicity of defining a custom console under VSOS03.
#include "vsos.h"
#include "consoleops.h"
#include <vs1005g.h>
#include <stdio.h>


u_int16 ConsoleWrite(register __i0 VO_FILE *self, void *buf, u_int16 sourceIndex, u_int16 bytes) {
	u_int16 oldRxInt;
	
	// Save RX interrupt state - needed for compatibility with legacy routines
	Disable();
	oldRxInt = PERIP(INT_ENABLE0_LP) & INTF_UART_RX;
	PERIP(INT_ENABLE0_LP) &= ~INTF_UART_RX;
	Enable();
	
	//since console is a console, only characters are written to it. Hence bytes is always 1.	
	//putchar(((char*)buf)[0]>>8); //Send character to VS3EMU Console 
	putch(((char*)buf)[0]); //Send character to raw UART port
	
	// Restore RX interrupt state
	Disable();
	PERIP(INT_ENABLE0_LP) |= oldRxInt;
	Enable();
};


const FILEOPS ConsoleFileOperations = {
	NULL, NULL, ConsoleIoctl, ConsoleWrite, ConsoleWrite
};

const SIMPLE_FILE consoleFile = {__MASK_PRESENT | __MASK_OPEN | __MASK_WRITABLE | __MASK_FILE, NULL, &ConsoleFileOperations};

