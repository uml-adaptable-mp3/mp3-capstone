#include <vstypes.h>
#include <hwLocks.h>


#define MAKE2(l,r) ((((r)&0xFF)<<8)|((l)&0xFF))

__y const u_int16 hwLockGpioBit[] = {
  MAKE2(HLIO_NF_B, HLIO_NF_B),			/* GPIO 0:00 & 0:01 */
  MAKE2(HLIO_NF_B, HLIO_NF_B),			/* GPIO 0:02 & 0:03 */
  MAKE2(HLIO_NF_B, HLIO_NF_B),			/* GPIO 0:04 & 0:05 */
  MAKE2(HLIO_NF_B, HLIO_NF_B),			/* GPIO 0:06 & 0:07 */
  MAKE2(HLIO_NF_B, HLIO_NF_B),			/* GPIO 0:08 & 0:09 */
  MAKE2(HLIO_NF_B, HLIO_0_11_B),		/* GPIO 0:10 & 0:11 */
  MAKE2(HLIO_SPDIF_B, HLIO_SPDIF_B),		/* GPIO 0:12 & 0:13 */
  MAKE2(HLIO_0_14_B, HLIO_0_15_B),		/* GPIO 0:14 & 0:15 */

  MAKE2(HLIO_1_00_B, HLIO_SPI0_B),		/* GPIO 1:00 & 0:01 */
  MAKE2(HLIO_SPI0_B, HLIO_SPI0_B),		/* GPIO 1:02 & 0:03 */
  MAKE2(HLIO_1_04_B, HLIO_SPI1_B),		/* GPIO 1:04 & 0:05 */
  MAKE2(HLIO_SPI1_B, HLIO_SPI1_B),		/* GPIO 1:06 & 0:07 */
  MAKE2(HLIO_UART_B, HLIO_UART_B),		/* GPIO 1:08 & 0:09 */
  MAKE2(HLIO_I2S_B, HLIO_I2S_B),		/* GPIO 1:10 & 0:11 */
  MAKE2(HLIO_I2S_B, HLIO_I2S_B),		/* GPIO 1:12 & 0:13 */
  MAKE2(HLIO_I2SCLK_B, HLIO_1_15_B),		/* GPIO 1:14 & 0:15 */

  MAKE2(HLIO_JTAG_B, HLIO_JTAG_B),		/* GPIO 2:00 & 0:01 */
  MAKE2(HLIO_JTAG_B, HLIO_JTAG_B),		/* GPIO 2:02 & 0:03 */
  MAKE2(HLIO_JTAG_B, HLIO_SD_B),		/* GPIO 2:04 & 0:05 */
  MAKE2(HLIO_SD_B, HLIO_SD_B),			/* GPIO 2:06 & 0:07 */
  MAKE2(HLIO_SD_B, HLIO_SD_B),			/* GPIO 2:08 & 0:09 */
  MAKE2(HLIO_SD_B, HLIO_ETH_B),			/* GPIO 2:10 & 0:11 */
  MAKE2(HLIO_ETH_B, HLIO_ETH_B),		/* GPIO 2:12 & 0:13 */
  /*MAKE2(-1, -1),				/* GPIO 2:14 & 0:15 */
};

auto u_int32 PinToGpioHwLockMask(register __d0 u_int16 pin) {
  if (pin >= 2*sizeof(hwLockGpioBit)/sizeof(hwLockGpioBit[0])) {
    return 0xFFFFFFFF; /* Error: no such GPIO pin */
  }
  return 1L << ((hwLockGpioBit[pin>>1] >> ((pin&1)*8)) & 0xFF);
};
