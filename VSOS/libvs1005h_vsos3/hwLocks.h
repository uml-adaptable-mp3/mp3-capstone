/**
  \file hwLocks.h Methods for locking and releasing VS1005g hardware resources.

  To obtain hardware resources, call \a ObtainHwLocksIPB(), and to release them
  call \a ReleaseHwLocksIPB().
*/

/*
  VS1005g hardware locks. (C) 2013 VLSI Solution Oy

  Revision History
  2013-12-20  1.04  HH  Added User Locks.
  2013-11-01  1.03  HH  Debugged and cleaned. Now works in all known conditions.
  2013-10-15  1.02  HH  Made using AttemptHwLocksBIP() possible in interrupts.
                        Using ObtainHwLocksBIP() still is not possible in
                        interrupts.
  2013-06-05  1.01  HH  Made unlocking possible in interrupts.
                        (Note: Unlocking takes enough time that the interrupt
                         routine needs to be interruptable.)
  2013-04-12  1.00  HH  First revision.

  There are three kinds of hardware locks (HL):
  - HLB_:  Lock common buffers.
  - HLIO_: Lock physical I/O pins or pin groups.
  - HLP_:  Lock peripheral hardware.
*/

#ifndef VS1005G_HW_LOCKS
#define VS1005G_HW_LOCKS



#ifndef ASM
#include <vstypes.h>
#include <lists.h>
#include <exec.h>
#endif /* !ASM */



/* Buffer resources */
#define HW_LOCK_BUFFER_GROUP 0

#define HLB_0_B		 0	/**< Buffer lock bit for 0x0000..0x00FF (in words)		*/
#define HLB_1_B		 1	/**< Buffer lock bit for 0x0100..0x01FF (in words)		*/
#define HLB_2_B		 2	/**< Buffer lock bit for 0x0200..0x02FF (in words)		*/
#define HLB_3_B		 3	/**< Buffer lock bit for 0x0300..0x03FF (in words)		*/
#define HLB_4_B		 4	/**< Buffer lock bit for 0x0400..0x04FF (in words)		*/
#define HLB_5_B		 5	/**< Buffer lock bit for 0x0500..0x05FF (in words)		*/
#define HLB_USER_0_B	 6	/**< User-defined lock 0 (doesn't have to be buffer lock)	*/
#define HLB_USER_1_B	 7	/**< User-defined lock 1 (doesn't have to be buffer lock)	*/
#define HLB_USER_2_B	 8	/**< User-defined lock 2 (doesn't have to be buffer lock)	*/
#define HLB_USER_3_B	 9	/**< User-defined lock 3 (doesn't have to be buffer lock)	*/
#define BUFFER_HW_LOCKS	10 /**< Number of buffer hardware locks */

#define HLB_NONE         0		/**< HLB empty mask: No buffers */
#define HLB_0 (1UL<<HLB_0_B)		/**< Buffer lock mask for 0x0000..0x00FF (in words)		*/
#define HLB_1 (1UL<<HLB_1_B)		/**< Buffer lock mask for 0x0100..0x01FF (in words)		*/
#define HLB_2 (1UL<<HLB_2_B)		/**< Buffer lock mask for 0x0200..0x02FF (in words)		*/
#define HLB_3 (1UL<<HLB_3_B)		/**< Buffer lock mask for 0x0300..0x03FF (in words)		*/
#define HLB_4 (1UL<<HLB_4_B)		/**< Buffer lock mask for 0x0400..0x04FF (in words)		*/
#define HLB_5 (1UL<<HLB_5_B)		/**< Buffer lock mask for 0x0500..0x05FF (in words)		*/
#define HLB_USER_0 (1UL<<HLB_USER_0_B) 	/**< User-defined lock 0 (doesn't have to be buffer lock)	*/
#define HLB_USER_1 (1UL<<HLB_USER_1_B) 	/**< User-defined lock 1 (doesn't have to be buffer lock)	*/
#define HLB_USER_2 (1UL<<HLB_USER_2_B) 	/**< User-defined lock 2 (doesn't have to be buffer lock)	*/
#define HLB_USER_3 (1UL<<HLB_USER_3_B) 	/**< User-defined lock 3 (doesn't have to be buffer lock)	*/

/* Physical I/O pins / pin groups */
#define HW_LOCK_IO_GROUP 1

#define HLIO_NF_B 	 0	/**< I/O lock bit for pin(s) 9-18,24	Nand Flash		*/
#define HLIO_NFCE_B 	 1	/**< I/O lock bit for pin(s) 26		Nand Flash CE		*/
#define HLIO_0_11_B HLIO_NFCE_B	/**< I/O lock bit for pin(s) 26		GPIO Port 0, Bit 11	*/
#define HLIO_SPI1_B	 2	/**< I/O lock bit for pin(s) 20,22-23	SPI 1			*/
#define HLIO_SPI1CS_B	 3	/**< I/O lock bit for pin(s) 19		SPI 1 CS		*/
#define HLIO_1_04_B HLIO_SPI1CS_B/**< I/O lock bit for pin(s) 19	GPIO Port 1, Bit 4 	*/
#define HLIO_SPI0_B	 4	/**< I/O lock bit for pin(s) 28-30	SPI 0			*/
#define HLIO_SPI0CS_B	 5	/**< I/O lock bit for pin(s) 27		SPI 0 CS		*/
#define HLIO_1_00_B HLIO_SPI0CS_B/**< I/O lock bit for pin(s) 27	GPIO Port 1, Bit 0 	*/
#define HLIO_JTAG_B	 6	/**< I/O lock bit for pin(s) 31-35	JTAG			*/
#define HLIO_UART_B	 7	/**< I/O lock bit for pin(s) 37-38	UART			*/
#define HLIO_I2S_B	 8	/**< I/O lock bit for pin(s) 39-42	I2S			*/
#define HLIO_I2SCLK_B	 9	/**< I/O lock bit for pin(s) 44		I2S clock output	*/
#define HLIO_SD_B	10	/**< I/O lock bit for pin(s) 45-50	SD card			*/
#define HLIO_ETH_B	11	/**< I/O lock bit for pin(s) 51,53,55	Ethernet		*/
#define HLIO_DIA_B HLIO_ETH_B	/**< I/O lock bit for pin(s) 51,53,55	Digital ADC inputs	*/
#define HLIO_1_15_B	12	/**< I/O lock bit for pin(s) 52		GPIO Port 1, bit 15	*/
#define HLIO_SPDIF_B	13	/**< I/O lock bit for pin(s) 56-57	S/PDIF in and out	*/
#define HLIO_0_14_B	14	/**< I/O lock bit for pin(s) 58		GPIO Port 0, bit 14	*/
#define HLIO_0_15_B	15	/**< I/O lock bit for pin(s) 59		GPIO Port 0, bit 15	*/
#define HLIO_PWRBTN_B	16	/**< I/O lock bit for pin(s) 63		Power button		*/
#define HLIO_AUX0_B	17	/**< I/O lock bit for pin(s) 67		Sar AD0			*/
#define HLIO_L2I2_B HLIO_AUX0_B	/**< I/O lock bit for pin(s) 67		Line 2 input 2		*/
#define HLIO_AUX1_B	18	/**< I/O lock bit for pin(s) 68		Sar AD1			*/
#define HLIO_L2I1_B HLIO_AUX1_B	/**< I/O lock bit for pin(s) 68		Line 2 input 1		*/
#define HLIO_AUX2_B	19	/**< I/O lock bit for pin(s) 69		Sar AD2			*/
#define HLIO_L1I3_B HLIO_AUX2_B	/**< I/O lock bit for pin(s) 69		Line 1 input 3		*/
#define HLIO_AUX3_B	20	/**< I/O lock bit for pin(s) 70		Sar AD3			*/
#define HLIO_L3I2_B HLIO_AUX3_B	/**< I/O lock bit for pin(s) 70		Line 3 input 2		*/
#define HLIO_MIC2P_B HLIO_AUX3_B/**< I/O lock bit for pin(s) 70		Mic 2 positive		*/
#define HLIO_AUX4_B	21	/**< I/O lock bit for pin(s) 71		Sar AD4			*/
#define HLIO_L3I1_B HLIO_AUX4_B	/**< I/O lock bit for pin(s) 71		Line 3 input 1		*/
#define HLIO_MIC2N_B HLIO_AUX4_B/**< I/O lock bit for pin(s) 71		Mic 2 negative		*/
#define HLIO_MIC1P_B	22	/**< I/O lock bit for pin(s) 72		Mic 1 positive		*/
#define HLIO_L1I2_B MIC1P_B    	/**< I/O lock bit for pin(s) 72		Line 1 input 2		*/
#define HLIO_MIC1N_B	23	/**< I/O lock bit for pin(s) 73		Mic 1 negative		*/
#define HLIO_L1I1_B MIC1N_B	/**< I/O lock bit for pin(s) 73		Line 1 input 2		*/
#define HLIO_FM_B	24	/**< I/O lock bit for pin(s) 75-76	FM receiver		*/
#define HLIO_USB_B	25	/**< I/O lock bit for pin(s) 85-87	USB			*/
#define HLIO_PWM_B	26	/**< I/O lock bit for pin(s) 88		PWM output		*/
#define IO_HW_LOCKS	27 /**< Number of I/O hardware locks */

#define HLIO_NONE   0	/**< HLIO empty mask: No I/O */
#define HLIO_NF     (1UL<<HLIO_NF_B)	/**< I/O lock mask for pin(s) 9-18,24	Nand Flash		*/
#define HLIO_NFCE   (1UL<<HLIO_NFCE_B)	/**< I/O lock mask for pin(s) 26	Nand Flash CE		*/
#define HLIO_0_11   (1UL<<HLIO_0_11_B)	/**< I/O lock mask for pin(s) 26	GPIO Port 0, Bit 11	*/
#define HLIO_SPI1   (1UL<<HLIO_SPI1_B)	/**< I/O lock mask for pin(s) 20-23	SPI 1			*/
#define HLIO_SPI1CS (1UL<<HLIO_SPI1CS_B)/**< I/O lock mask for pin(s) 19	SPI 1 CS		*/
#define HLIO_1_04   (1UL<<HLIO_1_04_B)	/**< I/O lock mask for pin(s) 19	GPIO Port 1, Bit 4 	*/
#define HLIO_SPI0   (1UL<<HLIO_SPI0_B)	/**< I/O lock mask for pin(s) 28-30	SPI 0			*/
#define HLIO_SPI0CS (1UL<<HLIO_SPI0CS_B)/**< I/O lock mask for pin(s) 27	SPI 0 CS		*/
#define HLIO_1_00   (1UL<<HLIO_1_00_B)	/**< I/O lock mask for pin(s) 27	GPIO Port 1, Bit 0 	*/
#define HLIO_JTAG   (1UL<<HLIO_JTAG_B)	/**< I/O lock mask for pin(s) 31-35	JTAG			*/
#define HLIO_UART   (1UL<<HLIO_UART_B)	/**< I/O lock mask for pin(s) 37-38	UART			*/
#define HLIO_I2S    (1UL<<HLIO_I2S_B)	/**< I/O lock mask for pin(s) 39-42	I2S			*/
#define HLIO_I2SCLK (1UL<<HLIO_I2SCLK_B)/**< I/O lock mask for pin(s) 44	I2S clock output	*/
#define HLIO_SD     (1UL<<HLIO_SD_B)	/**< I/O lock mask for pin(s) 45-50	SD card			*/
#define HLIO_ETH    (1UL<<HLIO_ETH_B)	/**< I/O lock mask for pin(s) 51,53,55	Ethernet		*/
#define HLIO_DIA    (1UL<<HLIO_DIA_B)	/**< I/O lock mask for pin(s) 51,53,55	Digital ADC inputs	*/
#define HLIO_1_15   (1UL<<HLIO_1_15_B)	/**< I/O lock mask for pin(s) 52	GPIO Port 1, bit 15	*/
#define HLIO_SPDIF  (1UL<<HLIO_SPDIF_B)	/**< I/O lock mask for pin(s) 56-57	S/PDIF in and out	*/
#define HLIO_0_14   (1UL<<HLIO_0_14_B)	/**< I/O lock mask for pin(s) 58	GPIO Port 0, bit 14	*/
#define HLIO_0_15   (1UL<<HLIO_0_15_B)	/**< I/O lock mask for pin(s) 59	GPIO Port 0, bit 15	*/
#define HLIO_PWRBTN (1UL<<HLIO_PWRBTN_B)/**< I/O lock mask for pin(s) 63	Power button		*/
#define HLIO_AUX0   (1UL<<HLIO_AUX0_B)	/**< I/O lock mask for pin(s) 67	Sar AD0			*/
#define HLIO_L2I2   (1UL<<HLIO_L2I2_B)	/**< I/O lock mask for pin(s) 67	Line 2 input 2		*/
#define HLIO_AUX1   (1UL<<HLIO_AUX1_B)	/**< I/O lock mask for pin(s) 68	Sar AD1			*/
#define HLIO_L2I1   (1UL<<HLIO_L2I1_B)	/**< I/O lock mask for pin(s) 68	Line 2 input 1		*/
#define HLIO_AUX2   (1UL<<HLIO_AUX2_B)	/**< I/O lock mask for pin(s) 69	Sar AD2			*/
#define HLIO_L1I3   (1UL<<HLIO_L1I3_B)	/**< I/O lock mask for pin(s) 69	Line 1 input 3		*/
#define HLIO_AUX3   (1UL<<HLIO_AUX3_B)	/**< I/O lock mask for pin(s) 70	Sar AD3			*/
#define HLIO_L3I2   (1UL<<HLIO_L3I2_B)	/**< I/O lock mask for pin(s) 70	Line 3 input 2		*/
#define HLIO_MIC2P  (1UL<<HLIO_MIC2P_B)	/**< I/O lock mask for pin(s) 70	Mic 2 positive		*/
#define HLIO_AUX4   (1UL<<HLIO_AUX4_B)	/**< I/O lock mask for pin(s) 71	Sar AD4			*/
#define HLIO_L3I1   (1UL<<HLIO_L3I1_B)	/**< I/O lock mask for pin(s) 71	Line 3 input 1		*/
#define HLIO_MIC2N  (1UL<<HLIO_MIC2N_B) /**< I/O lock mask for pin(s) 71	Mic 2 negative		*/
#define HLIO_MIC1P  (1UL<<HLIO_MIC1P_B)	/**< I/O lock mask for pin(s) 72	Mic 1 positive		*/
#define HLIO_L1I2   (1UL<<HLIO_L1I2_B)	/**< I/O lock mask for pin(s) 72	Line 1 input 2		*/
#define HLIO_MIC1N  (1UL<<HLIO_MIC1N_B)	/**< I/O lock mask for pin(s) 73	Mic 1 negative		*/
#define HLIO_L1I1   (1UL<<HLIO_L1I1_B)	/**< I/O lock mask for pin(s) 73	Line 1 input 2		*/
#define HLIO_FM     (1UL<<HLIO_FM_B)	/**< I/O lock mask for pin(s) 75-76	FM receiver		*/
#define HLIO_USB    (1UL<<HLIO_USB_B)	/**< I/O lock mask for pin(s) 85-87	USB			*/
#define HLIO_PWM    (1UL<<HLIO_PWM_B)	/**< I/O lock mask for pin(s) 88	PWM output		*/

// Bus arbiter in VS1005 Developer Board
#define HLIO_CS_SELECT (HLIO_NFCE|HLIO_1_15|HLIO_SPI0CS)	/**< I/O pins 26, 52, 27 */



/* Peripheral hardware */
#define HW_LOCK_PERIP_GROUP 2

#define HLP_DAC_B	 0	/**< Peripheral lock bit for Primary audio path			*/
#define HLP_USB_B	 1	/**< Peripheral lock bit for USB				*/
#define HLP_ETH_B	 2	/**< Peripheral lock bit for Xperip: Ethernet			*/
#define HLP_SPISLAVE_B	 3	/**< Peripheral lock bit for Xperip: Slave SPI			*/
#define HLP_NAND_B	 4	/**< Peripheral lock bit for Xperip: NAND Flash			*/
#define HLP_SD_B	 5	/**< Peripheral lock bit for Xperip: SD Card			*/
#define HLP_RSEN_B	 6	/**< Peripheral lock bit for Xperip: Reed-Solomon encoder	*/
#define HLP_RSDE_B	 7	/**< Peripheral lock bit for Xperip: Reed-Solomon decoder	*/
#define HLP_SPI0_B	 8	/**< Peripheral lock bit for SPI port 0				*/
#define HLP_SPI1_B	 9	/**< Peripheral lock bit for SPI port 1				*/
#define HLP_MAC1_B	10	/**< Peripheral lock bit for FM decimation-by-6 DEC6 filter	*/
#define HLP_MAC0_B	11	/**< Peripheral lock bit for ADC 1 & 2				*/
#define HLP_GPIO0_B	12	/**< Peripheral lock bit for GPIO port 0 including bit engines	*/
#define HLP_GPIO1_B	13	/**< Peripheral lock bit for GPIO port 1 including bit engines	*/
#define HLP_GPIO2_B	14	/**< Peripheral lock bit for GPIO port 2 including bit engines	*/
#define HLP_MAC2_B	15	/**< Peripheral lock bit for ADC 3 (mono ADC)			*/
#define HLP_I2S_B	16	/**< Peripheral lock bit for I2S transmitter/receiver		*/
#define HLP_UART_B	17	/**< Peripheral lock bit for UART				*/
#define HLP_TIMER0_B	18	/**< Peripheral lock bit for Timer 0				*/
#define HLP_TIMER1_B	19	/**< Peripheral lock bit for Timer 1				*/
#define HLP_TIMER2_B	20	/**< Peripheral lock bit for Timer 2				*/
#define HLP_FM_B	21	/**< Peripheral lock bit for FM Receiver at full rate (192 kHz)	*/
#define HLP_SRC_B	22	/**< Peripheral lock bit for Filterless sample rate converter	*/
#define HLP_DAOSET_B	23	/**< Peripheral lock bit for Secondary audio path		*/
#define HLP_RTC_B	24	/**< Peripheral lock bit for Real-time clock			*/
//#define HLP_STX_B	xx	/**< Peripheral lock bit for S/PDIF transmit			*/
//#define HLP_SRX_B	xx	/**< Peripheral lock bit for S/PDIF receiver			*/
#define HLP_REGU_B	25	/**< Peripheral lock bit for Power button			*/
#define HLP_PWM_B	26	/**< Peripheral lock bit for Pulse-width modulator		*/
#define HLP_SAR_B	27	/**< Peripheral lock bit for 10-bit SAR ADC			*/
#define HLP_PLL_B	28	/**< Peripheral lock bit for PLL clock				*/
#define HLP_RFVCO_B	29	/**< Peripheral lock bit for RF VCO clock			*/
#define HLP_XPERIP_IF_B	30	/**< Peripheral lock bit for Xperip DSP interface.		*/
#define PERIP_HW_LOCKS	31 /**< Number of peripheral hardware locks */


#define HLP_NONE     0	/**< Peripheral lock mask for HLP empty mask: No peripherals */
#define HLP_DAC      (1UL<<HLP_DAC_B)	/**< Peripheral lock mask for Primary audio path		*/
#define HLP_USB      (1UL<<HLP_USB_B)	/**< Peripheral lock mask for USB				*/
#define HLP_ETH      (1UL<<HLP_ETH_B)	/**< Peripheral lock mask for Xperip: Ethernet			*/
#define HLP_SPISLAVE (1UL<<HLP_SPISLAVE_B)/**< Peripheral lock mask for Xperip: Slave SPI		*/
#define HLP_NAND     (1UL<<HLP_NAND_B)	/**< Peripheral lock mask for Xperip: NAND Flash		*/
#define HLP_SD       (1UL<<HLP_SD_B)	/**< Peripheral lock mask for Xperip: SD Card			*/
#define HLP_RSEN     (1UL<<HLP_RSEN_B)	/**< Peripheral lock mask for Xperip: Reed-Solomon encoder	*/
#define HLP_RSDE     (1UL<<HLP_RSDE_B)	/**< Peripheral lock mask for Xperip: Reed-Solomon decoder	*/
#define HLP_SPI0     (1UL<<HLP_SPI0_B)	/**< Peripheral lock mask for SPI port 0			*/
#define HLP_SPI1     (1UL<<HLP_SPI1_B)	/**< Peripheral lock mask for SPI port 1			*/
#define HLP_MAC1     (1UL<<HLP_MAC1_B)	/**< Peripheral lock mask for FM decimation-by-6 DEC6 filter	*/
#define HLP_MAC0     (1UL<<HLP_MAC0_B)	/**< Peripheral lock mask for ADC 1 & 2				*/
#define HLP_GPIO0    (1UL<<HLP_GPIO0_B)	/**< Peripheral lock mask for GPIO port 0 including bit engines	*/
#define HLP_GPIO1    (1UL<<HLP_GPIO1_B)	/**< Peripheral lock mask for GPIO port 1 including bit engines	*/
#define HLP_GPIO2    (1UL<<HLP_GPIO2_B)	/**< Peripheral lock mask for GPIO port 2 including bit engines	*/
#define HLP_MAC2     (1UL<<HLP_MAC2_B)	/**< Peripheral lock mask for ADC 3 (mono ADC)			*/
#define HLP_I2S      (1UL<<HLP_I2S_B)	/**< Peripheral lock mask for I2S transmitter/receiver		*/
#define HLP_UART     (1UL<<HLP_UART_B)	/**< Peripheral lock mask for UART				*/
#define HLP_TIMER0   (1UL<<HLP_TIMER0_B)/**< Peripheral lock mask for Timer 0				*/
#define HLP_TIMER1   (1UL<<HLP_TIMER1_B)/**< Peripheral lock mask for Timer 1				*/
#define HLP_TIMER2   (1UL<<HLP_TIMER2_B)/**< Peripheral lock mask for Timer 2				*/
#define HLP_FM       (1UL<<HLP_FM_B)	/**< Peripheral lock mask for FM Receiver at full rate (192 kHz)*/
#define HLP_SRC      (1UL<<HLP_SRC_B)	/**< Peripheral lock mask for Filterless sample rate converter	*/
#define HLP_DAOSET   (1UL<<HLP_DAOSET_B)/**< Peripheral lock mask for Secondary audio path		*/
#define HLP_RTC      (1UL<<HLP_RTC_B)	/**< Peripheral lock mask for Real-time clock			*/
//#define HLP_STX      (1UL<<HLP_STX_B)	/**< Peripheral lock mask for S/PDIF transmit			*/
//#define HLP_SRX      (1UL<<HLP_SRX_B)	/**< Peripheral lock mask for S/PDIF receiver			*/
#define HLP_REGU     (1UL<<HLP_REGU_B)	/**< Peripheral lock mask for Power button			*/
#define HLP_PWM      (1UL<<HLP_PWM_B)	/**< Peripheral lock mask for Pulse-width modulator		*/
#define HLP_SAR      (1UL<<HLP_SAR_B)	/**< Peripheral lock mask for 10-bit SAR ADC			*/
#define HLP_PLL      (1UL<<HLP_PLL_B)	/**< Peripheral lock mask for PLL clock				*/
#define HLP_RFVCO    (1UL<<HLP_RFVCO_B)	/**< Peripheral lock mask for RF VCO clock			*/
#define HLP_XPERIP_IF (1UL<<HLP_XPERIP_IF_B)/**< Peripheral lock mask for Xperip DSP interface.		*/




#define TOTAL_HW_LOCKS (BUFFER_HW_LOCKS+IO_HW_LOCKS+PERIP_HW_LOCKS) /**< Total number of hardware locks */
#define HW_LOCK_GROUPS 3 /**< Amount of u_int32 variables needed to store lock bitmaps */


#ifndef ASM

/**
	Potential lock allocation or release error messages.
	Currently only leOk is returned.
*/
enum LockError {
  leOk = 0,
  leTimeout,
  leAlreadyAllocated,
  leAlreadyFree,
  leOtherFail
};


/**
	The hardware lock queue.
*/
struct HwLockQueue {
  struct MINNODE node; /**< Node structure needed to put this item in the queue list. */
  struct LIST queue; /**< Queue structure. The first item in the hwLocks.queue[n] will contain a list of queueing tasks. */
  struct TASK *taskP; /**< The owner task of this queue item. When this queue item is removed from the queue, taskP is signalled. */
};

/**
   This structure holds all hardware lock data, with the
   exception of the actual queues which are provided by
   the locking tasks.
*/
struct HwLocks {
  u_int32 lockBits[HW_LOCK_GROUPS]; /**< A bit is set for each lock */
  u_int32 queueBits[HW_LOCK_GROUPS]; /**< A bit is set for each lock that has a queue */ 
  struct HwLockQueue *queue[TOTAL_HW_LOCKS]; /**< Lock queues. For each existing queue there is a non-null pointer */
};

/**	Hardware locks. */
extern struct HwLocks hwLocks;

#if 1
/** Initializes hardware locks.
    Must be called one when the lock system is initiated. */
auto void InitHwLocks(void);
#else
#include <string.h>
/** Initializes hardware locks.
    Must be called one when the lock system is initiated. */
#define InitHwLocks() {memset(&hwLocks, 0, sizeof(hwLocks));}
#endif


/**
   Obtain hardware locks. If one or more resources are allocated,
   queue until they are freed. May NOT be called from interrupts.
   \param slio I/O lock mask.
   \param slp Peripheral lock mask.
   \param slb Buffer lock mask.
   \return leOk on success.
*/
auto enum LockError ObtainHwLocksBIP(register __reg_b u_int32 slb,
				     register __reg_c u_int32 slio,
				     register __reg_d u_int32 slp);

/**
   Attempt to obtain hardware locks. If one or more resources are allocated,
   don't allocate anything but return error code. May be called from interrupts.
   \param slio I/O lock mask.
   \param slp Peripheral lock mask.
   \param slb Buffer lock mask.
   \return leOk on success. On error bits 3:0 error type, bits 5:4 parameter
	number (0..2), bits 12:8 bit number (0..31).
*/
auto enum LockError AttemptHwLocksBIP(register __reg_b u_int32 slb,
				      register __reg_c u_int32 slio,
				      register __reg_d u_int32 slp);

/**
   Release hardware locks, and signal processes that are next
   in queue for each lock. May be called from interrupts, but may
   be very slow if there are queues for the resources allocated.
   \param slio I/O lock mask.
   \param slp Peripheral lock mask.
   \param slb Buffer lock mask.
   \return leOk on success.
*/
auto enum LockError ReleaseHwLocksBIP(register __reg_b u_int32 slb,
				      register __reg_c u_int32 slio,
				      register __reg_d u_int32 slp);


/*
  Get a HLIO_ mask for a GPIO pin.
  \param pin Pin number, as in GpioSetPin in vo_gpio.h:
	GPIO 0:11 = 0x0b, gpio 0:15 = 0x0f, gpio 1:0 = 0x10, etc.
	Must between 0 and 0x2e.
  \return Pin mask, or 0xFFFFFFFFU as error code.
  \example To get a lock mask and a subsequent lock to GPIO pins 0:11,
	0:15, and 1:0, do this.
	u_int32 mask = PinToGpioHwLockMask(0x0b);
	mask |= PinToGpioHwLockMask(0x0f);
	mask |= PinToGpioHwLockMask(0x10);
	ObtainHwLocksBIP(HLB_NONE, mask, HLP_NONE);
 */
auto u_int32 PinToGpioHwLockMask(register __d0 u_int16 pin);

/**
   Prints status of hardware lock, including potential queue.
   This function may not be compiled to the final binary.
   \param word The lock word (0..2).
   \param bit Lock bit (0..31).
*/
auto void PrintHwLock(s_int16 word, s_int16 bit);

s_int16 MSBSetU32(register __reg_d u_int32 n);

#endif /* !ASM */

/** Signals the task that a hardware lock has been freed */
#define SIGF_HW_LOCK_READY (1<<0)

#endif /* !VS1005G_HW_LOCKS */
