#include <vo_stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <apploader.h>
#include <vstypes.h>
#include <vs1005h.h>
#include <clockspeed.h>
#include <audio.h>
#include <timers.h>
#include <dsplib.h>
#include <crc32.h>
#include <kernel.h>
#include <cyclic.h>
#include <extSymbols.h>
#include <taskandstack.h>

#include "power.h"

/* unique 16-bit library identifier */
#define LIB_ID 0x2cA0
#define MIN(a,b) (((a)<(b))?(a):(b))

void monitorVoltage(void);
s_int16 GetSarValue(register u_int16 channelShifted);


float percent = 0;

void CyclicBattery(register struct CyclicNode *cyclicNode)
{
    monitorVoltage();
    percent;
}
struct CyclicNode myCyclicBattery = {{0}, CyclicBattery};

s_int16 GetSarValue(register u_int16 channelShifted) 
{
	// Configure SAR and start the conversion. 
	PERIP(SAR_CF) = SAR_CF_ENA | (10) | channelShifted;
	// Wait until the conversion is complete.
	while (PERIP(SAR_CF) & SAR_CF_ENA);
	// Return the conversion result (0...4095)
	return PERIP(SAR_DAT);
}

void monitorVoltage(void)
{
    u_int32 sar_aux, sar_rcap;
    float volts;

    // Read the AUX channel level using the SAR.
    sar_aux = GetSarValue(SAR_AUX2);
    // Read also the reference voltage using SAR. Since the reference voltage is
    // known, this protects against inaccuracies caused by fluctuations in the
    // analog supply voltage.
    sar_rcap = GetSarValue(SAR_RCAP);

    // Remove the commenting to print the raw SAR values.
    // printf("aux=%4d,  rcap=%4d  \n", sar_aux, sar_rcap);

    // Convert SAR_AUX conversion result to millivolts using the resistor
    // divide factor and a known 1.662V reference voltage (RCAP).
    // Resistor divide factor (1M vs 2M) * bandgap voltage (1.662 millivolts)
    sar_aux = ((s_int32)(sar_aux) * 1662) / sar_rcap;
    // Convert integer millivolts to float volts.
    volts = sar_aux * 0.001 * 2.2;
    percent = MIN((volts - 3) * 100 / 1.2, 100);

    printf("BATT: %.3f\n", percent);
    printf("Voltage: %2.2fV\n", volts);
}

/* This driver runs a cyclic process and adds a new symbol _batteryLevel
   to the system. This can be accessed through C with the name
   batteryLevel. 
   This will run every 10 seconds */
ioresult init(char *parameters)
{
    /* Add a new symbol */
    AddSymbol("_batteryLevel", (void *)LIB_ID, (int)(&percent));
    /* Activate the cyclic function. */
    AddCyclic(&myCyclicBattery, TICKS_PER_SEC * 10, TICKS_PER_SEC * 10);
}

void fini(void)
{
    /* Stop the cyclic function. */
    DropCyclic(&myCyclicBattery);
    /* Remove all symbols associated with LIB_ID. */
    SymbolDeleteLib((void *)LIB_ID);
}