#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <timers.h>
#include <consolestate.h>
#include <string.h>
#include <vs1005h.h>
#include <vo_gpio.h>
#include <kernel.h>

u_int16 gpioState=0, gpioPend=0;

#pragma interrupt y 0x27
void Gpio0Interrupt(void) {
	/* Let's make the interrupt as short as possible. */
	gpioPend |= PERIP(GPIO0_INT_PEND); /* Get which bits have changed. */
	PERIP(GPIO0_INT_PEND) = 0xFFFF;		/* Clear the flags. */
	gpioState = PERIP(GPIO0_IDATA);		/* Get the actual GPIO data. */
}

ioresult main(char *parameters) {
	int quit = 0;

	if (!strcmp(parameters, "-h")) {
		printf("Usage: GpioDemo [-h]\n");
		goto finally;
	}

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
	

	while (!(appFlags & APP_FLAG_QUIT)) {
		u_int16 localGpioPend, localGpioState;

		/* Disable interrupts */
		Disable();
		/* Take local copy of the variables */
		localGpioPend = gpioPend;
		localGpioState = gpioState;
		/* Clear main variables. */
		gpioPend = 0;
		/* Re-enable interrupts */
		Enable();

		if (localGpioPend) {
			int i;
			for (i=0; i<4; i++) {
				if ((localGpioPend >> i) & 1) {
					printf("GPIO0_%d = %d\n", i, (localGpioState>>i) & 1);
				}
			}
		} else {
			/* Wait for one millisecond */
			Delay(1);
		}
	} /* End main loop. */

	/* Remove the interrupt */
	PERIP(INT_ENABLE0_LP) &= ~INTF_GPIO0;
	printf("End.\n");

 finally:
	return S_OK;
}
