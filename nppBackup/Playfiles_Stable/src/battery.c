#include <vo_stdio.h>
#include <apploader.h>  // RunLibraryFunction etc
#include <kernel.h>
#include <cyclic.h>
#include <lcd.h>

#include "battery.h"
#include "ili9341.h"

#define MIN(a,b) (((a)<(b))?(a):(b))

extern u_int16 idleMode;
extern u_int16 anyButtonPressed;
extern struct CyclicNode myCyclicNode;
extern u_int16 voltageChecked;
extern u_int16 auxChoice;

void powerSavingDelay()
{
	if (idleMode == 1)
	{
		GpioSetPin(DISP_LED, 0);
		Delay(750);
	}
	else
	{
		GpioSetPin(DISP_LED, 1);
		Delay(50);
	}
}

s_int16 GetSarValue(register u_int16 channelShifted) 
{
	// Configure SAR and start the conversion. 
	PERIP(SAR_CF) = SAR_CF_ENA | (10) | channelShifted;
	// Wait until the conversion is complete.
	while (PERIP(SAR_CF) & SAR_CF_ENA);
	// Return the conversion result (0...4095)
	return PERIP(SAR_DAT);
}

void monitorVoltage()
{
	u_int32 sar_aux, sar_rcap;
	float volts;
	float percent;
	char output[10];

    // Read the AUX channel level using the SAR.
    if (auxChoice == 1)
    {		
		sar_aux = GetSarValue(SAR_AUX2);
	}
	else
	{
		sar_aux = GetSarValue(SAR_AUX3);
	}
    // Read also the reference voltage using SAR. Since the reference voltage is
    // known, this protects against inaccuracies caused by fluctuations in the
    // analog supply voltage.
    sar_rcap = GetSarValue(SAR_RCAP);
    
    // Remove the commenting to print the raw SAR values.
    printf("aux=%4d,  rcap=%4d  \n",sar_aux,sar_rcap);
    
    // Convert SAR_AUX conversion result to millivolts using the resistor
    // divide factor and a known 1.662V reference voltage (RCAP).
    // Resistor divide factor (1M vs 2M) * bandgap voltage (1.662 millivolts)
    sar_aux = ((s_int32)sar_aux * 1662) / sar_rcap; 
    // Convert integer millivolts to float volts.
    volts = sar_aux * 0.001 * 2.2;
    percent = MIN((volts - 3) * 100 / 1.2, 100);

    LcdTextOutXY(130, 1, "BATT(   ): ");
    
    if (GpioReadPin(CHARGE_DET))
    {
       lcd0.textColor = GREEN;
       LcdTextOutXY(170, 1, "CH");
	 }
	 else
	 {
	  lcd0.textColor = RED;
	  LcdTextOutXY(170, 1, "NC");
	 }
	 

	 if (percent > 60)
	 {
	  lcd0.textColor = GREEN;
	 }
	 else if (percent > 15)
	 {
	  lcd0.textColor = YELLOW;
	 }
	 else
	 {
	  lcd0.textColor = RED;
	 }

	 if (percent == 100)
	 {
		 LcdTextOutXY(205, 1, "100%");
 	 }
 	 else if (percent > 15)
 	 {
		sprintf(output, " %.2f%", percent);
		LcdTextOutXY(205, 1, output);
	 }
	 else
	 {
			LcdTextOutXY(205, 1, "LOW!");
	 }
	 
	 #if 1
	    printf("BATT: %.3f% \n", percent);
	    printf("Voltage: %2.2fV \n", volts);
	 #endif
	 
   lcd0.textColor = WHITE;
	 
	voltageChecked = 1;
}


void idleCheck()
{
	// Wait 10 seconds to check if a button was pressed since last check
	// If button isn't pressed, system is now in idleMode
	// Always clear button press for next check
	// Worst case, someone pressed a button the second after the Delay ends
	// and then not again through the next delay. So, the idle time will
	// range somewhere between 10-20 seconds before it kicks in.
	// ---
	// idleMode is always turned off whenever a button is pressed externally
	// anyButtonPressed is always turned on whatever a button is pressed externally
	// Minor race conditions may occur because there is no mutual exclusion, but the
	// probabliity is incredibly low and self correcting the next check, so it is okay
	while (1)
	{
		Delay(10000);
		if (anyButtonPressed == 0)
		{
			idleMode = 1;
		}
		anyButtonPressed = 0;
	}
}

// Function that is run by the task creator
// This passively reads the voltage every 60 seconds
void cyclicVoltage(void)
{
   while (1)
   {
   		Delay(60000);
   		
		// The voltage can be checked when menus are updated automatically,
		// so if that's true then we don't actually have to check it this cycle
		// This allows for further power reduction in cases that it was updated
		// already
   		if (voltageChecked)
   		{
   			voltageChecked = 0;
		}
		else
		{
	       monitorVoltage();
	       voltageChecked = 1;
		}
	}
}