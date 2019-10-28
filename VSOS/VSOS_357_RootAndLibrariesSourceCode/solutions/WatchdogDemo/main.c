/* For free support for VSIDE, please visit www.vsdsp-forum.com */

// Starting point template for creating VSOS3 libraries and device drivers.
// This will create a <projectname>.DL3 file, which you can copy to 
// your VS1005 Developer Board's system disk's SYS subdirectory.

// If init(), main() or fini() are not needed, remove them from the solution.
// There's no need to have any unneeded functions in the library.

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <timers.h>
#include <audio.h>
#include <consolestate.h>
#include <string.h>
#include <kernel.h>

#define WATCHDOG_RESET_SECONDS 10

ioresult main(char *parameters) {
	/* Reset value: XTALI / 65536 * watchdogTimeOutSeconds */
	u_int16 wdogResetValue = (u_int16)(12288000 * WATCHDOG_RESET_SECONDS >> 16);
	u_int16 i = 0;

	printf("\n\tWatchdog demo\n\n");
	printf("Setting watchdog reset value to %u.\n\n", wdogResetValue);
	PERIP(WDOG_CF) = wdogResetValue;
	PERIP(WDOG_KEY) = WDOG_KEY_VAL;
	printf("Watchdog is now active.\n");
	printf("Push 'd' to simulate software hanging up!\n\n");

	while (1) {
		int c = -1;

		if (ioctl(stdin, IOCTL_TEST, NULL) > 0) {
			c = fgetc(stdin);
		}

		if (c == 'd') {
			printf("\n'd' pushed, simulating system hanging up.\n");
			printf("In %d seconds the watchdog will reset the system.\n", WATCHDOG_RESET_SECONDS);
			while (1); /* Infinite loop */
		}

		printf("%d: System is running nicely, resetting watchdog\n", i++);
		PERIP(WDOG_KEY) = WDOG_KEY_VAL;
		Delay(500);
	}

 finally:
	return S_OK;
}
