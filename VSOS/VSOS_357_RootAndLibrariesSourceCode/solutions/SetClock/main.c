/*

  SetClock sets the clock frequency of VS1005g.
  
*/
#include <vo_stdio.h>
#include <stdlib.h>
#include <apploader.h>
#include <consolestate.h>
#include <string.h>
#include <stdlib.h>
#include <audio.h>
#include <clockspeed.h>
#include <uartSpeed.h>
#include <timers.h>
#include <kernel.h>
#include <imem.h>
#include "rf_clock.h"
#include "fuse.h"
#include "otp.h"

#define MV_CVDD 0
#define MV_IOVDD 1
#define MV_AVDD 2

#ifndef CF_CTRL
#define CF_CTRL 0xfc25
#define CF_ENABLE 0xfc26
#define CF_KEY 0x5a7d
#define CF_CTRL_PORD (1U<<15)
#endif

auto s_int16 VSOS_SetClockSpeed(register u_int32 clkHz);
auto void FindVCOLock(void);


__mem_y u_int16 *timerCountReg = TIMER_T0L;
s_int16 chipIsG = 0;




s_int16 DacOff(void) {
  Disable();
  PERIP32(DAC_LEFT_LSB) = 0;
  PERIP32(DAC_RIGHT_LSB) = 0;
  DelayMicroSec(1000);
  PERIP32(DAC_SRCL) = 0; /* Shut down DAC modulator -> no background noise. */
  Enable();
}


void FlushUart(void) {
  int consecutiveTxStopped = 0;
  /* Run until UART TX hasn't been on for 64 consecutive loops. */
  while (consecutiveTxStopped < 64) {
    if (PERIP(UART_STATUS) & (UART_ST_TXRUNNING | UART_ST_TXFULL)) {
      consecutiveTxStopped = 0;
    } else {
      consecutiveTxStopped++;
    }
  }
}


#define CLOCKS_PER_LOOP 10

#if 1
u_int32 TestSpeedLoopAsm(register __reg_d u_int32 startTime);
#else
u_int32 TestSpeedLoopAsm(register __reg_d u_int32 startTime) {
  u_int32 loops = 0;
  u_int32 endTime = startTime;

  while (endTime - startTime < TICKS_PER_SEC) {
    Disable();
    /* ReadTimeCount() is variable speed, so we can't use it here. */
    endTime = timeCount;
    Enable();
    loops++;
  }
  return loops;
}
#endif


void TestSpeed(void) {
  u_int32 loops = 0;
  double effectiveMHz, cpuMHz;
  const char *unit = "MHz";
  u_int32 startTime, endTime;
  u_int32 loops = 0;

  printf("CPU speed: ");
  FlushUart();

  startTime = ReadTimeCount();
  /* Wait for next time count switch. */
  while ((endTime = ReadTimeCount()) == startTime)
      ;
  loops = TestSpeedLoopAsm(endTime);
  endTime = ReadTimeCount();
  loops = (u_int32)((double)loops*TICKS_PER_SEC/(endTime-startTime));
  
  effectiveMHz = loops*(1.0e-6*CLOCKS_PER_LOOP);
  cpuMHz = 1.0e-6*clockSpeed.cpuClkHz;
  if (cpuMHz < 1.0) {
    effectiveMHz *= 1000.0;
    cpuMHz *= 1000.0;
    unit = "kHz";
  }
  printf("effective speed %3.2f/%3.2f %s = %2.1f%%, "
	 "overhead %3.2f %s\n",
	 effectiveMHz, cpuMHz, unit, 100.0*effectiveMHz/cpuMHz,
	 cpuMHz-effectiveMHz, unit);
}


auto void CalcClockSpeed(void);

auto u_int16 MyForceClockMultiplier(register u_int16 clockX) {
  while (clockX * clockSpeed.peripClkHz > 2 * clockSpeed.speedLimitHz)
    clockX--;

  if (clockX <= 2) {
    clockSpeed.pllPreDiv = 0;
    clockSpeed.pllMult = 1;
    clockSpeed.masterClkSrc = MASTER_CLK_SRC_XTALI;
    oldClockX = 2;
  } else {
    oldClockX = clockX;
    clockSpeed.masterClkSrc = MASTER_CLK_SRC_PLL;
    if (clockX & 1) {
      clockSpeed.pllPreDiv = 1;
      clockSpeed.pllMult = clockX;
    } else {
      clockSpeed.pllPreDiv = 0;
      clockSpeed.pllMult = clockX >> 1;
    }
  }

  /* Now it's time to set our clock. First turn PLL control off and set new
     PLL frequency. */
  PERIP(CLK_CF) = (PERIP(CLK_CF) & 0xFF00U) |
    (clockSpeed.pllMult-1) | clockSpeed.pllPreDiv*CLK_CF_DIVI;

  /* Now, if the multiplier is over 2, wait a while, then turn it back on. */
  if (clockSpeed.pllMult > 1) {
    Delay(1);
    PERIP(CLK_CF) |= CLK_CF_FORCEPLL;
  }

  CalcClockSpeed();


  return clockX;
}




extern int voltageValues[3];
extern int mV[3];
void SetVoltages(void);

/* ENTRY_1 */
DLLENTRY(MySetClockSpeed)
auto s_int16 MySetClockSpeed(register u_int32 clkHz) {
  int newMultiplier = 2;
  int retCode = -1;
  u_int32 f;
  int autoAdjust = !(clkHz & SET_CLOCK_NO_AUTO_ADJUST);
  int forceRf = (clkHz & SET_CLOCK_RF) ? 1 : 0;
  int suggestRamDelay=0, suggestCVDD=1500, suggestPOR=0;

  clkHz &= ~(SET_CLOCK_NO_AUTO_ADJUST|SET_CLOCK_RF);

  /* Go back to basic 1x clock from XTALI */
  timeCountAdd = 1;
  PERIP(SYSTEMPD) &= ~SYSTEMPD_XTALOFF;
  PERIP(ANA_CF1) &= ~ANA_CF1_XTDIV;
  PERIP(CLK_CF) &= ~(CLK_CF_GDIV2 | CLK_CF_GDIV256 |
		     CLK_CF_USBCLK | CLK_CF_FORCEPLL);
  PERIP(SYSTEMPD) &= ~SYSTEMPD_RTCCLK;
  PERIP32(timerCountReg) = (clockSpeed.extClkKHz>>1)-1;
  /* If old clock was RF clock, turn RF VCO off */
  if (clockSpeed.masterClkSrc == MASTER_CLK_SRC_RF) {
    PERIP(ANA_CF3) &= (ANA_CF3_GAIN2MASK | ANA_CF3_GAIN1MASK);
    PERIP(ANA_CF2) &= ~(ANA_CF2_UTM_ENA | ANA_CF2_2G_ENA);
  }
  clockSpeed.xtalPrescaler = 0;
  clockSpeed.div256 = 0;
  clockSpeed.div2 = 0;
  clockSpeed.masterClkSrc = MASTER_CLK_SRC_XTALI;
  SetUartSpeed(clockSpeed.uartByteSpeed*10);
  if (autoAdjust) {
    mV[MV_CVDD] = 1950;
    SetVoltages();
    Delay(1);
  }

  if (clkHz == SET_CLOCK_USB) {
    if (clockSpeed.extClkKHz != 12000) {
      forceRf = 1;
    }
    clkHz = (u_int32)60e6;
  }

  if (forceRf) {
    suggestCVDD = 1800;
    suggestPOR = 1;

    PERIP(ANA_CF3) |= ANA_CF3_480_ENA | 8*ANA_CF3_2GCNTR;
    PERIP(ANA_CF2) |= ANA_CF2_UTM_ENA | ANA_CF2_2G_ENA |
      ANA_CF2_HIGH_REF | ANA_CF2_REF_ENA;
    PERIP(FMCCF_LO) = 0xffff;      // => 60.000MHz USB @ 12.288MHz
    PERIP(FMCCF_HI) = 0x0387;      //
    PERIP(FM_CF)    = FM_CF_ENABLE|FM_CF_FM_ENA;

    InitRf();
    /* RF freq is in kHz, set to double because there is a divider to
       the core. */
    retCode = SetRfFreq(clkHz/500) ? 0 : -1;
 
    /* Activate USB clock */
    PERIP(CLK_CF) = 1*CLK_CF_MULT0;
    PERIP(0);
    PERIP(CLK_CF) = 1*CLK_CF_MULT0 | CLK_CF_USBCLK;
    PERIP(0);
    PERIP(CLK_CF) = 1*CLK_CF_MULT0 | CLK_CF_USBCLK | CLK_CF_FORCEPLL;

    clockSpeed.masterClkSrc = MASTER_CLK_SRC_RF;
    clockSpeed.cpuClkHz = clkHz/500*500;

    goto finally;
  }

  /* If requested clock is extClkKHz/2 or less, activate dividers. */
  if (clkHz <= (s_int32)clockSpeed.extClkKHz*500) {
    static u_int16 div[5][4] = {
      {1024, 1, 1, 1},
      { 512, 1, 0, 1},
      { 256, 1, 0, 0},
      {   4, 0, 1, 1},
      {   2, 0, 0, 1},
    };
    u_int16 *divP = div[0];
    int i=0;
    int shiftRight;
    u_int32 maxByteSpeed;

    while (i<4 && clkHz > (s_int32)clockSpeed.extClkKHz*1000 / divP[0]) {
      divP += 4;
    }
    clockSpeed.div256 = divP[1];
    clockSpeed.div2 = divP[2];
    clockSpeed.xtalPrescaler = divP[3];
    shiftRight =
      clockSpeed.xtalPrescaler + clockSpeed.div2 + clockSpeed.div256*8;
    maxByteSpeed = 122880>>shiftRight;
    if (shiftRight > 5) {
      shiftRight = 5;
    }

    if (clockSpeed.uartByteSpeed > maxByteSpeed) {
      printf("Warning: UART speed %ld bps too high for clock, "
	     "setting %ld bps\n",
	     clockSpeed.uartByteSpeed*10,
	     maxByteSpeed*10);
      clockSpeed.uartByteSpeed = maxByteSpeed;
      FlushUart();
    }
    PERIP32(timerCountReg) = (clockSpeed.extClkKHz>>(1+shiftRight))-1;
    if (clockSpeed.div256) {
      DacOff();
      PERIP(CLK_CF) |= CLK_CF_GDIV256;
    }
    if (clockSpeed.div2) PERIP(CLK_CF) |= CLK_CF_GDIV2;
    if (clockSpeed.xtalPrescaler) PERIP(ANA_CF1) |= ANA_CF1_XTDIV;
    MyForceClockMultiplier(2);
    timeCountAdd = divP[0]>>shiftRight;
    retCode = 0;
    SetUartSpeed(clockSpeed.uartByteSpeed*10);
    goto finally;
  }

  SetUartSpeed(clockSpeed.uartByteSpeed*10);

  if (clkHz == SET_CLOCK_RTC) {
    u_int16 extClock4KHz = 32768/4;
    u_int32 uartByteSpeed;

    if (clockSpeed.uartByteSpeed > 240) {
      printf("Warning: UART speed %ld bps too high for clock, "
	     "setting %d bps\n",
	     clockSpeed.uartByteSpeed*10,
	     240*10);
      clockSpeed.uartByteSpeed = 240;
      FlushUart();
    }

    uartByteSpeed = clockSpeed.uartByteSpeed*1000;
    while (uartByteSpeed > 65536UL) {
      extClock4KHz >>= 1;
      uartByteSpeed >>= 1;
    }
    DacOff();
    PERIP(SYSTEMPD) |= SYSTEMPD_RTCCLK;
    PERIP(SYSTEMPD) |= SYSTEMPD_XTALOFF;
    PERIP(UART_DIV) = UartDivider(extClock4KHz, (u_int16)uartByteSpeed);
#if 1
    /* These values have an inaccuracy of 37PPM = 3 seconds / 24 hours.
       It shouldn't matter for the time counter, and makes for acceptable
       though not perfect real-time performance: Delay(1) will last 13 ms. */
    timeCountAdd = (int)13;
    PERIP32(timerCountReg) = 212;
#else
    /* Best accurate values, but makes Delay(1) last a very long 125 ms. */
    timeCountAdd = (int)(1000/8);
    PERIP32(timerCountReg) = ((32768/8)>>1)-1;
#endif
    clockSpeed.masterClkSrc = MASTER_CLK_SRC_RTC;
    clockSpeed.cpuClkHz = 32768;
    retCode = 0;
    goto finally;
  }

  while ((f=((s_int32)clockSpeed.extClkKHz*500)*newMultiplier) <
	 clkHz && f < clockSpeed.speedLimitHz && newMultiplier < 32) {
    newMultiplier++;
  }
  if (f > clockSpeed.speedLimitHz) {
    newMultiplier--;   
    f=((s_int32)clockSpeed.extClkKHz*500)*newMultiplier;
  }
  if (newMultiplier >= 16) { /* If multiplier is over 16, it can't be odd */
    newMultiplier &= ~1;
    f=((s_int32)clockSpeed.extClkKHz*500)*newMultiplier;
  }

  if (f <= 13000000) {
    suggestCVDD = 1400;
  } else if (clkHz <= 49152000) {
    suggestCVDD = 1600;
  } else if (clkHz <= 61440000) {
    suggestCVDD = 1700;
    suggestPOR = 1;
  } else if (clkHz <= 86016000) {
    suggestCVDD = 1800;
    suggestRamDelay = 1;
    suggestPOR = 1;
  } else {
    suggestCVDD = 1950;
    suggestRamDelay = 1;
    suggestPOR = 1;
  }

  MyForceClockMultiplier(newMultiplier);

  PERIP(CLK_CF) |= CLK_CF_LCKCHK;
  PERIP(0);
  PERIP(CLK_CF) &= ~CLK_CF_LCKCHK;

  if (newMultiplier != 2 &&
      (PERIP(CLK_CF) & (CLK_CF_FORCEPLL | CLK_CF_LCKST)) !=
      (CLK_CF_FORCEPLL | CLK_CF_LCKST)) {
    goto finally;
  }

  retCode = 0;
 finally:
  if (clockSpeed.cpuClkHz >= 1000000 && !PERIP32(DAC_SRCL)) {
    PERIP32(DAC_SRCL) = 0x10000; /* Just some non-zero value. */
  }

  if (autoAdjust) {
    /* CVDD is 1.95 V since the start of this function. Now clock
       has been set, so we now can adjust other automatically adjusted
       parameters to their final values. */

    /* Turn POR to correct value. */
    PERIP(CF_ENABLE) = CF_KEY;
    if (suggestPOR) {
      PERIP(CF_CTRL) &= ~CF_CTRL_PORD;
    } else {
      PERIP(CF_CTRL) |= CF_CTRL_PORD;
    }

    /* Switch memory delay to correct value. */
    PERIP(SYSTEMPD) = (PERIP(SYSTEMPD) & ~SYSTEMPD_RAMDELAY_MASK) |
      (suggestRamDelay << SYSTEMPD_RAMDELAY_B);
    
    /* Finally, turn down voltage from 1.95 V to the final value. */
    mV[MV_CVDD] = suggestCVDD;
    SetVoltages();
  } /* if (autoAdjust) */

  return retCode;
}


auto s_int16 MySetClockSpeedShowError(register u_int32 clkHz)  {
  s_int16 ret;

  FlushUart();
  if (ret = MySetClockSpeed(clkHz)) {
     printf("SetClock: Couldn't set clock speed, result undefined!\n");
  }
  return ret;
}



int voltageValues[3];
int mV[3];

void GetVoltages(void) {
  voltageValues[MV_CVDD]  = (PERIP(REGU_VOLT) >> REGU_VOLT_CVDD_B ) & 0x1f;
  voltageValues[MV_IOVDD] = (PERIP(REGU_VOLT) >> REGU_VOLT_IOVDD_B) & 0x1f;
  voltageValues[MV_AVDD]  = (PERIP(REGU_VOLT) >> REGU_VOLT_AVDD_B ) & 0x1f;
  mV[MV_CVDD]  = 1325 + 25*(voltageValues[MV_CVDD]-GetTrim(ttVCore));
  mV[MV_IOVDD] = 1800 + 60*voltageValues[MV_IOVDD];
  mV[MV_AVDD]  = 2480 + 40*voltageValues[MV_AVDD];
}

s_int16 Sat5B(register s_int32 v) {
  if (v < 0) {
    return 0;
  } else if (v > 31) {
    return 31;
  }
  return (s_int16)v;
}

void SetVoltages(void) {
  voltageValues[MV_CVDD]  = Sat5B((mV[MV_CVDD] -1325+25/2)/25+GetTrim(ttVCore));
  voltageValues[MV_IOVDD] = Sat5B((mV[MV_IOVDD]-1800+60/2)/60);
  voltageValues[MV_AVDD]  = Sat5B((mV[MV_AVDD] -2480+40/2)/40);
  PERIP(REGU_VOLT) =
    (voltageValues[MV_CVDD]  << REGU_VOLT_CVDD_B) |
    (voltageValues[MV_IOVDD] << REGU_VOLT_IOVDD_B) |
    (voltageValues[MV_AVDD]  << REGU_VOLT_AVDD_B);
  PERIP(REGU_CF) |=  REGU_CF_REGCK;
  PERIP(REGU_CF) &= ~REGU_CF_REGCK;
  GetVoltages();
}



void PrintDevSpeeds(void) {
  int shiftRight = clockSpeed.xtalPrescaler +
    clockSpeed.div2 + clockSpeed.div256*8;
  u_int16 uartDiv = PERIP(UART_DIV);
  s_int32 uartSpeed =
    ((clockSpeed.masterClkSrc == MASTER_CLK_SRC_RTC) ?
     (s_int32)32768 : (s_int32)clockSpeed.extClkKHz*1000) / (s_int32)
    ((((uartDiv & UART_DIV_D1_MASK) >> UART_DIV_D1_B) + 1) *
     ((uartDiv & UART_DIV_D2_MASK) >> UART_DIV_D2_B)) >> shiftRight;
  s_int32 uartSpeedError = labs(uartSpeed-clockSpeed.uartByteSpeed*10);
  double spi0Speed =
    1e-3*clockSpeed.cpuClkHz / (double)
    (2 * (((PERIP(SPI0_CLKCF)&SPI_CC_CLKDIV_MASK) >> SPI_CC_CLKDIV_B) + 1));
  double spi1Speed =
    1e-3*clockSpeed.cpuClkHz / (double)
    (2 * (((PERIP(SPI1_CLKCF)&SPI_CC_CLKDIV_MASK) >> SPI_CC_CLKDIV_B) + 1));
  double nandSpeed =
    1e-3*clockSpeed.cpuClkHz / (double)
    (2 * (((PERIP(NF_CF)&NF_CF_WAITSTATES_MASK) >> NF_CF_WAITSTATES_B) + 1));
  double sdSpeed =
    1e-3*clockSpeed.cpuClkHz / (double)
    (2 * (((PERIP(SD_ST)&SD_ST_WAITSTATES_MASK) >> SD_ST_WAITSTATES_B) + 1)) *
    ((PERIP(SD_CF) & SD_CF_4BIT) ? 0.5 : 0.125);
  printf("  UART nominal %ld bps, real %ld bps (%3.1f%% error), reg 0x%04x\n",
	 10*clockSpeed.uartByteSpeed,
	 uartSpeed, 100.0*uartSpeedError/uartSpeed,
	 PERIP(UART_DIV));
  if (clockSpeed.cpuClkHz >= 1000000) {
    printf("  SPI0 %3.1f Mbit/s, SPI1 %3.1f Mbit/s, "
	   "NAND %3.1f MByte/s, SD %3.1f MByte/s\n",
	   spi0Speed*0.001, spi1Speed*0.001, nandSpeed*0.001, sdSpeed*0.001);
  } else {
    printf("  SPI0 %d kbit/s, SPI1 %d kbit/s, "
	   "NAND %d kByte/s, SD %d kByte/s\n",
	   (int)(spi0Speed+0.5), (int)(spi1Speed+0.5),
	   (int)(nandSpeed+0.5), (int)(sdSpeed+0.5));

  }
}


u_int16 GetDividerL(register u_int32 f, register u_int16 maxDiv) {
  u_int16 i;
  for (i=0; i<maxDiv; i++) {
    u_int32 t = clockSpeed.cpuClkHz / (u_int32)(2 * (i+1));
    if (t < f) {
      return i;
    }
  }
  return maxDiv;
}

/* GetDivider() returns cpuKHz / speedKHz / 2 - 1, rounded upwards. */
u_int16 GetDivider(register u_int16 maxSpeedKHz, register u_int16 maxVal) {
  u_int16 div = 0;
  u_int32 maxSpeedHzTimes2 = 2000L * maxSpeedKHz;
  if (maxSpeedHzTimes2 <= clockSpeed.cpuClkHz) {
    div = (u_int16)((clockSpeed.cpuClkHz + maxSpeedHzTimes2 - 1) /
		    maxSpeedHzTimes2);
    if (div > maxVal) {
      return maxVal;
    }
  }
  return div;
}

void SetSpiSpeed(register u_int32 f, register u_int16 spiPort) {
  u_int16 div = GetDividerL(f, SPI_CC_CLKDIV_MASK >> SPI_CC_CLKDIV_B);
  u_int16 reg = spiPort ? SPI1_CLKCF : SPI0_CLKCF;
  PERIP(reg) = (PERIP(reg) & ~(SPI_CC_CLKDIV_MASK)) | (div << SPI_CC_CLKDIV_B);
}

void SetNandSpeed(register u_int32 f) {
  u_int16 div = GetDividerL(f, NF_CF_WAITSTATES_MASK >> NF_CF_WAITSTATES_B);
  PERIP(NF_CF) =
    (PERIP(NF_CF) & ~(NF_CF_WAITSTATES_MASK)) | (div << NF_CF_WAITSTATES_B);
}

void SetSdSpeed(register u_int32 f) {
  u_int16 div = GetDividerL(f, SD_ST_WAITSTATES_MASK >> SD_ST_WAITSTATES_B);
  PERIP(SD_ST) =
    (PERIP(SD_ST) & ~(SD_ST_WAITSTATES_MASK)) | (div << SD_ST_WAITSTATES_B);
}




ioresult main(char *parameters) {
  int nParam, i;
  char *p = parameters;
  ioresult ret = S_OK;
  int nothingToDo = 1;
  int verbose = 0;
  int testSpeed = 0;
  int t;
  double ft;

  /* Is this VS1005g? */
  if (ReadIMem(0xffff) == 0xbbd7e455) {
    chipIsG = 1;
    timerCountReg = TIMER_T2L;
  }

  PERIP(UTM_TRIMRES) = GetTrim(ttUSB);
  GetVoltages();

  nParam = RunProgram("ParamSpl", parameters);
  for (i=0; i<nParam; i++) {
    if (!strcmp(p, "-h")) {
      printf("Usage: SetClock [x|fx|usb|rtc|-fx|-lx|-cv|-iv|-av|-uf|-sf|"
	     "-pd|-nf|-df|-rd|-wx|\n"
	     "                 -P|+P|-v|+v|-h]\n"
	     "x\tSet clock to x MHz, auto-adjust(1)\n"
	     "rx\tSet RF clock to 2x MHz, core to x MHz, auto-adjust(1)\n"
	     "usb\tSet clock to exactly 60 MHz, auto-adjust(1)\n"
	     "rtc\tSet to 32.768 kHz RTC clock, auto-adjust(1)\n"
	     "-fx\tSet clock to x MHz (USE WITH CAUTION!)\n"
	     "-frx\tSet RF clock to 2x MHz, core to x MHz (USE WITH CAUTION!)\n"
	     "\tx may also be \"usb\" or \"rtc\"\n"
	     "-lx\tLimit top speed to x MHz (USE WITH CAUTION!)\n"
	     "-cv\tSet CVDD  to v volts (USE WITH CAUTION!)\n"
	     "-iv\tSet IOVDD to v volts (USE WITH CAUTION!)\n"
	     "-av\tSet AVDD  to v volts (USE WITH CAUTION!)\n"
	     "-uf\tSet UART speed to f bps\n"
	     "-sf\tSet SPI0 speed to f MHz\n"
	     "-pf\tSet SPI1 speed to f MHz\n"
	     "-nf\tSet NAND FLASH speed to f MHz\n"
	     "-df\tSet SD speed to f MHz\n"
	     "-rd\tSet RAM delay line to d (range 0(slow)-3(fast)) (USE WITH CAUTION!)\n"
	     "-wx\tWait x milliseconds\n"
	     "-P|+P\tDisable/Enable Power-On-Reset (USE WITH CAUTION!)\n"
	     "-v/+v\tVerbose on/off\n"
	     "-t\tTest execution speed\n"
	     "-h\tShow this help\n"
	     "\n(1) Auto-adjustment affects CVDD, RAM Delay, and POR.\n");
      goto finally;
    } else if (!strcmp(p, "-v")) {
      verbose = 1;
    } else if (!strcmp(p, "+v")) {
      verbose = -1;
    } else if (!strncmp(p, "-l", 2)) {
      u_int32 speedLimitMHz = strtol(p+2, NULL, 0);
      if (speedLimitMHz) {
	clockSpeed.speedLimitHz = 1000000*speedLimitMHz;
      }
      nothingToDo = 0;
    } else if (!strncmp(p, "-c", 2)) {
      mV[MV_CVDD] = (int)(atof(p+2)*1000);
      SetVoltages();
      nothingToDo = 0;
    } else if (!strncmp(p, "-i", 2)) {
      mV[MV_IOVDD] = (int)(atof(p+2)*1000);
      SetVoltages();
      nothingToDo = 0;
    } else if (!strncmp(p, "-a", 2)) {
      mV[MV_AVDD] = (int)(atof(p+2)*1000);
      SetVoltages();
      nothingToDo = 0;
    } else if (!strncmp(p, "-u", 2)) {
      u_int32 uartSpeed = strtol(p+2, NULL, 0);
      if (clockSpeed.masterClkSrc == MASTER_CLK_SRC_RTC) {
	u_int16 extClock4KHz = 32768/4;
	u_int32 uartByteSpeed = uartSpeed*(1000/10);
	while (uartByteSpeed > 65536UL) {
	  extClock4KHz >>= 1;
	  uartByteSpeed >>= 1;
	}
	PERIP(UART_DIV) = UartDivider(extClock4KHz, (u_int16)uartByteSpeed);
      } else {
	SetUartSpeed(uartSpeed);
      }
      /* Following line needed for VSOS 3.35 and older */
      clockSpeed.uartByteSpeed = uartSpeed/10;
      nothingToDo = 0;
    } else if (!strncmp(p, "-s", 2)) {
      SetSpiSpeed((s_int32)(atof(p+2)*1e6), 0);
      nothingToDo = 0;
    } else if (!strncmp(p, "-p", 2)) {
      SetSpiSpeed((s_int32)(atof(p+2)*1e6), 1);
      nothingToDo = 0;
    } else if (!strncmp(p, "-n", 2)) {
      SetNandSpeed((s_int32)(atof(p+2)*1e6));
      nothingToDo = 0;
    } else if (!strncmp(p, "-d", 2)) {
      SetSdSpeed((s_int32)(atof(p+2)*1e6));
      nothingToDo = 0;
    } else if (!strncmp(p, "-r", 2)) {
      u_int16 t = atoi(p+2);
      if (t > 3) {
	t = 3;
      }
      t = (PERIP(SYSTEMPD) & ~SYSTEMPD_RAMDELAY_MASK) |
	(t << SYSTEMPD_RAMDELAY_B);
      PERIP(SYSTEMPD) = t;
      nothingToDo = 0;
    } else if (!strncmp(p, "-w", 2)) {
      Delay(atoi(p+2));
      nothingToDo = 0;
    } else if (!strcmp(p, "-P")) {
      PERIP(CF_ENABLE) = CF_KEY;
      PERIP(CF_CTRL) |= CF_CTRL_PORD;
      nothingToDo = 0;
    } else if (!strcmp(p, "+P")) {
      PERIP(CF_ENABLE) = CF_KEY;
      PERIP(CF_CTRL) &= ~CF_CTRL_PORD;
      nothingToDo = 0;
    } else if (!strcmp(p, "-t")) {
      testSpeed = 1;
      nothingToDo = 0;
    } else if (!strncmp(p, "-f", 2)) {
      ret = S_ERROR;
      if (!strcmp(p+2, "usb")) {
	if (MySetClockSpeedShowError(SET_CLOCK_USB|SET_CLOCK_NO_AUTO_ADJUST)) {
	  goto finally;
	}
      } else if (!strcmp(p+2, "rtc")) {
	if (MySetClockSpeedShowError(SET_CLOCK_RTC|SET_CLOCK_NO_AUTO_ADJUST)) {
	  goto finally;
	}
      } else if (p[2] == 'r' && (ft = strtod(p+3, NULL)) >= 0.0) {
	if (MySetClockSpeedShowError((u_int32)(ft*1000000.0+0.5)|
				     SET_CLOCK_NO_AUTO_ADJUST|SET_CLOCK_RF)) {
	  goto finally;
	}
      } else if ((ft = strtod(p+2, NULL)) >= 0.0) {
	if (MySetClockSpeedShowError((u_int32)(ft*1000000.0+0.5)|
				     SET_CLOCK_NO_AUTO_ADJUST)) {
	  goto finally;
	}
      } else {
	printf("Invalid parameter %s\n", p);
	goto finally;
      }
      ret = S_OK;
      nothingToDo = 0;
    } else if (!strcmp(p, "usb")) {
      if (MySetClockSpeedShowError(SET_CLOCK_USB)) {
	goto finally;
      }
      nothingToDo = 0;
    } else if (!strcmp(p, "rtc")) {
      if (MySetClockSpeedShowError(SET_CLOCK_RTC)) {
	goto finally;
      }
      nothingToDo = 0;
    } else if (p[0] == 'r' && (ft = strtod(p+1, NULL)) > 0.0) {
      if (MySetClockSpeedShowError((u_int32)(ft*1000000.0+0.5)|
				   SET_CLOCK_NO_AUTO_ADJUST|SET_CLOCK_RF)) {
	goto finally;
      }
      nothingToDo = 0;
    } else if ((ft = strtod(p, NULL)) > 0.0) {
      if (MySetClockSpeedShowError((u_int32)(ft*1000000.0+0.5))) {
	goto finally;
      }
      nothingToDo = 0;
    } else {
      printf("E: Unknown parameter \"%s\"\n", p);
      ret = S_ERROR;
      goto finally;
    }
    p += strlen(p)+1;
  }

  if ((nothingToDo || verbose>0) && verbose >= 0) {
    static char *master[4] = {"XTALI", "PLL", "RF", "RTC"};
    static const char *vs1005Type[2][4] =
      {{"VS1205h", "VS1005h", "VS8005h", "ERROR"},
       {"VS1205g", "VS1005g", "VS8005g", "ERROR"}};
    printf("SetClock running on %s, clocks:\n"
	   "  CLKI %5.3f MHz, limit %5.3f MHz, src %s, RAM Delay %d, POR %s\n",
	   vs1005Type[chipIsG][GetVS1005SubType()],
	   clockSpeed.cpuClkHz*1e-6, clockSpeed.speedLimitHz*1e-6,
	   (clockSpeed.masterClkSrc<4) ? master[clockSpeed.masterClkSrc]:"??",

	   (PERIP(SYSTEMPD)&SYSTEMPD_RAMDELAY_MASK)>>SYSTEMPD_RAMDELAY_B,
	   (PERIP(CF_CTRL) & CF_CTRL_PORD) ? "off" : "on");
    printf("  CVDD %4.3fV(%d), IOVDD %3.2fV(%d), AVDD %3.2fV(%d)\n",
	   0.001*mV[MV_CVDD],  voltageValues[MV_CVDD],
	   0.001*mV[MV_IOVDD], voltageValues[MV_IOVDD],
	   0.001*mV[MV_AVDD],  voltageValues[MV_AVDD]);
    PrintDevSpeeds();
    printf("Uptime %3.1fs, time counter interrupt %2.1f times/s\n",
	   0.001*ReadTimeCount(), 1000.0/timeCountAdd);
    PrintOTP();
  }
  if (clockSpeed.masterClkSrc == MASTER_CLK_SRC_PLL &&
      (PERIP(CLK_CF) & (CLK_CF_FORCEPLL|CLK_CF_LCKST)) == CLK_CF_FORCEPLL) {
    printf("SetClock: PLL lock lost, speed may be unexpected\n");
  }

  if (testSpeed) {
    TestSpeed();
  }

 finally:
  return ret;
}
