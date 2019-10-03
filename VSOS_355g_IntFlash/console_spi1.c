/// \file console.h This file demonstrates the simplicity of defining a custom console under VSOS03.
#include "vsos.h"
#include "consoleops.h"
#include <vs1005g.h>
#include <stdio.h>
#include <vo_gpio.h>
#include <timers.h>

#if 1 // SPI1 MOSI SEND

void SetSPI1(u_int32 ck) {
	PERIP(SPI1_CF) = (10 << SPI_CF_DLEN_B) | SPI_CF_MASTER | SPI_CF_FSIDLE;
	PERIP(SPI1_CLKCF) = ((ck-1) << 2);
	//GpioSetAsPeripheral(0x17); //MOSI1
	GpioSetAsPeripheral(0x14); //XCS1
}

void SPI1Send(unsigned char ch) {
	u_int16 i;
	unsigned char ch2 = 0xFC01;
	
	while(PERIP(SPI1_STATUS) & SPI_ST_TXRUNNING) {
		//Wait
	}	
	for (i=0; i<8; i++) if (ch & (0x80>>i)) ch2 |= (2 << i);
	PERIP(SPI1_FSYNC) = ch2;
	PERIP(SPI1_DATA) = ch2;
}


u_int16 ConsoleWrite(register __i0 VO_FILE *self, void *buf, u_int16 sourceIndex, u_int16 bytes) {
	static u_int16 doInit = 1;
	if (doInit) {
		
		SetSPI1(212); //212=115200 at 4x clock, 28800 at 1x clock
		doInit = 0;
		SPI1Send(' ');
	}
	//putch(((char*)buf)[0]>>8); //Send character to raw UART port
	SPI1Send(((char*)buf)[0]>>8); //Send character to Timer1 software UART
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

