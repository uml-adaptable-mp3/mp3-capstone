#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <timers.h>
#include <consolestate.h>
#include <string.h>
#include <clockspeed.h>
#include <vs1005g.h>
#include <vo_gpio.h>
#include <stdlib.h>
#include "intvectors.h"

#define GET_SOF_CNT() (PERIP(USB_UTMIR)&0x3FFF)
u_int16 GetLR1();
u_int16 GetLR0();

u_int16 itab[10];
u_int16 *ip = itab; 


void IntReguC(void) {
	u_int16 lr1 = GetLR1();
	u_int16 i;
	vo_stdout = vo_stderr;
	fprintf(vo_stderr,"\nSOF count: %d ",GET_SOF_CNT());
	fprintf(vo_stderr,"\nStop at %u(0x%04x): ",lr1,lr1);
	RunLibraryFunction("TRACE",ENTRY_1,lr1);
	fprintf(vo_stderr,"\nPrevious PC samples: ");
	for (i=0; i<sizeof(itab); i++) {
		fprintf(vo_stderr,"\n0x%04x=",itab[i]);
		RunLibraryFunction("TRACE",ENTRY_1,itab[i]);		
	}
	fprintf(vo_stderr,"\nSOF count: %d\n",GET_SOF_CNT());
	RunProgram("TASKS","-v");
	RunProgram("LIBLIST2","");
	while(1);
}

/*
void PrintLibInfo3(u_int16 *lib) {
	u_int16 *p = &lib[lib[0]]+6;
	u_int16 nSections = *p++;
	printf("%d sections: ",nSections);
	for (;nSections;nSections--) {
		u_int16 size = (*p >> 2) << 1;		
		u_int16 page = *p++ & 3;
		u_int16 addr = *p++;		
		printf("pg%d,addr%04x,sz%04d  ",page,addr,size);
		if (page==1) {
			printf("%04x ",*(u_int16*)(addr+size-1));
		}
		if (page==2) {
			printf("%04x ",*(__y u_int16*)(addr+size-1));
		}
	}
}
*/

char *bs = " BUFFER OVERFLOW";
void IntTimer1C(void) {
	u_int16 lr1 = GetLR1();
	*ip++ = lr1;
	if (ip > &itab[sizeof(itab)-1]) { //Store array of 
		ip = itab;
	}
	
	{int i;
		for (i=0; i<MAX_LIB; i++) {
			if (loadedLib[i]) {
				u_int16 *lib = loadedLib[i];
				u_int16 *p = &lib[lib[0]]+6;
				u_int16 nSections = *p++;
				for (;nSections;nSections--) {
					u_int16 size = (*p >> 2) << 1;
					u_int16 page = *p++ & 3;
					u_int16 addr = *p++;
					if (page==2 && *(__y u_int16*)(addr+size-1) != 0xbeef) {
						vo_stdout = vo_stderr; Disable();
						fprintf(stderr,"\nY:0x%04x ",addr);
						goto printat;
					}
					if (page==1 && *(u_int16*)(addr+size-1) != 0xbeef) {
						vo_stdout = vo_stderr; Disable();
						fprintf(stderr,"\nX:0x%04x ",addr);
						printat:
						p+=(nSections*2)-1;
						{
							char *s = (char*)p;
							while (*s && *s != '.') fputc(*s++, vo_stderr);
						}
						fprintf(stderr,"%s near 0x%04x ",bs,lr1);
						RunLibraryFunction("TRACE",ENTRY_1,lr1);
						IntReguC();
					}
				}
			}
		}
	}
	

}




ioresult main(char *parameters) {
	static u_int16 interval = 3000;
	if (parameters[0]) interval = atoi(parameters);

	/* Set REGU (power button) interrupt vector. */
	WriteIMem((void *)(0x20+INTV_REGU), ReadIMem((void *)(&IntReguVector)));

	// Clock out any pending POWBTN keypress press
	PERIP(REGU_CF) |= REGU_CF_REGCK;
	PERIP(REGU_CF) &= ~REGU_CF_REGCK;
	PERIP(INT_ORIGIN1) |= INTF1_REGU;
	PERIP(REGU_CF) |= REGU_CF_REGCK;
	PERIP(REGU_CF) &= ~REGU_CF_REGCK;

	/* Activate REGU interrupt at lowest interrupt priority */
	PERIP(INT_ENABLE1_LP) |= INTF1_REGU;
	PERIP(INT_ENABLE1_HP) &= ~INTF1_REGU;

	WriteIMem((void *)(0x20 + INTV_TIMER1), ReadIMem((void *)(&IntTimer1Vector)));
	//PERIP(TIMER_T1H) = (3000-1) >> 16;
	//PERIP(TIMER_T1L) = (3000-1);
	PERIP(TIMER_T1H) = 0;
	PERIP(TIMER_T1L) = (interval);
	PERIP(TIMER_ENA) |= TIMER_ENA_T1;
	PERIP(INT_ENABLE0_HP) |= INTF_TIMER1;

	return S_OK;
}
