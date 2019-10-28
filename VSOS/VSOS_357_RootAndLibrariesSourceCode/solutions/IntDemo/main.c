#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <timers.h>
#include <consolestate.h>
#include <string.h>
#include <clockspeed.h>
#include <vs1005h.h>
#include <vo_gpio.h>
#include <imem.h>
#include <kernel.h>
#include "intvectors.h"




u_int16 timer1Counter = 0;

void IntTimer1C(void) {
	timer1Counter++;
}


u_int16 powerButtonPushed = 0;

void IntReguC(void) {
	powerButtonPushed = 1;
}


u_int16 gpio0State=0, gpio0Pend=0;

void IntGpio0C(void) {
	/* Let's make the interrupt as short as possible. */
	gpio0Pend |= PERIP(GPIO0_INT_PEND); /* Get which bits have changed. */
	PERIP(GPIO0_INT_PEND) = 0xFFFF;		/* Clear the flags. */
	gpio0State = PERIP(GPIO0_IDATA);		/* Get the actual GPIO0 data. */
}

ioresult main(char *parameters) {
	int quit = 0;

	if (!strcmp(parameters, "-h")) {
		printf("Usage: IntDemo [-h]\n");
		goto finally;
	}


	/* Set GPIO0 interrupt vector. */
	WriteIMem(0x20+INTV_GPIO0, ReadIMem((u_int16)(&IntGpio0Vector)));

	/* Make GPIO0_0 through GPIO0_3 inputs */
	GpioSetAsInput(0x00);
	GpioSetAsInput(0x01);
	GpioSetAsInput(0x02);
	GpioSetAsInput(0x03);

	/* We are interested in the rising edges of GPIO0_0 through GPIO0_3... */
	PERIP(GPIO0_INT_RISE) = 0x000F;
	/* ... as well as the falling edges. */
	PERIP(GPIO0_INT_FALL) = 0x000F;

	/* Activate GPIO0 interrupt at lowest interrupt priority */
	PERIP(INT_ENABLE0_LP) |= INTF_GPIO0;
	PERIP(INT_ENABLE0_HP) &= ~INTF_GPIO0;




	/* Set REGU (power button) interrupt vector. */
	WriteIMem(0x20+INTV_REGU, ReadIMem((u_int16)(&IntReguVector)));

	/* Activate REGU interrupt at lowest interrupt priority */
	PERIP(INT_ENABLE1_LP) |= INTF1_REGU;
	PERIP(INT_ENABLE1_HP) &= ~INTF1_REGU;




	/* Set TIMER1 interrupt vector. */
	WriteIMem(0x20+INTV_TIMER1, ReadIMem((u_int16)(&IntTimer1Vector)));

	/*
		Set TIMER1 to actuate once every second.
		VSOS has already set TIMER_CF to be 1 -> half of input clock = 6.144 MHz.
		So, we need to set TIMER1 according to this.
	*/
	PERIP(TIMER_T1H) = (u_int16)(6144000>>16);
	PERIP(TIMER_T1L) = (u_int16)6144000;
	PERIP(TIMER_ENA) |= TIMER_ENA_T1;
	PERIP(TIMER_T1CNTH) = PERIP(TIMER_T1H);

	/* Activate TIMER1 interrupt at lowest interrupt priority */
	PERIP(INT_ENABLE0_LP) |= INTF_TIMER1;
	PERIP(INT_ENABLE0_HP) &= ~INTF_TIMER1;

	while (!(appFlags & APP_FLAG_QUIT)) {
		u_int16 localGpio0Pend, localGpio0State;
		static u_int16 oldTimer1Counter = 0;

		/* Disable interrupts */
		Disable();
		/* Take local copy of the variables */
		localGpio0Pend = gpio0Pend;
		localGpio0State = gpio0State;
		/* Clear main variables. */
		gpio0Pend = 0;
		/* Re-enable interrupts */
		Enable();

		if (localGpio0Pend) {
			int i;
			for (i=0; i<4; i++) {
				if ((localGpio0Pend >> i) & 1) {
					printf("GPIO0_%d = %d\n", i, (localGpio0State>>i) & 1);
				}
			}
		} else if (powerButtonPushed) {
			Disable();
			powerButtonPushed = 0;
			Enable();
			printf("Power button pushed\n");
		} else if (timer1Counter != oldTimer1Counter) {
			oldTimer1Counter = timer1Counter;
			printf("Timer counter %u\n", oldTimer1Counter);
		} else {
			/* Allows for the power button interrupt to happen again. */
			PERIP(REGU_CF) |= REGU_CF_REGCK;
			PERIP(REGU_CF) &= ~REGU_CF_REGCK;
			/* Wait for one millisecond */
			Delay(1);
		}
	} /* End main loop. */


	/* Stop TIMER1 */
	PERIP(TIMER_ENA) &= ~TIMER_ENA_T1;

	/* Remove the interrupts */
	PERIP(INT_ENABLE0_LP) &= ~(INTF_GPIO0|INTF_TIMER1);
	PERIP(INT_ENABLE1_LP) &= ~INTF1_REGU;
	printf("End.\n");

 finally:
	return S_OK;
}
