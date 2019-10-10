/// \file devboard.c Functionality for VS1005 developer board V 1.0
#include "vo_stdio.h"
#include <vstypes.h>
#include "vs1005g.h"
#include "iochannel.h"
#include "power.h"
#include "vo_gpio.h"



u_int16 powerControlLatchState = 0;

///Lasse style delay function
void DelayL (volatile u_int32 i) {
	while(i--){
#pragma msg 92 off		
		PERIP(0); 
#pragma msg 92 on		
	}
}

void WritePCL(u_int16 value) {		
	u_int16 modeSave = PERIP(GPIO0_MODE);
	u_int16 ddrSave = PERIP(GPIO0_DDR);
	powerControlLatchState = value;
	PERIP(GPIO0_MODE) &= ~(0x00FF);		// Set as gpio
	PERIP(GPIO0_DDR) |= 0x00FF;			// Set as output	
	PERIP(GPIO0_CLEAR_MASK) = 0x00FF; 	// clear old value from gpio port
	PERIP(GPIO0_SET_MASK) = 0x00FF & value; // write value to lower 8 bits
	
	DelayL(10);
	SELECT_IO_CHANNEL(0); 				// PCL channel select 	
	DelayL(10);
	SELECT_IO_CHANNEL_IDLE();  			// deselect iochannel 
	DelayL(10);
	PERIP(GPIO0_CLEAR_MASK) = 0x00FF; 	// pull low;
	PERIP(GPIO0_MODE) = modeSave;
	PERIP(GPIO0_DDR) = ddrSave;
}


void DefaultSetPower(register u_int16 mask, register u_int16 onoff) {
	if (onoff) {
		powerControlLatchState |= mask;
	} else {
		powerControlLatchState &= ~mask;
	}
	WritePCL(powerControlLatchState);
}

#if 0
void AmpBoardSetPower(register u_int16 mask, register u_int16 onoff) {
	if (mask == PCL_SDCARD) {
		GpioSetPin(0x07, onoff);
	}
}
#endif


void ResetSdCard(void) {
	//	printf("#### (sdreset)");
	//Pull all pins low and cut off power
	GpioSetPin(0x26, 0); //miso gpio2.6
	GpioSetPin(0x2a, 0); //mosi gpio2.10
	GpioSetPin(0x25, 0); //sclk gpio2.5
	GpioSetPin(0x29, 0); //xcs gpio2.9
	GpioSetPin(0x27, 0); //dataX gpio2.7
	GpioSetPin(0x28, 0); //dataX gpio2.8
	SetPowerOff(PCL_SDCARD);
	DelayL(100000);
	// Switch power on
	SetPowerOn(PCL_SDCARD);
	DelayL(10000);
}

void SetPWMLevel(int level) {
	PERIP(PWM_FRAMELEN) = 255; // pulse end position        	
	PERIP(PWM_PULSELEN) = level;  // pulse start position 0,1 disable
}

void InitBoard(void) {
	// set address decoder control bits to gpio outputs
	USEY(GPIO1_MODE) &= ~((1 << 0)|(1 << 15)); // gpio1 to GPIO mode = 0
	USEY(GPIO1_DDR) |= ((1 << 0)|(1 << 15));  // gpio1 to output = 1
	USEY(GPIO0_MODE) &= ~(1 << 11);
	USEY(GPIO0_DDR) |= (1 << 11);
	// set initial state of power control register on the PCB
	powerControlLatchState = 0; //All off
	WritePCL(powerControlLatchState);
	SetPWMLevel(128); //Set an initial level to the PWM output	
	ResetSdCard();
}
