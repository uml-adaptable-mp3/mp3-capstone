/// VSOS3 KERNEL FOR VS1005G DEVELOPER BOARD

#include "vo_stdio.h" // Standard Output with printf to VSOS console
#include "vstypes.h"  // Common definitions for the VS_DSP signal processor
#include <string.h>   // Part of the C Standard Library
#include <exec.h>
#include <timers.h>
#include "vs1005g.h"   // I/O definitions for the VS1005G
#include "vsos.h"     // Headers for using the VSOS Kernel
#include "vsos_vs1005g.h" // Functions for starting the OS on a VS1005G Developer Board
#include "vsostasks.h"
#include "power.h"    // Functions for controlling power on the PCB
#include "lcd.h"
#include "audio.h"
#include "apploader.h"
#include <sysmemory.h>
#include "touch.h"
#include <clockspeed.h>
#include <swap.h>
#include "devAudio.h"
#include <audiofs.h>
#include "timeCount.h"
#include <uartSpeed.h>
#include "stdbuttons.h"
#include "hwLocks.h"
#include "xPerip.h"
#include "voaudio.h"
#include "vo_gpio.h"
#include "lowlevels.h"
#include <ctype.h>
#include <strings.h>
#include "msc.h"
#include "clockspeedpatch.h"
#include "extSymbols.h"
#include "transient.h"
#include <cyclic.h>
#include <stdarg.h>
#include "sysuimsg.h"

extern VO_FILE *appFile;

ioresult DefaultSysError(const char *msg, ...) {
	va_list args;
	va_start(args, msg);
	vo_fprintf(vo_stderr, "E'");
	vfprintf(vo_stderr, msg, args);
	vo_fprintf(vo_stderr,"'\n");
	va_end(args);
	return (ioresult)msg;
}
ioresult DefaultSysReport(const char *msg, ...) {
	va_list args;
	va_start(args, msg);
	vfprintf(vo_stderr, msg, args);
	vo_fprintf(vo_stderr,"\n");
	va_end(args);
	return 0;
}

u_int16 DevNullWrite (register __i0 VO_FILE *self, void *buf, u_int16 sourceIndex, u_int16 bytes) {
	fprintf(vo_stderr,"DevNulWr");
	if (bytes>1) {
		fprintf(vo_stderr,"(%d)",bytes);
	}
	fprintf(vo_stderr,"  %c  ", *(char*)buf>>8);
	return 0;
}

void MyDelayHook(void) {
	Delay(1);
}

ioresult SetNextApp(const char *appFileName, const char *parameters){
	if (appFile) {
		fclose(appFile);
	}
	appFile = fopen(appFileName,"rb");
	if (!appFile) return S_ERROR;
	appParameters[0] = 0;
	if (parameters) {
		strcpy(appParameters, parameters);
	}
	return S_OK;
}



// Malloc memory available for libraries, loadable device drivers and applications.
// The "end" address is not included in the allowed memory region
// THESE ADDRESSES MUST NOT BE INCLUDED IN MEM_DESC_KERNEL03.MEM
// After linking the kernel you can check the mem_desc.available file
// to see which memory regions from the mem_desc file were left unused.
#define APP_X_START 0x1C00
#define APP_X_END   0x7FD0
#define APP_Y_START 0x0C00
#define APP_Y_END   0x6FE0
#define APP_I_START 0x3300
#define APP_I_END   0x7FC0

u_int16 appIXYStart[3] = {APP_I_START, APP_X_START, APP_Y_START};

struct SysTask sysTasks[SYS_TASKS] = {
  {{0}, "I/O"   , 256+64+64+128,  5}, // Main Task
  {{0}, "Int"   ,  64,  0},	// Not a real task, but requires stack space
  {{0}, "Net"   , 128-126,  1}, // Placeholder for ethernet handler
  {{0}, "UI"    , 128-126,  1}, // Placeholder for advanced UI handler
  {{0}, "DECOD", 256+124, 10}, // Audio Decoder Task
};

#define PROTECT_XMEM_START

#ifdef PROTECT_XMEM_START

#pragma interrupt y 0x2f
void Timer1Interrupt(void) {
	if (USEX(0)) {
		Disable();
		fprintf(vo_stderr,"X:0 not zero\n");
		while(1);
	}
}

#endif

// Low level system init
auto void Init0(void) {

	Disable(); //Disable Interrupts

	// Initialize the RTOS
	SetupMainStackSize(sysTasks[TASK_IO].stackSize);
	SetupInterruptStackSize(sysTasks[TASK_INTERRUPT].stackSize);
	AddIntServer(INTV_TIMER2, TimeCountAndScheduleHandler);
	PERIP(INT_ENABLE1_HP) &= ~(INTF1_STX | INTF1_SRC);
	
	// Set Malloc Areas
	__InitMemAllocI((void*)APP_I_START, APP_I_END-APP_I_START);
	InitMemAlloc((void*)APP_X_START, APP_X_END-APP_X_START, (void __y*)APP_Y_START, APP_Y_END-APP_Y_START);

	// Set xperipheral interrupt handler
	WriteIMem((void *)(0x20+INTV_XPERIP), 0x2a00000e+((u_int32)((u_int16)XPeripIntAsm) << 6));
	
	memset(0,0,16);	// Clear start of memory to trap zero pointer object references
	*((u_int16*)(4)) = DevNullWrite;	
	
	// Write stack tracing pattern
	{
	  int n,i6;
	  i6 = GetI6()+8;
	  n = STACK_START+STACK_SIZE-i6;
	  if (n > 0) {
	    memset((u_int16 *)i6, 0xbabe, n);
	    memsetY((u_int16 __y *)i6, 0xebab, n);
	  }
	}
	
	InitHwLocks();

	console.Ioctl = CommonOkResultFunction; //For VSOS2 console frame compatibility

	// Setup the time count and schedule handler timer
	PERIP(TIMER_T2H) = (6144-1) >> 16;
	PERIP(TIMER_T2L) = (6144-1);
	PERIP(TIMER_ENA) |= TIMER_ENA_T2;
	PERIP(TIMER_ENA) &= ~(TIMER_ENA_T0);
	useQuantum = 5;

	SetHookFunction((u_int16)Sleep, MyDelayHook);
	SetHookFunction((u_int16)IdleHook, MyDelayHook);
	{
		u_int32 cmd = 0x2a000000 + (((u_int32)DelayMicroSecPatch)<<6);
		WriteIMem(DelayMicroSec, cmd);
	}

	PERIP(INT_ENABLE1_LP) |= INTF1_TIMER2;

	#ifdef PROTECT_XMEM_START
	PERIP(TIMER_T1H) = (6144-1) >> 16;
	PERIP(TIMER_T1L) = (6144-1);
	PERIP(TIMER_ENA) |= TIMER_ENA_T1;
	PERIP(INT_ENABLE0_LP) |= INTF_TIMER1;
	WriteIMem((void *)(0x20+INTV_TIMER1), 0x2a00000e+((u_int32)((u_int16)Timer1Interrupt) << 6));
	#endif

	Enable();	 	
}



int StartTask(int taskId, void(*func)(void)) {
  /// \todo NET/UI tasks cannot be started with this function
  u_int16 i=0x10, j=0;
  while (j < taskId && j < SYS_TASKS) {
    i += sysTasks[j].stackSize;
    j++;
  }
  __curStack = (void *)i;
  return CreateTask(&(sysTasks[j].task), sysTasks[j].name, func,
		    sysTasks[j].priority, sysTasks[j].stackSize);
}

int PrintTasks(void) {
	u_int16 i=0x10, j=0;
	fprintf(vo_stderr, "\nVSOS running with %d tasks:\n", SYS_TASKS);
	for(j=0; j < SYS_TASKS; j++) {
		u_int16 k = sysTasks[j].stackSize;
		u_int16 m,n = 0;
		for (m=i+k-1; (m>i)&&(USEX(m)==0xbabe); m--){
			n++;
		}		
		fprintf(vo_stderr, "Task %5s Stack:%04x-%04x (%3dw), free:%d\n", sysTasks[j].name, i, i+k-1, k, n);
		i += sysTasks[j].stackSize;
	}
	fprintf(vo_stderr, "\n");
}
	

void PrintInt(void) {
	static const char *intName[] = {
		"DAC", "USB", "XPERIP", "SP0", "SP1", "MAC1", "MAC0", "GP0",
		"GP1", "GP2", "MAC2", "I2S", "TX",  "RX",  "TI0", "TI1", 
		"TI2", "FM",  "SRC", "DAO", "RTC", "RDS", "SPDIFR", "SPDIFT", 
		"POS", "REG", "PWM", "SAR", "ERR", "ERR", "ERR", "ERR"
	};
	u_int32 h, l;
	u_int16 i;

	fprintf(vo_stderr, "Interrupts: ");
	h = ((u_int32)PERIP(INT_ENABLE1_HP) << 16) | PERIP(INT_ENABLE0_HP);
	l = ((u_int32)PERIP(INT_ENABLE1_LP) << 16) | PERIP(INT_ENABLE0_LP);
	for (i=0; i<32; i++) {
		int pri = ((u_int16)l&1) | (((u_int16)h&1)<<1);
		l >>= 1;
		h >>= 1;
		if (pri) fprintf(vo_stderr, " INT%d_%s:%d->%u", i, intName[i], pri, (u_int16)(ReadFromProgramRam(0x20+i)>>6L));
	}
	fprintf(vo_stderr, "\n");
}


int LoadDrivers(const char *filename, s_int16 initialConfiguration) {
	u_int16 n = 0, i;
	char *paramStr;
	FILE *f = fopen(filename,"r");
	char s[50];
	static u_int16 neededConfiguration = 0;
	u_int16 currentConfiguration = 0;

	if (initialConfiguration != -1) 
	{
		neededConfiguration = initialConfiguration;
	} 
	else 
	{
		FILE *fp = fopen("S:mode.txt", "rb");
		if (fp)
		{
			neededConfiguration = fgetc(fp) - '0';
			fclose(fp);
		}
		else
		{
			printf("Error : mode file does not exist! \n");
		}
	}
    
    if (neededConfiguration != 8 && neededConfiguration != 9)
    {
        GpioSetPin(0x08, 0);
    }

	if (f) {
		fprintf(vo_stderr,"Load drivers, config %d...\n",neededConfiguration);
		while (fgets(s,sizeof(s),f)) {
			char *p = (s-1)+strlen(s);
			if (*p != '\n' && *p != '\r') {
				int c;
				while ((c = fgetc(f)) != EOF && c != '\n' && c != '\r') {
					// Discard any characters that are left in an overlong line
					// This allows for comment lines that are longer than 14 characters
				}
			}
			while (s[0] && isspace(*p)) {
				*p-- = '\0';
			}
			if (s[0]=='[') {
				currentConfiguration = s[1]-'0';
			} else {
				paramStr = strchr(s,' ');
				if (paramStr) {
					*paramStr=0;
					paramStr++;
				} else {
					paramStr = "";
				}
				if ((neededConfiguration == currentConfiguration)
				&& s[0] && s[0] != '#') {
					fprintf(vo_stderr,"Driver: %s... ",s);
					if (LoadLibraryP(s,paramStr)) {
						n++;
						PERIP(ANA_CF1) |= ANA_CF1_BTNDIS; //Disable power button reset
						RunLibraryFunction(s,0,(int)paramStr);
					} else {
						fprintf(vo_stderr," not loaded");
					}
					fprintf(vo_stderr,"\n");
				}
			}
		}	
		fclose(f);
	}
	return n;
}
	
extern struct TASK mainTask;
extern u_int16 __intStack;

void main(void) {
	int result;
	u_int16 i;
	PERIP(SYSTEMPD) &= ~(SYSTEMPD_RAMDELAY_MASK|SYSTEMPD_ROMDELAY_MASK); //All memory speed settings to 0
	//	InitExtSymbols();
	TransientAddKernelSymbols();

	vo_stdout = vo_stdin = vo_stderr = &consoleFile;
	
	//for (i=0x20; i<0x40; i++) {
	//	printf("0x%08lx, ",ReadFromProgramRam(i));
	//}


	
	PERIP(INT_ENABLE0_LP) = INTF_UART_RX;
	PERIP(INT_ENABLE0_HP) = 0;
	PERIP(INT_ENABLE1_LP) = 0;
	PERIP(INT_ENABLE1_HP) = 0;
	fprintf(vo_stdout,"\nHello.\n"); //goes to UART because console.c uses UART	
	ApplyGFixes();
	appParameters[0]=0;
	loadedLibs=0;

	thisTask->tc_Node.name = "MainTask";



	Disable();
	PERIP(ANA_CF1) |= ANA_CF1_BTNDIS; //Disable power button reset
	Init0();
	CreateCyclic(256, 10, TICKS_PER_SEC/10);
	InitBoard(); //initialize power and chip select decoder on the PCB
	//kernelDebugLevel = GpioReadPin(0x03)?99:0;
	kernelDebugLevel = 0;

	InitClockSpeed(12288, 11520);
	SetClockSpeedLimit(93000000);
	SetClockSpeed(12288000);

	Enable();
	Enable();
	Enable();

	StartOS3(); //Start the VSOS operating system and S: disk
	
	//SetClockSpeed(61440000); //5 x 12.288, saves power
	SetClockSpeed(SET_CLOCK_USB); // 60 MHz from VCO, usb compatible, uses more power
	//SetClockSpeed(120000000); //120 MHz, please (or highest you've got)

	SetRate(48000);
	
	stdaudioout = &audioout;
	ioctl(stdaudioout,IOCTL_RESTART,0);

	/*
	#define CF_CTRL 0xFC25
	#define CF_ENABLE 0xFC26
	#define CF_KEY 0x5A7D
	
	printf("CF_CTRL=%04x\n",PERIP(CF_CTRL));
	PERIP(CF_ENABLE) = CF_KEY;
	PERIP(CF_CTRL) &= ~(1<<7);
	printf("CF_CTRL=%04x\n",PERIP(CF_CTRL));
	*/

	/*
	while(1) {
		for (i6=0; i6<16; i6++) {
			//printf("GPIO 0x0%x: %d\n",i6,GpioReadPin(i6));
			printf("%d",GpioReadPin(i6));
		}
		for (i6=0; i6<16; i6++) {
			//printf("GPIO 0x0%x: %d\n",i6,GpioReadPin(i6));
			printf(" %x:%d",i6,GpioReadPin(i6));
		}
		printf("\n");
		Delay(1000);
	}
	*/
	/*
	{
		u_int16 i;
		for (i=0x20; i<0x40; i++) {
			printf("0x%x: 0x%lx\n",i,ReadIMem((void*)i));
		}
	}
	*/

	AddCyclic(&systemUiMessageCyclicNode, TICKS_PER_SEC/10, 0);
		
	// To acticate USB Mass Storage for the system disk, press [S1] on the PCB while booting
	if (GpioReadPin(0x00)) 
	{
		// Special: To show another disk such as the Nand Flash on the PC, press [S1] and [S2] together 
		// while booting and write section [5] in CONFIG.TXT which replaces device S with the other disk.
		if (GpioReadPin(0x01)) 
		{
			LoadDrivers("S:CONFIG.TXT",9);
		}
		VoMassStorage(VODEV('U') ? 'U' : 'S'); //Try to connect to PC USB as mass storage for drive U: or S:
		// This function will not return.
	}
	
	// Holding down this button in conjunction with another button will allow the set mode to be bypassed
	// and chosen manually without having to rewrite the S:mode.txt file
	if (GpioReadPin(0x1E)) // Play-Pause button
	{
		// Choosing to load mode 0
		// Mode 1 is Basic MP3 mode
		if (GpioReadPin(0x1D)) // Bottom left screen button
		{
			i = LoadDrivers("S:CONFIG.TXT",0);
		}
		// Choosing to load mode 8
		// Mode 8 is going to be loading the internal SPI flash for now. This doesn't require any drivers loaded
		else if (GpioReadPin(0x23)) // Stop button
		{
			printf("Setting internal SPI Flash as mass storage device on PC \n");
			VoMassStorage('S'); //Try to connect to PC USB as mass storage for drive S:
		}
		
		// Choosing to load mode 9
		// Mode 9 is going to be loading the flash drive as a mass storage device
		else if (GpioReadPin(0x1B)) // Top left screen button
		{
			LoadDrivers("S:CONFIG.TXT",9);
			printf("Setting flash drive as mass storage device on PC \n");
			VoMassStorage(VODEV('F') ? 'F' : 'S'); //Try to connect to PC USB as mass storage for drive F: or S:
		}
        // We held down the manual button but didn't choose a mode with it? Read the mode from the file instead
        else
        {
            i = LoadDrivers("S:CONFIG.TXT",-1);
        }
	}
	// We're not manually controlling the mode, let the load driver scan the S:mode.txt file for the proper mode to use
	else
	{
		i = LoadDrivers("S:CONFIG.TXT",-1);
	}

	// Optional: Activate 1-bit digital sound output (pins GPIO2.1 and GPIO2.2)
 	// GpioSetAsPeripheral(0x21); //1-bit Digital Out
	// GpioSetAsPeripheral(0x22); //1-bit Digital Out

	fprintf(vo_stderr, "\n%d driver(s) loaded.\n",i);

	

	PrintTasks();
	PrintInt();

	fprintf(vo_stderr,"\nLoad S:INIT.AP3...");
	appFile = fopen("S:INIT.AP3","rb");
	PERIP(TIMER_ENA) &= ~(TIMER_ENA_T0);
		
	if (!appFile) {
		printf("\nS:INIT.AP3 not found.\n");
		printf("Nothing to do.");
		PERIP(INT_ENABLE1_HP) = 0;
		PERIP(INT_ENABLE0_HP) = 0;
		PERIP(INT_ENABLE1_LP) = 0;
		PERIP(INT_ENABLE0_LP) = INTF_UART_RX;	
		//PrintInt();
		Enable();
		Enable();
		VoMassStorage('S');
		while(1) {
			if (PERIP(UART_DATA) == 0x03) { //Ctrl-C
				PERIP(WDOG_CF) = 1;
				PERIP(WDOG_KEY) = 0x4ea9;
			}
			// Do nothing
		}
	}


	while (__F_OPEN(appFile) && appFile->pos == 0) {
		console.Ioctl(&console, IOCTL_START_FRAME, appFile->Identify(appFile,NULL,0));
		result = RunAppFile3(appParameters);

		__asm {
			// Some pseudo-code to get flush registers and get 
			// around compiler bug 2016-11-29
			and a,null,a; ldx (i6),b0; ldy (i6),b1
		}

		if (result != S_OK) {
			printf("\nApp exit, result: %d.\n",result);
			DelayL(10000000);
		}


		#if 0
		// Close file handles possible opened by application
		{
			u_int16 i;
			for (i=0; i<__FOPEN_MAX_FILES; i++) {
				VO_FILE *f = &vo_files[i];
				if ((f != appFile) && (f != stdaudioout)) {
					if (__F_OPEN(f) && f->op->Close) {
						f->op->Close(f);
					}
					f->flags = 0;
				}
			}
		}
		#endif

	}

	Delay(500);
	printf("Restart.\n");
	//PrintInt();

	Disable();
	PERIP(INT_ENABLE1_HP) = 0;	
	PERIP(INT_ENABLE0_HP) = 0;	
	PERIP(INT_ENABLE1_LP) = 0;	
	PERIP(INT_ENABLE0_LP) = INTF_UART_RX;	

}
	
#if 0
/* Example for FatFindFirst, FatFindNext
#define NAMELENGTH 255
void ListFiles(char *path) {
	char *filename = malloc(NAMELENGTH); //Buffer for file info
	if (filename) { //Yes, got X memory for the buffer
	
		FILE *f = fopen(path,"s"); //Get search handle for the disk
		if (f) {
			fprintf(vo_stderr,"List of files in %s\n",path);
			path += 2; //Remove drive letter and colon from path
			if (FatFindFirst(f,path,filename,NAMELENGTH) == S_OK) {
				do {
					fprintf(vo_stderr, "Name: %-20s ",filename);
					fprintf(vo_stderr, "Attr: %02x ",f->ungetc_buffer);
					fprintf(vo_stderr, "Short: %-13s ",f->extraInfo);
					fprintf(vo_stderr, "Extension: '%s' ",&f->extraInfo[13]);
					fprintf(vo_stderr, "\n");
				} while (S_OK == FatFindNext(f,filename,255));
			}
			fclose(f);
		}
		free(filename); //release the malloc-allocated buffer
	}
}
*/
#endif
