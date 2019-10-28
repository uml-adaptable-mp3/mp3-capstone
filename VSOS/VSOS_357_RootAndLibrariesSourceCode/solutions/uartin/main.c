/// \file main.c Simple UART device driver. No speed setting yet.
/// \author Panu-Kristian Poiksalo, VLSI Solution Oy

/* For free support for VSIDE, please visit www.vsdsp-forum.com */

// If you add the libary name to S:CONFIG.TXT, and put the library 
// to S:SYS, it will be loaded and run during boot-up. Use 8.3 file names.

// A short example on how to use the UART file interface:
//	void *libUart=LoadLibrary("devUart"); // (assuming this project is called 'devUart')
//	FILE *uart = (FILE *)RunProgram("devUart",NULL);
//	fprintf(uart,"Hello, world.\n");
//  DropLibrary(libUart);

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <string.h>
#include <timers.h>
#include <vs1005h.h>
#include <imem.h>
#include <ringbuf.h>
#include <kernel.h>
#include <consolestate.h>
#include <uimessages.h>


/* Must be power of 2 */
#define UART_RX_BUFFER_SIZE 256
/* Must be power of 2 */
#define UART_TX_BUFFER_SIZE 256
__mem_y char __align uartRxBuffer[UART_RX_BUFFER_SIZE]; //must be power of 2
__mem_y char *uartRxRd=uartRxBuffer;
__mem_y char *uartRxWr=uartRxBuffer;
__mem_y char __align uartTxBuffer[UART_TX_BUFFER_SIZE]; //must be power of 2
__mem_y char *uartTxRd=uartTxBuffer;
__mem_y char *uartTxWr=uartTxBuffer;

void WatchdogReset(void) {
  //PERIP(UART_DATA) = 'R';
  PERIP(WDOG_CF) = 1;
  PERIP(WDOG_KEY) = 0x4ea9;
  while (1)
    ;
}
	
void IntUartTxC(void) {
  while (!(PERIP(UART_STATUS) & UART_ST_TXFULL) && uartTxWr != uartTxRd) {
    PERIP(UART_DATA) = *uartTxRd;
    uartTxRd = ringmodY(uartTxRd, MAKEMODF(UART_TX_BUFFER_SIZE));
  }
}

void IntUartRxC(void) {
  static u_int16 inProtocol = 0;	
  static u_int32 value;
  static u_int16 address;
  char c = PERIP(UART_DATA);

  if (c==0x03) { //Ctrl-C
    static u_int16 count = 0;
    if ((appFlags & (APP_FLAG_QUIT | APP_FLAG_RAWTTY)) == APP_FLAG_QUIT) {
      count++;
      //PERIP(UART_DATA) = '0'+count;
      if (count==2) { // 3rd Ctrl-C -> force watchdog reset
	PERIP(WDOG_CF) = 1;
	PERIP(WDOG_KEY) = 0x4ea9;
	while (1) {
	  //Wait for watchdog reset				
	}
      }
    } else {
      count=0;
      appFlags |= APP_FLAG_QUIT;
    }
  } else if ((c==0x02) && (!(appFlags & APP_FLAG_RAWTTY))) { //Ctrl-B
    inProtocol = 1;
    value = 0;
    return;
  }
  if (inProtocol) {
    if (c >= '0' && c <= '9') {
      value <<= 4;
      value |= (c-'0');
    } else if (c >= 'a' && c <= 'f') {
      value <<= 4;
      value |= (c-'a'+10);
    } else if (c == 'm') {
      address = value;
      value = 0;
    } else if (c == 's') {
      SystemUiMessage(0, address, value);
      inProtocol = 0;
    }
    return;		
  }

  *uartRxWr = c;
  uartRxWr = ringmodY(uartRxWr, MAKEMODF(UART_RX_BUFFER_SIZE));
}



void UartPutChar(char c) {
  __mem_y char *nextTxWr = ringmodY(uartTxWr, MAKEMODF(UART_TX_BUFFER_SIZE));

  while (nextTxWr == uartTxRd) {
    Delay(1);
  }

  Disable();
  if (!(PERIP(UART_STATUS) & UART_ST_TXFULL) && uartTxWr == uartTxRd) {
    PERIP(UART_DATA) = c;
  } else {
    *uartTxWr = c;
    uartTxWr = nextTxWr;
  }
  Enable();
}

char UartGetChar(void) {
  char ch;
  while (uartRxRd == uartRxWr) {
    Delay(1);
  }
  ch = *uartRxRd;
  uartRxRd = ringmodY(uartRxRd, MAKEMODF(UART_RX_BUFFER_SIZE));
  return ch;
}



IOCTL_RESULT UartIoctl(register __i0 VO_FILE *self, s_int16 request, IOCTL_ARGUMENT arg) {
  if (request == IOCTL_TEST) {
    return (uartRxWr-uartRxRd)&(UART_TX_BUFFER_SIZE-1);
  }
  return S_ERROR; // No IOCTL's defined yet. Setting the speed would be a nice one.
};

u_int16 UartWrite(register __i0 VO_FILE *self, void *buf, u_int16 sourceIndex, u_int16 bytes) {
  UartPutChar(*((u_int16 *)buf)); //Send character to raw UART port
};

u_int16 UartRead(register __i0 VO_FILE *self, void *buf, u_int16 destinationIndex, u_int16 bytes) {
  char ch = UartGetChar();
  MemCopyPackedBigEndian(buf,destinationIndex,&ch,1,1);
  return 1;
};

u_int16 StdInWrite(register __i0 VO_FILE *self, void *buf, u_int16 sourceIndex, u_int16 bytes) {
  *uartRxWr = ((char*)buf)[0]>>8;
  uartRxWr = ringmodY(uartRxWr, MAKEMODF(UART_RX_BUFFER_SIZE));
};


const FILEOPS ConsoleInFileOperations = { NULL, NULL, UartIoctl, UartRead, StdInWrite };
DLLENTRY(uartInFile)
SIMPLE_FILE uartInFile = {__MASK_PRESENT | __MASK_OPEN | __MASK_READABLE | __MASK_WRITABLE | __MASK_FILE, NULL, &ConsoleInFileOperations};

const FILEOPS ConsoleOutFileOperations = { NULL, NULL, UartIoctl, UartRead, UartWrite };
DLLENTRY(uartOutFile)
SIMPLE_FILE uartOutFile = {__MASK_PRESENT | __MASK_OPEN | __MASK_READABLE | __MASK_WRITABLE | __MASK_FILE, NULL, &ConsoleOutFileOperations};

extern u_int16 IntUartRxVector;
extern u_int16 IntUartTxVector;

struct Orig {
  FILE *in, *out;
  u_int16 intEna0Mask;
  u_int32 rxVec, txVec;
} orig;

void init(char *parameters) {
  orig.intEna0Mask = PERIP(INT_ENABLE0_HP) | ~(INTF_UART_TX|INTF_UART_RX);
  orig.rxVec = ReadIMem((0x20+INTV_UART_RX));
  orig.txVec = ReadIMem((0x20+INTV_UART_TX));
  orig.in = vo_stdin;
  orig.out = vo_stdout;
}

// Library finalization code. This is called when DropLibrary is called
void fini(void) {
  PERIP(INT_ENABLE0_HP) &= orig.intEna0Mask;
  WriteIMem((0x20+INTV_UART_RX), orig.rxVec);
  WriteIMem((0x20+INTV_UART_TX), orig.txVec);
  vo_stdout = orig.out;
  vo_stdin = orig.in;
}

// The main function returns a pointer to the UART file.
ioresult main(char *parameters) {
  int in=0, out=0;

  while (*parameters) {
    char c = *parameters | 0x20;
    if (c == 'i') {
      in=1;
    } else if (c == 'o') {
      out=1;
    }
    parameters++;
  }
  if (!(in|out)) {
    in = out = 1;
  }

  if (in) {
    WriteIMem((0x20+INTV_UART_RX), ReadIMem((u_int16)(&IntUartRxVector)));
    PERIP(INT_ENABLE0_HP) |= INTF_UART_RX;
    vo_stdin = &uartInFile;
  }

  if (out) {
    WriteIMem((0x20+INTV_UART_TX), ReadIMem((u_int16)(&IntUartTxVector)));
    PERIP(INT_ENABLE0_HP) |= INTF_UART_TX;
    vo_stdout = &uartOutFile;
  }

  return S_OK;
}
