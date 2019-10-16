#ifndef BATTERY_H
#define BATTERY_H

#define CHARGE_DET (0x17)
#define DISP_LED   (0x16)

#define SAR_AUX2 (0x0 << 8)
#define SAR_AUX3 (0x2 << 8) 
#define SAR_RCAP (0x8 << 8)

void cyclicVoltage(void);
s_int16 GetSarValue(register u_int16 channelShifted);
void monitorVoltage();
void cyclicVoltage(void);
void powerSavingDelay();
void idleCheck();

#endif