#ifndef __VS1005_CLOCK_SPEED_H__
#define __VS1005_CLOCK_SPEED_H__

#include <vstypes.h>

#define SET_CLOCK_NO_AUTO_ADJUST 0x80000000UL
#define SET_CLOCK_USB            0x40000000UL
#define SET_CLOCK_RTC            0x40000001UL
#define SET_CLOCK_RF             0x20000000UL

#ifndef ASM

auto s_int16 ActivateUsb60MHzClock(void);
auto s_int16 DeactivateUsb60MHzClock(void);

#define CLOCK_RF_PLL_RESERVED_MASTER	1
#define CLOCK_RF_PLL_RESERVED_RF	2
#define CLOCK_RF_PLL_RESERVED_UNKNOWN	0x8000U

#define MASTER_CLK_SRC_XTALI		0
#define MASTER_CLK_SRC_PLL		1
#define MASTER_CLK_SRC_RF		2
#define MASTER_CLK_SRC_RTC		3

#define CLOCK_DIV_MASK_XTAL_DIV2	1
#define CLOCK_DIV_MASK_PRESCALER_DIV2	2
#define CLOCK_DIV_MASK_PRESCALER_DIV256	4


/**
   Initialized system clock parameters and sets clock to basic 1X mode.

   \param extClockKHz external clock frequency in kHz.
*/
auto s_int16 InitClockSpeed(register u_int16 extClockKHz,
			    register u_int32 uartByteSpeed);


/**
   Set absolute upper speed limit for clock.

   \param speedLimitHz external clock frequency in Hz.
*/
auto s_int16 SetClockSpeedLimit(register u_int32 speedLimitHz);


/**

   Sets system clock.

   \param n Set clock to at least n Hz (but never higher than speed limit).
	If n == SET_CLOCK_USB, then set exactly 60 MHz (only possible if
	crystal is exactly 12.000 or 12.288 MHz).

 */
auto s_int16 SetClockSpeed(register u_int32 clkHz);

/**

   Sets system clock dividers.

   \param clockDividerMask CLOCK_DIV_MASK_XTAL_DIV2 = divide input clock by 2.
   	CLOCK_DIV_MASK_PRESCALER_DIV2 = divide input clock by 2 in prescaler.
	CLOCK_DIV_MASK_PRESCALER_DIV256 = divide input clock by 256 in prescaler.
	All combinations are allowed, so maximum divider is 2*256*2 = 1024.
 */
auto u_int16 SetClockDividers(register u_int16 clockDividerMask);


/**

   Force clock multiplier.

   \param clockX Clock multiplier multiplied by 2. E.g. 2 = 1x clock ~= 12 MHz.
	Note that this function will not have any effect unless clock
	source is either XTALI or PLL. Note also that if new multiplier would
	cause exceeding maximum speed limit, it will be automatically lowered.
   \return The new multiplier.
 */
auto u_int16 ForceClockMultiplier(register u_int16 clockX);

/**
   Get current clock multiplier.
   \return Current clock multiplier multiplied by 2. E.g. 2 = 1x.
*/
auto u_int16 GetClockMultiplier(void);

struct ClockSpeed {
  u_int32 cpuClkHz;	/**< Current clock frequency in Hz */
  u_int32 peripClkHz;	/**< Current peripheral clock frequency in Hz */
  u_int32 speedLimitHz;	/**< Speed limit in Hz */
  u_int16 extClkKHz;	/**< External clock frequency in kHz */
  u_int16 xtalPrescaler;/**< Divide by 2 right after crystal input */
  u_int16 div256;	/**< Divide by 256 in clock prescaler */
  u_int16 div2;		/**< Divide by 2 in clock prescaler */
  u_int16 pllPreDiv;	/**< Divide by 2 in PLL */
  u_int16 pllMult;	/**< PLL clock multiplier */
  u_int16 masterClkSrc; /**< Source for master clock */
  u_int16 rtcAvailable;	/**< RTC clock available */
  u_int16 rfPllReserved;/**< Is RF PLL reserved by someone? */
  u_int32 uartByteSpeed;/**< UART speed bytes/second */
};


auto void DelayMicroSec(u_int32 microSec);

auto void CalcClockSpeed(void);

extern struct ClockSpeed clockSpeed;

#endif /* !ASM */

#endif /* !__VS1005_USB_CLOCK_SPEED_H__ */
