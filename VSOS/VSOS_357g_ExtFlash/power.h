/// Headers for controlling power on the PCB

#ifndef POWER_H
#define POWER_H

#define PCL_SDCARD (1 << 0)
#define PCL_USB_A (1 << 1)
#define PCL_USB_AB (1 << 2)
#define PCL_USB_5V (1 << 3)
#define PCL_RFLINK_POWERDOWN (1 << 4)
#define PCL_RFLINK_XRESET (1 << 5)
#define PCL_AUX_PIN_6 (1 << 6)
#define PCL_RFLINK_XTAL_POWERDOWN (1 << 7)

extern u_int16 powerControlLatchState;
void DelayL (volatile u_int32 i);
void WritePCL(u_int16 value);		
void SetPower(register u_int16 mask, register u_int16 onoff);
#define SetPowerOn(a){SetPower((a),1);}
#define SetPowerOff(a){SetPower((a),0);}
	
void InitBoard();
void ResetSdCard();
void SetPWMLevel(int Level);

#endif
