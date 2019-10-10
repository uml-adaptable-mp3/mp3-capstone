#include <vstypes.h>
#include "vo_gpio.h"
#include "vs1005g.h"
#include <vo_stdio.h>
#include <clockspeed.h>

const u_int16 vo_gpio_perips[] = {GPIO0_DDR, GPIO1_DDR, GPIO2_DDR};

typedef struct __gpio_structure {
	u_int16  ddr;
	u_int16  odata;
	u_int16  idata;
	u_int16  int_fall;
	u_int16  int_rise;
	u_int16  int_pend;
	u_int16  set_mask;
	u_int16  clear_mask;
	u_int16  bit_conf;
	u_int16  bit_eng0;
	u_int16  bit_eng1;
} GpioPeripheral;

auto u_int16 GpioReadInputPin(register u_int16 pin) {
	volatile __y GpioPeripheral *gpioport = (__y void*)vo_gpio_perips[pin>>4];			
	return (gpioport->idata & (1 << (pin & 0xf))) ? 1 : 0;
}


auto u_int16 GpioReadPinDelay(register u_int16 pin, register u_int32 microSeconds, register u_int16 waitAlways) {
	volatile __y GpioPeripheral *gpioport = (__y void*)vo_gpio_perips[pin>>4];		
	u_int16 modeSave;
	u_int16 ddrSave;
	u_int16 result;
	u_int16 mask;
	Disable(); // The Disable() length for this function is way too long for comfort
	modeSave = PERIP(GPIO0_MODE+(pin>>4));
	ddrSave = gpioport->ddr;
	mask = (1 << (pin & 0xf));
	if (waitAlways || (modeSave & mask) || (ddrSave & mask)) {
		gpioport->ddr &= ~mask; // Set as input
		PERIP(GPIO0_MODE+(pin>>4)) &= ~mask; //Set as GPIO
		DelayMicroSec(microSeconds);
	}
	result = (gpioport->idata & mask) ? 1 : 0;
	PERIP(GPIO0_MODE+(pin>>4)) = modeSave; //Restore mode
	gpioport->ddr = ddrSave; // Restore DDR
	Enable();
	return result;	
}

#if 0
auto u_int16 GpioReadPin(register u_int16 pin) {
	return GpioReadPinDelay(pin, 15, 0);
}
#endif

auto void GpioSetPin(register u_int16 pin, register u_int16 state) {
	volatile __y GpioPeripheral *gpioport = (__y void*)vo_gpio_perips[pin>>4];		
	u_int16 mask = (1U << (pin & 0xf));
	Disable();
	gpioport->ddr |= mask; // Set as output
	PERIP(GPIO0_MODE+(pin>>4)) &= ~mask; //Set as GPIO
	Enable();
	if (state) {
		gpioport->set_mask = mask;
	} else {
		gpioport->clear_mask = mask;
	}
}

auto void GpioSetAsInput(register u_int16 pin) {
	volatile __y GpioPeripheral *gpioport = (__y void*)vo_gpio_perips[pin>>4];		
	u_int16 mask = (1 << (pin & 0xf));
	Disable();
	gpioport->ddr &= ~mask; // Set as input
	PERIP(GPIO0_MODE+(pin>>4)) &= ~mask; //Set as GPIO
	Enable();
}

// Below here: not in VS1005F ROM
auto void GpioSetAsPeripheral(register u_int16 pin) {
	volatile __y GpioPeripheral *gpioport = (__y void*)vo_gpio_perips[pin>>4];		
	u_int16 mask = (1 << (pin & 0xf));
	// gpioport->ddr &= ~mask; // Set as input
	Disable();
	PERIP(GPIO0_MODE+(pin>>4)) |= mask; //Set as Peripheral
	Enable();
}
