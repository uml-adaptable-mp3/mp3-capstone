#ifndef __VS1005G_H__
#define __VS1005G_H__

/*
  VS1005h definition file. (C) 2012-2017 VLSI Solution.
  Version: HH & Others 2017-11-21

  20171121 Updated relevant bits from vs1005g.h
  20140910 I2S rate is no longer dependent on 16/32-bit mode
  20140916 PWM_PULSELEN / PWM_FRAMELEN
  20140917 I2S register fix
  20141023 Added ROMIDs
*/

#include <vstypes.h>

#define obsoleted(a) OBSOLETE##a

#define ROM_ID_VS1005G 0xbbd7e455U
#define ROM_ID_VS1005H 0xb6f679e5U

#define ROM_EXEC_START 0x8000 //0x8000 // TODO: change to 0x8000 for ROM version

#define IROM_START ROM_EXEC_START
#define IROM_SIZE  0x8000
#define XROM_START ROM_EXEC_START
#define XROM_SIZE  0x8000
#define YROM_START ROM_EXEC_START
#define YROM_SIZE  0x7400

#define XRAM_START    0x0
#define XRAM_SIZE  0x8000
#define YRAM_START    0x0
#define YRAM_SIZE  0x8000
#define IRAM_START    0x0
#define IRAM_SIZE  0x8000

#define STACK_START 0x30 /* these may not be final.. */
#define STACK_SIZE  0x3d0 /* 0x100 is not quite enough! */
#define DEBUG_STACK (STACK_START+STACK_SIZE-32)  /* 78..98.. need 18 + tmp */
#define AUDIO_START  0x7000 /* 4096 */

/* For VS1005h */
/* Some I memory for kernelVectors (see kernel_abs.s) */
#define MEMPOOL_I_START 0x80
#define MEMPOOL_I_SIZE  (0x7fc6-MEMPOOL_I_START)
/* 32 from X memory reserved for rtosVariables */
#define MEMPOOL_X_START 0x900
#define MEMPOOL_X_SIZE  (XRAM_START+XRAM_SIZE-32-MEMPOOL_X_START)
/* 32 from X memory reserved for intVectors */
#define MEMPOOL_Y_START 0x900
#define MEMPOOL_Y_SIZE  (AUDIO_START-MEMPOOL_Y_START-32)


  // 
  // vs1005 peripherals (2012-04-02):
  // 
  // Fast peripherals from 0xFC00 to 0xFDFF
  //    INT_ADDR_c   : FC00-FC1F
  //    DSPIF_ADDR_c : FC20-FC3F
  //    SPI0_ADDR_c  : FC40-FC4F
  //    SPI1_ADDR_c  : FC50-FC5F
  //    ETHER_ADDR_c : FC60-FC65
  //    BUFFER_ADDR_c: FC66-FC6C
  //    REED_ADDR_c  : FC70-FC76
  //    NFLSH_ADDR_c : FC77-FC7A
  //    SD_ADDR_c    : FC7B-FC7F
  //    USB_ADDR_c   : FC80-FC9F
  //    GPIO0_ADDR_c : FCA0-FCBF	// 16-bit GPIO port 0
  //    GPIO1_ADDR_c : FCC0-FCDF	// 16-bit GPIO port 1
  //    GPIO2_ADDR_c : FCE0-FCFF	// 14-bit GPIO port 2
  //    SPDIF_ADDR_c : FD00-FD1F
  //    I2S Data Regs: FD20-FD3F

  // Slow peripherals from 0xFE00 to 0xFFFF
  //    UART_ADDR_c  : FE00-FE1F
  //    WDOG_ADDR_c  : FE20-FE3F
  //    FM_ADDR_c    : FE40-FE5F
  //    I2S_ADDR_c   : FE60-FE7F
  //    TIM_ADDR_c   : FE80-FE9F
  //    RTC_ADDR_c   : FEA0-FEBF
  //    DAC_ADDR_c   : FEC0-FEDF

  // 
  // vs1005 interrupts (2012-04-02):
  //
  //    INT_SAR     : 27;  -- 16-bit ADC
  //    INT_PWN     : 26;  -- pulse width modulator 
  //    INT_REGU_c  : 25;  -- power button
  //    INT_POSD_c  : 24;  -- pos detect
  //    INT_STX_c   : 23;  -- spdif tx 
  //    INT_SRX_c   : 22;  -- spdif rx
  //    INT_RDS_c   : 21;  -- FM RDS receiver
  //    INT_RTC_c   : 20;  -- RTC time alarm
  //    INT_DAOSET_c: 19;  -- DAC offset
  //    INT_SRC_c   : 18;  -- DAC sample rate converter
  //    INT_FM_c    : 17;  -- FM interrupt (typ. 192kHz)
  //    INT_TIM2_c  : 16;  -- Timer 2
  //
  //    INT_TIM1_c  : 15;  -- Timer 1
  //    INT_TIM0_c  : 14;  -- Timer 0
  //    INT_RX_c    : 13;  -- UART receive
  //    INT_TX_c    : 12;  -- UART transmit
  //    INT_I2S_c   : 11;  -- I2S transmitter/receiver
  //    INT_MAC2_c  : 10;  -- A/D 3 (mono AD) 192/96/48/24 kHz
  //    INT_GPIO2_c :  9;  -- GPIO port 2, fast clock
  //    INT_GPIO1_c :  8;  -- GPIO port 1, fast clock
  //    INT_GPIO0_c :  7;  -- GPIO port 0, fast clock
  //    INT_MAC0_c  :  6;  -- A/D 1/2 (stereo AD) 192/96/48/24kHz
  //    INT_MAC1_c  :  5;  -- FM decimation filter
  //    INT_SPI1_c  :  4;  -- SPI 1, fast clock
  //    INT_SPI0_c  :  3;  -- SPI 0, fast clock
  //    INT_XPERIP_c:  2;  -- Reed-Solomon, SD, Nand Flash, SPI, Ethernet,
  //                       --     fast clock
  //    INT_USB_c   :  1;  -- Full/High speed USB fast clock
  //    INT_DAC_c   :  0;  -- DAC, audio s-rate
  // 

#define INTV_SAR      27
#define INTV_PWM      26
#define INTV_REGU     25
#define INTV_POSD     24
#define INTV_STX      23 //SPDIF tx
#define INTV_SRX      22 //SPDIF rx
#define INTV_RDS      21
#define INTV_RTC      20
#define INTV_DAOSET   19
#define INTV_SRC      18
#define INTV_FM       17
#define INTV_TIMER2   16

#define INTV_TIMER1   15
#define INTV_TIMER0   14
#define INTV_UART_RX  13
#define INTV_UART_TX  12
#define INTV_I2S      11
#define INTV_MAC2     10 //mono AD
#define INTV_GPIO2    9
#define INTV_GPIO1    8
#define INTV_GPIO0    7
#define INTV_MAC0     6  //stereo AD
#define INTV_MAC1     5  //FM decimator
#define INTV_SPI1     4
#define INTV_SPI0     3
#define INTV_XPERIP   2
#define INTV_USB      1
#define INTV_DAC      0

#define INTF1_SAR      (1<<(INTV_SAR-16))
#define INTF1_PWM      (1<<(INTV_PWM-16))
#define INTF1_REGU     (1<<(INTV_REGU-16))
#define INTF1_POSD     (1<<(INTV_POSD-16))
#define INTF1_STX      (1<<(INTV_STX-16))
#define INTF1_SRX      (1<<(INTV_SRX-16))
#define INTF1_RDS      (1<<(INTV_RDS-16))
#define INTF1_RTC      (1<<(INTV_RTC-16))
#define INTF1_DAOSET   (1<<(INTV_DAOSET-16))
#define INTF1_SRC      (1<<(INTV_SRC-16))
#define INTF1_FM       (1<<(INTV_FM-16))
#define INTF1_TIMER2   (1<<(INTV_TIMER2-16))

#define INTF_TIMER1   (1<<INTV_TIMER1)
#define INTF_TIMER0   (1<<INTV_TIMER0)
#define INTF_UART_RX  (1<<INTV_UART_RX)
#define INTF_UART_TX  (1<<INTV_UART_TX)
#define INTF_I2S      (1<<INTV_I2S)
#define INTF_MAC2     (1<<INTV_MAC2)
#define INTF_GPIO2    (1<<INTV_GPIO2)
#define INTF_GPIO1    (1<<INTV_GPIO1)
#define INTF_GPIO0    (1<<INTV_GPIO0)
#define INTF_MAC0     (1<<INTV_MAC0)
#define INTF_MAC1     (1<<INTV_MAC1)
#define INTF_SPI1     (1<<INTV_SPI1)
#define INTF_SPI0     (1<<INTV_SPI0)
#define INTF_XPERIP   (1<<INTV_XPERIP)
#define INTF_USB      (1<<INTV_USB)
#define INTF_DAC      (1<<INTV_DAC)


// Symbols obsoleted with VSOS 0.22 2012-10-18
#define INT_ENABLEH0 PleaseReplaceThisSymbolWithINT_ENABLE0_HP
#define INT_ENABLEL0 PleaseReplaceThisSymbolWithINT_ENABLE0_LP
#define INT_ENABLEH1 PleaseReplaceThisSymbolWithINT_ENABLE1_HP
#define INT_ENABLEL1 PleaseReplaceThisSymbolWithINT_ENABLE1_LP

#define INT_ENABLE0_HP 	0xFC02	/**< Sources  0..15, high priority bits */
#define INT_ENABLE0_LP 	0xFC00	/**< Sources  0..15, low priority bits */
#define INT_ENABLE1_HP 	0xFC03	/**< Sources 16..31, high priority bits */
#define INT_ENABLE1_LP 	0xFC01	/**< Sources 16..31, low priority bits */
#define INT_ORIGIN0  	0xFC04
#define INT_ORIGIN1  	0xFC05
#define INT_VECTOR  	0xFC06
#define INT_ENCOUNT 	0xFC07
#define INT_GLOB_DIS 	0xFC08
#define INT_GLOB_ENA	0xFC09




/* General purpose software registers */

#define SW_REG0          0xFC20
#define SW_REG1          0xFC21
#define SW_REG2          0xFC22
#define SW_REG3          0xFC23




/* Peripheral I/O control */
/* Bits:
   0: GPIO mode
   1: Peripheral mode
*/

#define GPIO0_MODE       0xFC30
#define GPIO1_MODE       0xFC31
#define GPIO2_MODE       0xFC32




/* PLL clock control */

#define CLK_CF           0xFC33

#define CLK_CF_EXTOFF_B         15
#define CLK_CF_NFOFF_B          14
#define CLK_CF_USBOFF_B         13
#define CLK_CF_RTCSLP_B         12
#define CLK_CF_LCKST_B          11
#define CLK_CF_GDIV256_B        10
#define CLK_CF_GDIV2_B           9
#define CLK_CF_LCKCHK_B          8
#define CLK_CF_VCOOUT_B          7
#define CLK_CF_USBCLK_B          6
#define CLK_CF_FORCEPLL_B        5
#define CLK_CF_DIVI_B            4
#define CLK_CF_MULT3_B           3
#define CLK_CF_MULT2_B           2
#define CLK_CF_MULT1_B	         1
#define CLK_CF_MULT0_B           0

#define CLK_CF_EXTOFF    (1<<CLK_CF_EXTOFF_B)
#define CLK_CF_NFOFF     (1<<CLK_CF_NFOFF_B)
#define CLK_CF_USBOFF    (1<<CLK_CF_USBOFF_B)
#define CLK_CF_RTCSLP    (1<<CLK_CF_RTCSLP_B)
#define CLK_CF_LCKST     (1<<CLK_CF_LCKST_B)
#define CLK_CF_GDIV256   (1<<CLK_CF_GDIV256_B)
#define CLK_CF_GDIV2     (1<<CLK_CF_GDIV2_B)
#define CLK_CF_LCKCHK    (1<<CLK_CF_LCKCHK_B)
#define CLK_CF_VCOOUT    (1<<CLK_CF_VCOOUT_B)
#define CLK_CF_USBCLK    (1<<CLK_CF_USBCLK_B)
#define CLK_CF_FORCEPLL  (1<<CLK_CF_FORCEPLL_B)
#define CLK_CF_DIVI      (1<<CLK_CF_DIVI_B)
#define CLK_CF_MULT3     (1<<CLK_CF_MULT3_B)
#define CLK_CF_MULT2     (1<<CLK_CF_MULT2_B)
#define CLK_CF_MULT1     (1<<CLK_CF_MULT1_B)
#define CLK_CF_MULT0     (1<<CLK_CF_MULT0_B)

#define CLK_CF_MULT_MASK (15*CLK_CF_MULT0)





/* Power down control */

#define SYSTEMPD         0xFC3E
#define SYSTEMPD_REG_BITS 13

#define SYSTEMPD_SFENA_B       12 // Enables SPI for internal flash
#define SYSTEMPD_FRAM_B        11
#define SYSTEMPD_YRAM1_B       10
#define SYSTEMPD_YRAM0_B        9
#define SYSTEMPD_XRAM1_B        8
#define SYSTEMPD_XRAM0_B        7
#define SYSTEMPD_IRAM1_B        6
#define SYSTEMPD_RTCCLK_B       5
#define SYSTEMPD_XTALOFF_B      4
#define SYSTEMPD_ROMDELAY_B     2
#define SYSTEMPD_RAMDELAY_B     0
#define SYSTEMPD_ROMDELAY_HI_B  3
#define SYSTEMPD_ROMDELAY_LO_B  2
#define SYSTEMPD_RAMDELAY_HI_B  1
#define SYSTEMPD_RAMDELAY_LO_B  0

#define SYSTEMPD_SFENA       (1<<12) // Enables SPI for internal flash
#define SYSTEMPD_FRAM        (1<<11)
#define SYSTEMPD_YRAM1       (1<<10)
#define SYSTEMPD_YRAM0       (1<< 9)
#define SYSTEMPD_XRAM1       (1<< 8)
#define SYSTEMPD_XRAM0       (1<< 7)
#define SYSTEMPD_IRAM1       (1<< 6)
#define SYSTEMPD_RTCCLK      (1<< 5)
#define SYSTEMPD_XTALOFF     (1<< 4)
#define SYSTEMPD_ROMDELAY    (1<< 2)
#define SYSTEMPD_ROMDELAY_MASK (3<<SYSTEMPD_ROMDELAY_B)
#define SYSTEMPD_RAMDELAY    (1<< 0)
#define SYSTEMPD_RAMDELAY_MASK (3<<SYSTEMPD_RAMDELAY_B)
#define SYSTEMPD_ROMDELAY_HI (1<< 3)
#define SYSTEMPD_ROMDELAY_LO (1<< 2)
#define SYSTEMPD_RAMDELAY_HI (1<< 1)
#define SYSTEMPD_RAMDELAY_LO (1<< 0)


/* System miscellaneous: divider control register */
#define SYSMISC 0xFC3F

#define SYSMISC_CLK_INVPOL_B	3
#define SYSMISC_CLK_DIV_B	0

#define SYSMISC_CLK_INVPOL	(1<<SYSMISC_CLK_INVPOL_B)
#define SYSMISC_CLK_DIV		(1<<SYSMISC_CLK_DIV_B)

#define SYSMISC_CLK_DIV_MASK	(7<<SYSMISC_CLK_DIV_B)



/* Analog control registers */

#define ANA_CF0          0xfecc
#define ANA_CF1          0xfecb
#define ANA_CF2          0xfed2
#define ANA_CF3          0xfed3

#define ANA_CF0_SETTOZERO2_B 11
#define ANA_CF0_M1LIN_B    10
#define ANA_CF0_M2LIN_B     9
#define ANA_CF0_SETTOZERO1_B  6
#define ANA_CF0_M2MIC_B     5
#define ANA_CF0_LCKST_B     4 //2GHz VCO lock status
#define ANA_CF0_LCKCHK_B    3 //2GHz VCO lock test
#define ANA_CF0_M1MIC_B     2
#define ANA_CF0_M2FM_B      1
#define ANA_CF0_M1FM_B      0

#define ANA_CF0_SETTOZERO2   (1<<11)
#define ANA_CF0_M1LIN   (1<<10)
#define ANA_CF0_M2LIN    (1<<9)
#define ANA_CF0_SETTOZERO1   (1<<6)
#define ANA_CF0_M2MIC    (1<<5)
#define ANA_CF0_LCKST    (1<<4) //2GHz VCO lock status
#define ANA_CF0_LCKCHK   (1<<3) //2GHz VCO lock test
#define ANA_CF0_M1MIC    (1<<2)
#define ANA_CF0_M2FM     (1<<1)
#define ANA_CF0_M1FM     (1<<0)

#define ANA_CF1_SETTOZERO_B 15
#define ANA_CF1_VHMON_B    14
#define ANA_CF1_PWRBTN_B   13
#define ANA_CF1_BTNDIS_B   12  //button reset disable
#define ANA_CF1_XTMODE_B   11
#define ANA_CF1_DBG_B      10
#define ANA_CF1_XTDIV_B     9 //input divider for 24MHz osc
#define ANA_CF1_SAR_ENA_B   8 //SAR power
#define ANA_CF1_PWM_MODE1_B 7
#define ANA_CF1_DA_ENA_B    6 //DAC enable
#define ANA_CF1_UNUSED_B    5
#define ANA_CF1_ADDA_LOOP_B 4
#define ANA_CF1_DRV_ENA_B   3 //DAC driver enable
#define ANA_CF1_PWM_MODE0_B 2
#define ANA_CF1_DAGAIN_B    obsoleted("ANA_CF1_DAGAIN_B")

#define ANA_CF1_SETTOZERO (1<<15)
#define ANA_CF1_VHMON     (1<<14)
#define ANA_CF1_PWRBTN    (1<<13)
#define ANA_CF1_BTNDIS    (1<<12)  //button reset disable
#define ANA_CF1_XTMODE    (1<<11)
#define ANA_CF1_DBG       (1<<10)
#define ANA_CF1_XTDIV     (1<<9) //input divider for 24MHz osc
#define ANA_CF1_SAR_ENA   (1<<8) //SAR power
#define ANA_CF1_PWM_MODE1 (1<<7)
#define ANA_CF1_DA_ENA    (1<<6) //DAC enable
#define ANA_CF1_UNUSED    (1<<5)
#define ANA_CF1_ADDA_LOOP (1<<4)
#define ANA_CF1_DRV_ENA   (1<<3) //DAC driver enable
#define ANA_CF1_PWM_MODE0 (1<<2)
#define ANA_CF1_DAGAIN    obsoleted("ANA_CF1_DAGAIN")

#define ANA_CF1_PWM_MODE_MASK       (ANA_CF1_PWM_MODE1 | ANA_CF1_PWM_MODE0)
#define ANA_CF1_PWM_MODE_USBDISCONN (ANA_CF1_PWM_MODE1 | ANA_CF1_PWM_MODE0)
#define ANA_CF1_PWM_MODE_USBSQUELCH (ANA_CF1_PWM_MODE1                    )
#define ANA_CF1_PWM_MODE_USBDIFF    (                    ANA_CF1_PWM_MODE0)
#define ANA_CF1_PWM_MODE_PWM        (                                    0)

#define ANA_CF1_DAGAIN_MASK    obsoleted("ANA_CF1_DAGAIN_MASK")
#define ANA_CF1_DAGAIN_0DB     obsoleted("ANA_CF1_DAGAIN_0DB")
#define ANA_CF1_DAGAIN_M6DB    obsoleted("ANA_CF1_DATAIN_M6DB")
//#define ANA_CF1_DAGAIN_M2DB_DO_NOT_USE 2
#define ANA_CF1_DAGAIN_M12DB   obsoleted("ANA_CF1_DAGAIN_M12DB")

#define ANA_CF2_RTC_XTALO_ST_B 15
#define ANA_CF2_RTC_SNFD_ST_B  14
#define ANA_CF2_TSTE_B      13
#define ANA_CF2_VCMST_B     12 //CBUF short circuit monitor
#define ANA_CF2_VCMDIS_B    11
#define ANA_CF2_UTM_ENA_B   10 //High-speed USB UTM enable
#define ANA_CF2_LNA_ENA_B    9 //Low-noise amplifier powerdown
#define ANA_CF2_2G_ENA_B     8 //2GHz VCO power
#define ANA_CF2_AMP1_ENA_B   7 //Mic amp 1 powerdown
#define ANA_CF2_AMP2_ENA_B   6 //Mic amp 2 powerdown
#define ANA_CF2_USBLP_B      5
#define ANA_CF2_HIGH_REF_B   4 //Reference select 1.6V / 1.2V
#define ANA_CF2_REF_ENA_B    3 //Reference powerdown
#define ANA_CF2_M3_ENA_B     2 //Modulator 3 enable
#define ANA_CF2_M2_ENA_B     1 //Modulator 2 enable
#define ANA_CF2_M1_ENA_B     0 //Modulator 1 enable

#define ANA_CF2_RTC_XTALO_ST (1<<15)
#define ANA_CF2_RTC_SNFD_ST  (1<<14)
#define ANA_CF2_TSTE      (1<<13)
#define ANA_CF2_VCMST     (1<<12) //CBUF short circuit monitor
#define ANA_CF2_VCMDIS    (1<<11)
#define ANA_CF2_UTM_ENA   (1<<10) //High-speed USB UTM enable
#define ANA_CF2_LNA_ENA    (1<<9) //Low-noise amplifier powerdown
#define ANA_CF2_2G_ENA     (1<<8) //2GHz VCO power
#define ANA_CF2_AMP1_ENA   (1<<7) //Mic amp 1 powerdown
#define ANA_CF2_AMP2_ENA   (1<<6) //Mic amp 2 powerdown
#define ANA_CF2_USBLP      (1<<5)
#define ANA_CF2_HIGH_REF   (1<<4) //Reference select 1.6V / 1.2V
#define ANA_CF2_REF_ENA    (1<<3) //Reference powerdown
#define ANA_CF2_M3_ENA     (1<<2) //Modulator 3 enable
#define ANA_CF2_M2_ENA     (1<<1) //Modulator 2 enable
#define ANA_CF2_M1_ENA     (1<<0) //Modulator 1 enable

#define ANA_CF3_480_ENA_B   15
#define ANA_CF3_UTMBIAS_B   14
#define ANA_CF3_FMDIV_B     12
#define ANA_CF3_DIV_B       10
#define ANA_CF3_GAIN2_B      7
#define ANA_CF3_GAIN1_B      4
#define ANA_CF3_2GCNTR_B     0

#define ANA_CF3_480_ENA   (1<<15)
#define ANA_CF3_UTMBIAS   (1<<14) //use '0'
#define ANA_CF3_FMDIV24   (3<<12) //:6:4
#define ANA_CF3_FMDIV20   (2<<12) //:5:4
#define ANA_CF3_FMDIV16   (0<<12) //:4:4
#define ANA_CF3_FMDIV     (1<<12)
#define ANA_CF3_DIV       (1<<10)
#define ANA_CF3_GAIN2      (1<<7)
#define ANA_CF3_GAIN1      (1<<4)
#define ANA_CF3_2GCNTR     (1<<0)

#define ANA_CF3_FMDIV_MASK (3<<12)
#define ANA_CF3_DIV_MASK (3<<12)

#define ANA_CF3_GAIN20DB 1	// Can be put into ANA_CF3_GAIN2 or GAIN1
#define ANA_CF3_GAIN17DB 2	// Can be put into ANA_CF3_GAIN2 or GAIN1
#define ANA_CF3_GAIN14DB 4	// Can be put into ANA_CF3_GAIN2 or GAIN1
#define ANA_CF3_GAIN11DB 0	// Can be put into ANA_CF3_GAIN2 or GAIN1

#define ANA_CF3_GAIN1MASK (7<<ANA_CF3_GAIN1_B)
#define ANA_CF3_GAIN1_20DB (1<<ANA_CF3_GAIN1_B)
#define ANA_CF3_GAIN1_17DB (2<<ANA_CF3_GAIN1_B)
#define ANA_CF3_GAIN1_14DB (4<<ANA_CF3_GAIN1_B)
#define ANA_CF3_GAIN1_11DB (0<<ANA_CF3_GAIN1_B)

#define ANA_CF3_GAIN2MASK (7<<ANA_CF3_GAIN2_B)
#define ANA_CF3_GAIN2_20DB (1<<ANA_CF3_GAIN2_B)
#define ANA_CF3_GAIN2_17DB (2<<ANA_CF3_GAIN2_B)
#define ANA_CF3_GAIN2_14DB (4<<ANA_CF3_GAIN2_B)
#define ANA_CF3_GAIN2_11DB (0<<ANA_CF3_GAIN2_B)

#define ANA_CF3_2GCNTR_MASK 15




/* Regulator and peripheral clock control registers */

#define REGU_CF 0xFECE
#define REGU_CF_REG_BITS 12
#define REGU_VOLT 0xFED0

#define REGU_VOLT_AVDD_B 10
#define REGU_VOLT_IOVDD_B 5
#define REGU_VOLT_CVDD_B 0

#define REGU_VOLT_AVDD  (1<<REGU_VOLT_AVDD_B)
#define REGU_VOLT_IOVDD (1<<REGU_VOLT_IOVDD_B)
#define REGU_VOLT_CVDD  (1<<REGU_VOLT_CVDD_B)

#define REGU_CF_SNFVOLT_B 7
#define REGU_CF_SNFOFF_B  6
#define REGU_CF_ADOFF_B   5
#define REGU_CF_FMOFF_B   4
#define REGU_CF_REGCK_B   3
#define REGU_CF_AOFF_B    2
#define REGU_CF_IOOFF_B   1
#define REGU_CF_COFF_B    0

#define REGU_CF_SNFVOLT (1<<7)
#define REGU_CF_SNFOFF  (1<<6)
#define REGU_CF_ADOFF   (1<<5)
#define REGU_CF_FMOFF   (1<<4)
#define REGU_CF_REGCK   (1<<3)
#define REGU_CF_AOFF    (1<<2)
#define REGU_CF_IOOFF   (1<<1)
#define REGU_CF_COFF    (1<<0)

#define IO_1V8  0
#define IO_2V7 15
#define IO_3V0 20
#define IO_3V3 25
#define IO_3V6 30

#define FLASH_1V8 IO_1V8
#define FLASH_2V7 IO_2V7
#define FLASH_3V0 IO_3V0
#define FLASH_3V3 IO_3V3
#define FLASH_3V6 IO_3V6

#define ANA_2V5  0
#define ANA_2V7  6
#define ANA_3V0 13
#define ANA_3V3 20
#define ANA_3V6 28

#define CORE_1V4   5
#define CORE_1V5   9
#define CORE_1V6  12
#define CORE_1V7  15
#define CORE_1V75 17
#define CORE_1V8  19
#define CORE_1V85 20
#define CORE_1V9  22
#define CORE_1V95 24
#define CORE_2V0  25
#define CORE_2V1  29




/* Analog trimming resistors (VLSI Solution internal use only) */

#define UTM_TRIMRES     0xFED1




/* 24-bit digital to analog converter (DAC) */

#define DAC_SRCL         0xFC34    // 15:0 = SRC bits 15: 0
#define DAC_SRCH         0xFC35    //  3:0 = SRC bits 19:16
#define DAC_LEFT_LSB     0xFC36    // 15:8 = DAC bits  7: 0
#define DAC_LEFT         0xFC37    // 15:0 = DAC bits 24: 8
#define DAC_RIGHT_LSB    0xFC38    // 15:8 = DAC bits  7: 0
#define DAC_RIGHT        0xFC39    // 15:0 = DAC bits 24: 8



/* Filter RAM test modes and DAC/ADC control registers,
   (VLSI Solution internal use only) */

#define DAC_MTEST        0xFECF
#define DAC_MTEST_REG_BITS 13

#define DAC_MTEST_3MUAD_B 12
#define DAC_MTEST_96K_B   11
#define DAC_MTEST_WR_B    10
#define DAC_MTEST_RD_B     9
#define DAC_MTEST_L_B      8
#define DAC_MTEST_R_B      7
#define DAC_MTEST_ADDR_B   0

#define DAC_MTEST_3MUAD (1<<12)
#define DAC_MTEST_96K   (1<<11)
#define DAC_MTEST_WR    (1<<10)
#define DAC_MTEST_RD    (1<< 9)
#define DAC_MTEST_L     (1<< 8)
#define DAC_MTEST_R     (1<< 7)
#define DAC_MTEST_ADDR  (1<< 0)




/* DAC volume control */

#define DAC_VOL          0xFEC0

#define DAC_VOL_LADD_B 12   // Left channel in 0.5 dB steps
#define DAC_VOL_LSFT_B  8   // Left channel in -6 dB steps
#define DAC_VOL_RADD_B  4   // Right channel in 0.5 dB steps
#define DAC_VOL_RSFT_B  0   // Right channel in -6 dB steps




/* DAC offset registers */

#define DAOSET_CF        0xFEC1
#define DAOSET_CF_REG_BITS 15
#define DAOSET_LEFT_LSB	 0xFEC2
#define DAOSET_LEFT   	 0xFEC3
#define DAOSET_RIGHT_LSB 0xFEC4
#define DAOSET_RIGHT     0xFEC5

#define DAOSET_CF_URUN_B 14
#define DAOSET_CF_FULL_B 13
#define DAOSET_CF_ENA_B  12
#define DAOSET_CF_FS_B    0	// 12 bits

#define DAOSET_CF_URUN (1<<14)
#define DAOSET_CF_FULL (1<<13)
#define DAOSET_CF_ENA  (1<<12)
#define DAOSET_CF_FS   (1<< 0)	// 12 bits




/* Sample rate converter (SRC) registers */

#define SRC_CF   	 0xFEC6
#define SRC_LEFT_LSB   	 0xFEC7
#define SRC_LEFT   	 0xFEC8
#define SRC_RIGHT_LSB  	 0xFEC9
#define SRC_RIGHT   	 0xFECA

#define SRC_CF_ORUN_B  15
#define SRC_CF_RFULL_B 14
#define SRC_CF_LFULL_B 13
#define SRC_CF_ENA_B   12
#define SRC_CF_FS_B     0	// 12 bits

#define SRC_CF_ORUN  (1<<15)
#define SRC_CF_RFULL (1<<14)
#define SRC_CF_LFULL (1<<13)
#define SRC_CF_ENA   (1<<12)
#define SRC_CF_FS    (1<< 0)	// 12 bits




/* Asynchronous audio read port (VLSI Solution internal use only) */

#define FAUDIO_SEL       0xFC3B
#define FAUDIO_L         0xFC3C
#define FAUDIO_R         0xFC3D

#define FAUDIO_SEL_DEC6_L   15
#define FAUDIO_SEL_DEC6_H   13
#define FAUDIO_SEL_FM        9
#define FAUDIO_SEL_AD32_LO   7
#define FAUDIO_SEL_AD32_HI   5
#define FAUDIO_SEL_AD12_LO   3
#define FAUDIO_SEL_AD12_HI   1
#define FAUDIO_SEL_SAR       8
#define FAUDIO_SEL_SRC_LO    2
#define FAUDIO_SEL_SRC_HI    0




/* SPI peripherals */

#define SPI0_CF         0xFC40
#define SPI0_CF_REG_BITS    12
#define SPI0_CLKCF      0xFC41
#define SPI0_CLKCF_REG_BITS 10
#define SPI0_STATUS     0xFC42
#define SPI0_STATUS_BITS     8
#define SPI0_DATA       0xFC43
#define SPI0_FSYNC      0xFC44
#define SPI0_DEFAULT    0xFC45

#define SPI1_CF         0xFC50
#define SPI1_CF_REG_BITS    13
#define SPI1_CLKCF      0xFC51
#define SPI1_CLKCF_REG_BITS 10
#define SPI1_STATUS     0xFC52
#define SPI1_STATUS_BITS     8
#define SPI1_DATA       0xFC53
#define SPI1_FSYNC      0xFC54
#define SPI1_DEFAULT    0xFC55

#define SPI_CF_EARLYINT_B    12    // Interrupt when !SPI_ST_TXFULL
#define SPI_CF_SRESET_B      11    // software reset
#define SPI_CF_RXFIFOMODE_B  10    // int mode
#define SPI_CF_RXFIFO_ENA_B   9
#define SPI_CF_TXFIFO_ENA_B   8
#define SPI_CF_XCSMODE_B      6
#define SPI_CF_MASTER_B       5
#define SPI_CF_DLEN_B         1
#define SPI_CF_FSIDLE_B       0

#define SPI_CF_EARLYINT    (1<<12)    // Interrupt when !SPI_ST_TXFULL
#define SPI_CF_SRESET      (1<<11)    // software reset
#define SPI_CF_RXFIFOMODE  (1<<10)    // int mode
#define SPI_CF_RXFIFO_ENA   (1<<9)
#define SPI_CF_TXFIFO_ENA   (1<<8)
#define SPI_CF_XCSMODE      (1<<6)
#define SPI_CF_INTXCS       (0<<6)
#define SPI_CF_FALLXCS      (2<<6)
#define SPI_CF_RISEXCS      (3<<6)
#define SPI_CF_MASTER       (1<<5)
#define SPI_CF_SLAVE        (0<<5)
#define SPI_CF_DLEN         (1<<1)
#define SPI_CF_DLEN8        (7<<1)
#define SPI_CF_DLEN16      (15<<1)
#define SPI_CF_FSIDLE       (1<<0)
#define SPI_CF_FSIDLE1      (1<<0)
#define SPI_CF_FSIDLE0      (0<<0)

#define SPI_CC_CLKDIV_B       2
#define SPI_CC_INV_CLKPOL_B   1
#define SPI_CC_INV_CLKPHASE_B 0

#define SPI_CC_CLKDIV       (1<<2)
#define SPI_CC_CLKDIV_MASK (255<<SPI_CC_CLKDIV_B)
#define SPI_CC_INV_CLKPOL   (1<<1)
#define SPI_CC_INV_CLKPHASE (1<<0)

#define SPI_ST_RXFIFOFULL_B   7
#define SPI_ST_TXFIFOFULL_B   6
#define SPI_ST_BREAK_B        5
#define SPI_ST_RXORUN_B       4
#define SPI_ST_RXFULL_B       3
#define SPI_ST_TXFULL_B       2
#define SPI_ST_TXRUNNING_B    1
#define SPI_ST_TXURUN_B       0

#define SPI_ST_RXFIFOFULL   (1<<7)
#define SPI_ST_TXFIFOFULL   (1<<6)
#define SPI_ST_BREAK        (1<<5)
#define SPI_ST_RXORUN       (1<<4)
#define SPI_ST_RXFULL       (1<<3)
#define SPI_ST_TXFULL       (1<<2)
#define SPI_ST_TXRUNNING    (1<<1)
#define SPI_ST_TXURUN       (1<<0)




/* Common data interfaces */

#define ECC_LP_LOW      0xFC66 //line parity bits
#define ECC_CP_LP_HIGH  0xFC67 //column parity bits, line parity msb, ecc cntr

#define XP_CF    0xFC68
#define XP_ADDR  0xFC69
// NOTE! WARNING 120402! SOME EARLIER .H FILES called XP_ODATA XP_DSP_IDATA!!!
#define XP_ODATA 0xFC6A // data to fifo.
// NOTE! WARNING 120402! SOME EARLIER .H FILES called XP_IDATA XP_DSP_ODATA!!!
#define XP_IDATA 0xFC6B /* data from fifo, needs 2 dummy reads. */
#define XP_ST    0xFC6C
#define XP_ST_REG_BITS 15

#define XP_CF_ODAT_B       12  // RS_ODATA mux control
#define XP_CF_ECC_RESET_B   9  // Reset 1-bit ECC
#define XP_CF_ECC_ENA_B     8  // Enable 1-bit ECC calculation
#define XP_CF_WRBUF_ENA_B   1  // Enable data buffer write
#define XP_CF_RDBUF_ENA_B   0  // Enable data buffer read

#define XP_CF_ODAT       (1<<12) // RS_ODATA mux control
#define XP_CF_ECC_RESET  (1<< 9) // Reset 1-bit ECC
#define XP_CF_ECC_ENA    (1<< 8) // Enable 1-bit ECC calculation
#define XP_CF_WRBUF_ENA  (1<< 1) // Enable data buffer write
#define XP_CF_RDBUF_ENA  (1<< 0) // Enable data buffer read

#define XP_ST_INT_ENA_B         14 /* global int enable */
#define XP_ST_ETXRB_HALF2_INT_B 13 /* Ethernet tx ring buffer last */
#define XP_ST_ETXRB_HALF1_INT_B 12 /* Ethernet tx ring buffer middle */
#define XP_ST_ERXRB_HALF2_INT_B 11 /* Ethernet rx ring buffer last */
#define XP_ST_ERXRB_HALF1_INT_B 10 /* Ethernet rx ring buffer middle */
#define XP_ST_SPIERR_INT_B       9
#define XP_ST_RSEC_RDY_INT_B     8
#define XP_ST_RSDEC_RDY_INT_B    7
#define XP_ST_RSENC_RDY_INT_B    6
#define XP_ST_SD_INT_B           5
#define XP_ST_NF_INT_B           4
#define XP_ST_SPI_STOP_INT_B     3
#define XP_ST_SPI_START_INT_B    2
#define XP_ST_ETHRX_INT_B        1 /* Ethernet rx (new packet) */
#define XP_ST_ETHTX_INT_B        0 /* Ethernet tx (transmitter done) */

#define XP_ST_INT_ENA         (1<<14) /* global int enable */
#define XP_ST_ETXRB_HALF2_INT (1<<13) /* Ethernet tx ring buffer last */
#define XP_ST_ETXRB_HALF1_INT (1<<12) /* Ethernet tx ring buffer middle */
#define XP_ST_ERXRB_HALF2_INT (1<<11) /* Ethernet rx ring buffer last */
#define XP_ST_ERXRB_HALF1_INT (1<<10) /* Ethernet rx ring buffer middle */
#define XP_ST_SPIERR_INT      (1<< 9)
#define XP_ST_RSEC_RDY_INT    (1<< 8)
#define XP_ST_RSDEC_RDY_INT   (1<< 7)
#define XP_ST_RSENC_RDY_INT   (1<< 6)
#define XP_ST_SD_INT          (1<< 5)
#define XP_ST_NF_INT          (1<< 4)
#define XP_ST_SPI_STOP_INT    (1<< 3)
#define XP_ST_SPI_START_INT   (1<< 2)
#define XP_ST_ETHRX_INT       (1<< 1) /* Ethernet rx (new packet) */
#define XP_ST_ETHTX_INT       (1<< 0) /* Ethernet tx (transmitter done) */
#define XP_ST_ALL_INTS (XP_ST_ETXRB_HALF2_INT |XP_ST_ETXRB_HALF1_INT | XP_ST_ERXRB_HALF2_INT |XP_ST_ERXRB_HALF1_INT | XP_ST_SPIERR_INT | XP_ST_RSEC_RDY_INT | XP_ST_RSDEC_RDY_INT | XP_ST_RSENC_RDY_INT | XP_ST_SD_INT | XP_ST_NF_INT | XP_ST_SPI_STOP_INT | XP_ST_SPI_START_INT | XP_ST_ETHRX_INT | XP_ST_ETHTX_INT)




/* Ethernet controller */

#define ETH_TXLEN   0xFC60
#define ETH_TXPTR   0xFC61
#define ETH_RXLEN   0xFC62
#define ETH_RXPTR   0xFC63
#define ETH_RBUF    0xFC64
#define ETH_RBUF_REG_BITS 10
#define ETH_RXADDR  0xFC65

#define ETH_TXLEN_META_B  15
#define ETH_TXLEN_RX_BE_B 14
#define ETH_TXLEN_TX_BE_B 13
#define ETH_TXLEN_LEN_B    0

#define ETH_TXLEN_META  (1<<15)
#define ETH_TXLEN_RX_BE (1<<14)
#define ETH_TXLEN_TX_BE (1<<13)
#define ETH_TXLEN_LEN   (1<< 0)

#define ETH_TXPTR_SPI_TX_ENA_B 15
#define ETH_TXPTR_SPI_RX_ENA_B 14
#define ETH_TXPTR_BUSY_B       13
#define ETH_TXPTR_START_B      12
#define ETH_TXPTR_PTR_B         0

#define ETH_TXPTR_SPI_TX_ENA (1<<15)
#define ETH_TXPTR_SPI_RX_ENA (1<<14)
#define ETH_TXPTR_BUSY       (1<<13)
#define ETH_TXPTR_START      (1<<12)
#define ETH_TXPTR_PTR        (1<< 0)

#define ETH_RXLEN_SPIMODE_B   15
#define ETH_RXLEN_SPIINVCLK_B 14
#define ETH_RXLEN_LEN_B        0

#define ETH_RXLEN_SPIMODE   (1<<15)
#define ETH_RXLEN_SPIINVCLK (1<<14)
#define ETH_RXLEN_LEN       (1<< 0)

#define ETH_RXPTR_CRCOK_B  15
#define ETH_RXPTR_NEWPKT_B 14
#define ETH_RXPTR_BUSY_B   13
#define ETH_RXPTR_ENA_B    12
#define ETH_RXPTR_PTR_B     0

#define ETH_RXPTR_CRCOK  (1<<15)
#define ETH_RXPTR_NEWPKT (1<<14)
#define ETH_RXPTR_BUSY   (1<<13)
#define ETH_RXPTR_ENA    (1<<12)
#define ETH_RXPTR_PTR    (1<< 0)

#define ETH_RBUF_CLKCF_B 8
#define ETH_RBUF_TXENA_B 7
#define ETH_RBUF_TXCF_B  4
#define ETH_RBUF_RXENA_B 3
#define ETH_RBUF_RXCF_B  0

#define ETH_RBUF_CLKCF		(1<<8)
#define ETH_RBUF_CLKCF_60MHZ	(0<<8)
#define ETH_RBUF_CLKCF_80MHZ	(1<<8)
#define ETH_RBUF_CLKCF_100MHZ	(2<<8)
#define ETH_RBUF_CLKCF_120MHZ	(3<<8)
#define ETH_RBUF_TXENA		(1<<7)
#define ETH_RBUF_TXCF		(1<<4)
#define ETH_RBUF_TXCF_64W	(0<<4)
#define ETH_RBUF_TXCF_128W	(1<<4)
#define ETH_RBUF_TXCF_256W	(2<<4)
#define ETH_RBUF_TXCF_512W	(3<<4)
#define ETH_RBUF_TXCF_1024W	(4<<4)
#define ETH_RBUF_RXENA		(1<<3)
#define ETH_RBUF_RXCF		(1<<0)
#define ETH_RBUF_RXCF_64W	(0<<0)
#define ETH_RBUF_RXCF_128W	(1<<0)
#define ETH_RBUF_RXCF_256W	(2<<0)
#define ETH_RBUF_RXCF_512W	(3<<0)
#define ETH_RBUF_RXCF_1024W	(4<<0)




/* Reed-Solomon codec */

#define RS_ST      0xFC70
#define RS_ST_REG_BITS 13
#define RS_CF      0xFC71
#define RS_CF_REG_BITS 14
#define RS_EPTR    0xFC72
#define RS_ELEN    0xFC73
#define RS_DPTR    0xFC74
#define RS_DLEN    0xFC75
#define RS_DATA    0xFC76

#define RS_ST_DNERR_B  8
#define RS_ST_DFFAIL_B 6
#define RS_ST_DFRDY2_B 5
#define RS_ST_DFBUSY_B 4
#define RS_ST_DFRDY1_B 3
#define RS_ST_DFAIL_B  2
#define RS_ST_DERR_B   1
#define RS_ST_DOK_B    0

#define RS_ST_DNERR  (1<<8)
#define RS_ST_DFFAIL (1<<6)
#define RS_ST_DFRDY2 (1<<5)
#define RS_ST_DFBUSY (1<<4)
#define RS_ST_DFRDY1 (1<<3)
#define RS_ST_DFAIL  (1<<2)
#define RS_ST_DERR   (1<<1)
#define RS_ST_DOK    (1<<0)

#define RS_CF_DNF_B   13
#define RS_CF_D10B_B  12
#define RS_CF_DEND_B  11
#define RS_CF_DSTR_B  10
#define RS_CF_DENA_B   9
#define RS_CF_DMODE_B  8
#define RS_CF_PSEL_B   4
#define RS_CF_ENF_B    3
#define RS_CF_ESTR_B   2
#define RS_CF_EENA_B   1
#define RS_CF_EMODE_B  0

#define RS_CF_DNF   (1<<13)
#define RS_CF_D10B  (1<<12)
#define RS_CF_DEND  (1<<11)
#define RS_CF_DSTR  (1<<10)
#define RS_CF_DENA  (1<< 9)
#define RS_CF_DMODE (1<< 8)
#define RS_CF_PSEL  (1<< 4)
#define RS_CF_ENF   (1<< 3)
#define RS_CF_ESTR  (1<< 2)
#define RS_CF_EENA  (1<< 1)
#define RS_CF_EMODE (1<< 0)




/* NAND flash interface */

#define NF_CF       0xFC77
#define NF_CF_REG_BITS  10
#define NF_CTRL     0xFC78
#define NF_CTRL_REG_BITS 5
#define NF_PTR      0xFC79
#define NF_LEN      0xFC7A

#define NF_CF_SCLK_INV_B   9 /* invert slave clock polarity */
#define NF_CF_SLAVE_B      8 /* slave enable */
#define NF_CF_FLT_BUS_B    7 /* 1 = float bus between accesses */
#define NF_CF_INT_ENA_B    6
#define NF_CF_WAITSTATES_B 0 /* bits 5-0: wait states */

#define NF_CF_SCLK_INV   (1<<9) /* invert slave clock polarity */
#define NF_CF_SLAVE      (1<<8) /* slave enable */
#define NF_CF_FLT_BUS    (1<<7) /* 1 = float bus between accesses */
#define NF_CF_INT_ENA    (1<<6)
#define NF_CF_WAITSTATES (1<<0) /* bits 5-0: wait states */
#define NF_CF_WAITSTATES_MASK (0x3f << NF_CF_WAITSTATES_B)

#define NF_CTRL_RDY_B       4 /* RDY/BUSY state */
#define NF_CTRL_READSEL_B   2 /* 0=write, 1=read */
#define NF_CTRL_ENA_B       1 /* NF enable, '1' to start op */
#define NF_CTRL_USEPERIP_B  0 /* Use data buffer */

#define NF_CTRL_RDY       (1<<4) /* RDY/BUSY state */
#define NF_CTRL_READSEL   (1<<2) /* 0=write, 1=read */
#define NF_CTRL_WRITESEL  (0<<2) /* 0=write, 1=read */
#define NF_CTRL_ENA       (1<<1) /* NF enable, '1' to start op */
#define NF_CTRL_USEPERIP  (1<<0) /* Use data buffer */

#define NF_PTR_RENA_B   15
#define NF_PTR_RCF_B    12
#define NF_PTR_PTR_B     0

#define NF_PTR_RENA     (1<<15)
#define NF_PTR_RCF      (1<<12)
#define NF_PTR_PTR      (1<< 0)
#define NF_PTR_RCF_1024 (4<<12)
#define NF_PTR_RCF_512  (3<<12)
#define NF_PTR_RCF_256  (2<<12)
#define NF_PTR_RCF_128  (1<<12)
#define NF_PTR_RCF_64   (0<<12)




/* SD card interface */

#define SD_PTR         0xFC7C
#define SD_LEN         0xFC7D
#define SD_CF          0xFC7E
#define SD_CF_REG_BITS 13
#define SD_ST          0xFC7F
#define SD_ST_REG_BITS 13

#define SD_CF_NOCRCTX_B  12
#define SD_CF_NOCRCRST_B 11
#define SD_CF_4BIT_B     10
#define SD_CF_ENA_B       7
#define SD_CF_READSEL_B   6
#define SD_CF_CMDSEL_B    5
#define SD_CF_NOSTARTB_B  4
#define SD_CF_NOSTOPB_B   3
#define SD_CF_CRC16_B     2
#define SD_CF_CRC7_B      1
#define SD_CF_POLL_B      0

#define SD_CF_NOCRCTX  (1<<12)
#define SD_CF_NOCRCRST (1<<11)
#define SD_CF_4BIT     (1<<10)
#define SD_CF_ENA      (1<< 7)
#define SD_CF_READSEL  (1<< 6)
#define SD_CF_WRITESEL (0<< 6)
#define SD_CF_CMDSEL   (1<< 5)
#define SD_CF_DATASEL  (0<< 5)
#define SD_CF_NOSTARTB (1<< 4)
#define SD_CF_NOSTOPB  (1<< 3)
#define SD_CF_CRC16    (1<< 2)
#define SD_CF_CRC7     (1<< 1)
#define SD_CF_POLL     (1<< 0)

#define SD_ST_WAITSTATES_B 8
#define SD_ST_REPEAT_B     7
#define SD_ST_CMDBRK_B     5
#define SD_ST_DAT0_B       4
#define SD_ST_NOSTOPB_B    3
#define SD_ST_CRC16_B      2
#define SD_ST_CRC7_B       1
#define SD_ST_NOSTARTB_B   0

#define SD_ST_WAITSTATES   (1<<8)
#define SD_ST_WAITSTATES_MASK   (0x1f<<SD_ST_WAITSTATES_B)
#define SD_ST_REPEAT       (1<<7)
#define SD_ST_CMDBRK       (1<<5)
#define SD_ST_DAT0         (1<<4)
#define SD_ST_NOSTOPB_ERR  (1<<3)
#define SD_ST_CRC16_ERR    (1<<2)
#define SD_ST_CRC7_ERR     (1<<1)
#define SD_ST_NOSTARTB_ERR (1<<0)

#define SD_ST_ANY_ERR (SD_ST_NOSTOPB_ERR|SD_ST_CRC16_ERR|SD_ST_CRC7_ERR|SD_ST_NOSTARTB_ERR)



/* USB peripheral registers */

#define USB_RECV_MEM 0xF400
#define USB_SEND_MEM 0xF800

#define USB_CF            0xFC80
#define USB_CTRL          0xFC81
#define USB_ST            0xFC82
#define USB_RDPTR         0xFC83
#define USB_WRPTR         0xFC84
#define USB_UTMIR         0xFC85
#define USB_UTMIW         0xFC86
#define USB_HOST          0xFC87
#define USB_EP_SEND0      0xFC88
#define USB_EP_SEND1      0xFC89
#define USB_EP_SEND2      0xFC8A
#define USB_EP_SEND3      0xFC8B
#define USB_EP_ST0        0xFC90
#define USB_EP_ST1        0xFC91
#define USB_EP_ST2        0xFC92
#define USB_EP_ST3        0xFC93

#define USB_CF_RST_B         15
#define USB_CF_HDTOG_B       14
#define USB_CF_DDTOG_B       13
#define USB_CF_DTOGERR_B     10
#define USB_CF_NOHIGHSPEED_B 11
#define USB_CF_MASTER_B       9
#define USB_CF_RSTUSB_B       8
#define USB_CF_USBENA_B       7
#define USB_CF_USBADDR_B      0

#define USB_CF_RST         (1<<15)
#define USB_CF_HDTOG       (1<<14)
#define USB_CF_DDTOG       (1<<13)
#define USB_CF_NOHIGHSPEED (1<<11)
#define USB_CF_DTOGERR     (1<<10)
#define USB_CF_MASTER      (1<< 9)
#define USB_CF_RSTUSB      (1<< 8)
#define USB_CF_USBENA      (1<< 7)
#define USB_CF_USBADDR     (1<< 0)

#define USB_CTRL_BUS_RESET_B 15
#define USB_CTRL_SOF_B       14
#define USB_CTRL_RX_B        13
#define USB_CTRL_TX_B        11
#define USB_CTRL_NAK_B       10
#define USB_CTRL_TIME_B       9
#define USB_CTRL_SUSP_B       8
#define USB_CTRL_RESM_B       7
#define USB_CTRL_BR_START_B   6
#define USB_CTRL_DCON_B       5
#define USB_CTRL_CF_B         0

#define USB_CTRL_BUS_RESET (1<<15)
#define USB_CTRL_SOF       (1<<14)
#define USB_CTRL_RX        (1<<13)
#define USB_CTRL_TX        (1<<11)
#define USB_CTRL_NAK       (1<<10)
#define USB_CTRL_TIME      (1<< 9)
#define USB_CTRL_SUSP      (1<< 8)
#define USB_CTRL_RESM      (1<< 7)
#define USB_CTRL_BR_START  (1<< 6)
#define USB_CTRL_DCON      (1<< 5)
#define USB_CTRL_CF        (1<< 0)

#define USB_ST_BRST_B      15
#define USB_ST_SOF_B       14
#define USB_ST_RX_B        13
#define USB_ST_TX_HLD_B    12
#define USB_ST_TX_EMPTY_B  11
#define USB_ST_NAK_B       10
#define USB_ST_TIME_B       9
#define USB_ST_SUSPI_B      8
#define USB_ST_RES_B        7
#define USB_ST_MTERR_B      6
#define USB_ST_STAT_B       5
#define USB_ST_SPD_B        4
#define USB_ST_PID_B        0

#define USB_ST_BRST      (1<<15)
#define USB_ST_SOF       (1<<14)
#define USB_ST_RX        (1<<13)
#define USB_ST_TX_HLD    (1<<12)
#define USB_ST_TX_EMPTY  (1<<11)
#define USB_ST_NAK       (1<<10)
#define USB_ST_TIME      (1<< 9)
#define USB_ST_SUSPI     (1<< 8)
#define USB_ST_RES       (1<< 7)
#define USB_ST_MTERR     (1<< 6)
#define USB_ST_STAT      (1<< 5)
#define USB_ST_SPD       (1<< 4)
#define USB_ST_PID       (1<< 0)

#define USB_UTMIR_LSTATE_B 14
#define USB_UTMIR_CNT_B     0

#define USB_UTMIR_LSTATE (1<<14)
#define USB_UTMIR_CNT    (1<< 0)

#define USB_UTMIW_ORIDE_B   15
#define USB_UTMIW_LBACK_B   14
#define USB_UTMIW_AUTOSOF_B 12
#define USB_UTMIW_J_B        6
#define USB_UTMIW_HSHK_B     5
#define USB_UTMIW_K_B        4
#define USB_UTMIW_RCVSEL_B   3
#define USB_UTMIW_TERMSEL_B  2
#define USB_UTMIW_OPMOD_B    0

#define USB_UTMIW_ORIDE   (1<<15)
#define USB_UTMIW_LBACK   (1<<14)
#define USB_UTMIW_AUTOSOF (1<<12)
#define USB_UTMIW_J       (1<< 6)
#define USB_UTMIW_HSHK    (1<< 5)
#define USB_UTMIW_K       (1<< 4)
#define USB_UTMIW_RCVSEL  (1<< 3)
#define USB_UTMIW_TERMSEL (1<< 2)
#define USB_UTMIW_OPMOD   (1<< 0)

#define USB_HOST_PID_B   12
#define USB_HOST_ISOC_B  11
#define USB_HOST_TX_B     9

#define USB_HOST_PID   (1<<12)
#define USB_HOST_ISOC  (1<<11)
#define USB_HOST_TX    (1<< 9)

#define USB_EP_SEND_TXR_B  15
#define USB_EP_SEND_ADDR_B 10
#define USB_EP_SEND_LEN_B   0

#define USB_EP_SEND_TXR  (1<<15)
#define USB_EP_SEND_ADDR (1<<10)
#define USB_EP_SEND_LEN  (1<< 0)

#define USB_EP_ST_OTYP_B      14
#define USB_EP_ST_OENA_B      13
#define USB_EP_ST_OSTL_B      12
#define USB_EP_ST_OSTL_SENT_B 11
#define USB_EP_ST_OEP_SIZE_B   8
#define USB_EP_ST_ITYP_B       6
#define USB_EP_ST_INT_ENA_B    5
#define USB_EP_ST_ISTL_B       4
#define USB_EP_ST_ISTL_SENT_B  3
#define USB_EP_ST_INAKSENT_B   2
#define USB_EP_ST_IXMIT_EMP_B  1
#define USB_EP_ST_FORCE_NAK_B  0

#define USB_EP_ST_OTYP      (1<<14)
#define USB_EP_ST_OENA      (1<<13)
#define USB_EP_ST_OSTL      (1<<12)
#define USB_EP_ST_OSTL_SENT (1<<11)
#define USB_EP_ST_OEP_SIZE  (1<< 8) /* OUT endpoint size */
#define USB_EP_ST_ITYP      (1<< 6)
#define USB_EP_ST_IENA      (1<< 5)
#define USB_EP_ST_INT_ENA   obsoleted("actually USB_EP_ST_IENA")
#define USB_EP_ST_ISTL      (1<< 4)
#define USB_EP_ST_ISTL_SENT (1<< 3)
#define USB_EP_ST_INAKSENT  (1<< 2)
#define USB_EP_ST_IXMIT_EMP (1<< 1)
#define USB_EP_ST_FORCE_NAK (1<< 0) /* Force NAK */

/* Endpoint types for both in and out endpoints. */
#define USB_EP_ST_TYP_BULK 0 
#define USB_EP_ST_TYP_INT  1
#define USB_EP_ST_TYP_ISO  3


/* Interruptable general purpose IO ports 0-2 */

#define GPIO0_DDR 	  0xFCA0
#define GPIO0_ODATA 	  0xFCA1
#define GPIO0_IDATA 	  0xFCA2
#define GPIO0_INT_FALL 	  0xFCA3
#define GPIO0_INT_RISE 	  0xFCA4
#define GPIO0_INT_PEND 	  0xFCA5
#define GPIO0_SET_MASK 	  0xFCA6
#define GPIO0_CLEAR_MASK  0xFCA7
#define GPIO0_BIT_CONF    0xFCA8
#define GPIO0_BIT_ENG0    0xFCA9
#define GPIO0_BIT_ENG1    0xFCAA

#define GPIO1_DDR 	  0xFCC0
#define GPIO1_ODATA 	  0xFCC1
#define GPIO1_IDATA 	  0xFCC2
#define GPIO1_INT_FALL 	  0xFCC3
#define GPIO1_INT_RISE 	  0xFCC4
#define GPIO1_INT_PEND 	  0xFCC5
#define GPIO1_SET_MASK 	  0xFCC6
#define GPIO1_CLEAR_MASK  0xFCC7
#define GPIO1_BIT_CONF    0xFCC8
#define GPIO1_BIT_ENG0    0xFCC9
#define GPIO1_BIT_ENG1    0xFCCA

#define GPIO2_DDR 	  0xFCE0
#define GPIO2_ODATA 	  0xFCE1
#define GPIO2_IDATA 	  0xFCE2
#define GPIO2_INT_FALL 	  0xFCE3
#define GPIO2_INT_RISE 	  0xFCE4
#define GPIO2_INT_PEND 	  0xFCE5
#define GPIO2_SET_MASK 	  0xFCE6
#define GPIO2_CLEAR_MASK  0xFCE7
#define GPIO2_BIT_CONF    0xFCE8
#define GPIO2_BIT_ENG0    0xFCE9
#define GPIO2_BIT_ENG1    0xFCEA

#define GPIO_BE_DAT1_B 12
#define GPIO_BE_IO1_B   8
#define GPIO_BE_DAT0_B  4
#define GPIO_BE_IO0_B   0

#define GPIO_BE_DAT1 (1<<12)
#define GPIO_BE_IO1  (1<< 8)
#define GPIO_BE_DAT0 (1<< 4)
#define GPIO_BE_IO0  (1<< 0)




/* S/PDIF peripheral */

// 
//    SPDIF_ADDR_c : FD0x/FD1x
// 
#define SP_RX_CF	0xFD00
#define SP_RX_CF_REG_BITS 4
#define SP_RX_CLKDIV	0xFD01
#define SP_RX_CLKDIV_REG_BITS 8
#define SP_LDATA_LSB	0xFD02
#define SP_LDATA	0xFD03
#define SP_RDATA_LSB	0xFD04
#define SP_RDATA	0xFD05
#define SP_RX_STAT	0xFD06
#define SP_RX_BLFRCNT	0xFD07
#define SP_TX_CHST0	0xFD08
#define SP_TX_CHST1	0xFD09
#define SP_TX_CHST2	0xFD0A
#define SP_TX_CHST2_REG_BITS 14
#define SP_TX_CF	0xFD0B

#define SP_RX_CF_EN_B      3
#define SP_RX_CF_INT_ENA_B 1

#define SP_RX_CF_EN      (1<<3)
#define SP_RX_CF_INT_ENA (1<<1)

#define SP_RX_STAT_CHSCH_B   15
#define SP_RX_STAT_FRCV_B    14
#define SP_RX_STAT_MISS_B    12
#define SP_RX_STAT_BERR_B    11
#define SP_RX_STAT_FERR_B    10
#define SP_RX_STAT_SFERR_B    9
#define SP_RX_STAT_BIPHERR_B  8
#define SP_RX_STAT_RPERR_B    7
#define SP_RX_STAT_LPERR_B    6
#define SP_RX_STAT_RV_B       5
#define SP_RX_STAT_RU_B       4
#define SP_RX_STAT_RC_B       3
#define SP_RX_STAT_LV_B       2
#define SP_RX_STAT_LU_B       1
#define SP_RX_STAT_LC_B       0

#define SP_RX_STAT_CHSCH   (1<<15)
#define SP_RX_STAT_FRCV    (1<<14)
#define SP_RX_STAT_MISS    (1<<12)
#define SP_RX_STAT_BERR    (1<<11)
#define SP_RX_STAT_FERR    (1<<10)
#define SP_RX_STAT_SFERR   (1<< 9)
#define SP_RX_STAT_BIPHERR (1<< 8)
#define SP_RX_STAT_RPERR   (1<< 7)
#define SP_RX_STAT_LPERR   (1<< 6)
#define SP_RX_STAT_RV      (1<< 5)
#define SP_RX_STAT_RU      (1<< 4)
#define SP_RX_STAT_RC      (1<< 3)
#define SP_RX_STAT_LV      (1<< 2)
#define SP_RX_STAT_LU      (1<< 1)
#define SP_RX_STAT_LC      (1<< 0)

#define SP_RX_BLCNT_B 8
#define SP_RX_FRCNT_B 0

#define SP_RX_BLCNT (1<<8)
#define SP_RX_FRCNT (1<<0)

#define SP_TX_CHST0_CAT_B        8
#define SP_TX_CHST0_MD0_B        6
#define SP_TX_CHST0_PCMM_B       3
#define SP_TX_CHST0_CP_B         2
#define SP_TX_CHST0_PCM_B        1
#define SP_TX_CHST0_PROCON_B     0

#define SP_TX_CHST0_CAT        (1<<8)
#define SP_TX_CHST0_MD0        (1<<6)
#define SP_TX_CHST0_PCMM       (1<<3)
#define SP_TX_CHST0_CP         (1<<2)
#define SP_TX_CHST0_PCM        (1<<1)
#define SP_TX_CHST0_PROCON     (1<<0)

#define SP_TX_CHST1_CLKA_B      12
#define SP_TX_CHST1_FS_B         8
#define SP_TX_CHST1_CH_B         4
#define SP_TX_CHST1_SRC_B        0

#define SP_TX_CHST1_CLKA      (1<<12)
#define SP_TX_CHST1_FS        (1<< 8)
#define SP_TX_CHST1_CH        (1<< 4)
#define SP_TX_CHST1_SRC       (1<< 0)

#define SP_TX_CHST2_ST_NWRQ_B   13
#define SP_TX_CHST2_TX_ENA_B    12
#define SP_TX_CHST2_RS1_RU_B    11
#define SP_TX_CHST2_RS1_RV_B    10
#define SP_TX_CHST2_LS1_RU_B     9
#define SP_TX_CHST2_LS1_RV_B     8
#define SP_TX_CHST2_CH2_FSO_B    4
#define SP_TX_CHST2_CH2_WRDL_B   1
#define SP_TX_CHST2_CH2_WRDLM_B  0

#define SP_TX_CHST2_ST_NWRQ   (1<<13)
#define SP_TX_CHST2_TX_ENA    (1<<12)
#define SP_TX_CHST2_RS1_RU    (1<<11)
#define SP_TX_CHST2_RS1_RV    (1<<10)
#define SP_TX_CHST2_LS1_RU    (1<< 9)
#define SP_TX_CHST2_LS1_RV    (1<< 8)
#define SP_TX_CHST2_CH2_FSO   (1<< 4)
#define SP_TX_CHST2_CH2_WRDL  (1<< 1)
#define SP_TX_CHST2_CH2_WRDLM (1<< 0)

#define SP_TX_CF_CLKDIV_B 2
#define SP_TX_CF_IE_B     1
#define SP_TX_CF_SND_B    0

#define SP_TX_CF_CLKDIV (1<<2)
#define SP_TX_CF_IE     (1<<1)
#define SP_TX_CF_SND    (1<<0)




/* UART peripheral */

#define UART_STATUS 	0xFE00
#define UART_STATUS_REG_BITS 5
#define UART_DATA 	0xFE01
#define UART_DATAH 	0xFE02
#define UART_DIV       	0xFE03

#define UART_ST_FRAMERR_B   4
#define UART_ST_RXORUN_B    3
#define UART_ST_RXFULL_B    2
#define UART_ST_TXFULL_B    1
#define UART_ST_TXRUNNING_B 0

#define UART_ST_FRAMERR   (1<<4)
#define UART_ST_RXORUN    (1<<3)
#define UART_ST_RXFULL    (1<<2)
#define UART_ST_TXFULL    (1<<1)
#define UART_ST_TXRUNNING (1<<0)

#define UART_DIV_D1_B 8
#define UART_DIV_D2_B 0

#define UART_DIV_D1 (1<<8)
#define UART_DIV_D2 (1<<0)

#define UART_DIV_D1_MASK (255<<UART_DIV_D1_B)
#define UART_DIV_D2_MASK (255<<UART_DIV_D2_B)


/* Watchdog peripheral */

#define WDOG_CF         0xFE20
#define WDOG_KEY        0xFE21
#define WDOG_DUMMY      0xFE22
#define WDOG_BREAK      WDOG_DUMMY
#define WDOG_KEY_VAL    0x4ea9




/* Line and mic inputs */

#define FM_CF            0xFE40
#define AD_CF            0xFE41
#define AD_CF_REG_BITS   10
#define AD_LEFT_LSB      0xFE46
#define AD_LEFT          0xFE47
#define AD_RIGHT_LSB     0xFE48
#define AD_RIGHT         0xFE49
#define AD_MONO_LSB      0xFE4A
#define AD_MONO          0xFE4B
#define DEC6_LEFT_LSB    0xFE4E
#define DEC6_LEFT        0xFE4F
#define DEC6_RIGHT_LSB   0xFE50
#define DEC6_RIGHT       0xFE51

#define FM_CF_TEST      15
#define FM_CF_UAD2_B    14
#define FM_CF_UAD1_B    13
#define FM_CF_UAD3_B    12
#define FM_CF_PHCOMP_B   7
#define FM_CF_ENABLE_B   6
#define FM_CF_RDSSYNC_B  5
#define FM_CF_MONO_B     4
#define FM_CF_DEEMP_B    3
#define FM_CF_RDSENA_B   2
#define FM_CF_CCFLCK_B   1
#define FM_CF_FM_ENA_B   0

#define FM_CF_UAD2     (1<<14)
#define FM_CF_UAD1     (1<<13)
#define FM_CF_UAD3     (1<<12)
#define FM_CF_PHCOMP   (1<< 7)
#define FM_CF_ENABLE   (1<< 6)
#define FM_CF_RDSSYNC  (1<< 5)
#define FM_CF_MONO     (1<< 4)
#define FM_CF_DEEMP    (1<< 3)
#define FM_CF_DEEMP_75 (1<< 3)
#define FM_CF_DEEMP_50 (0<< 3)
#define FM_CF_RDSENA   (1<< 2)
#define FM_CF_CCFLCK   (1<< 1)
#define FM_CF_FM_ENA   (1<< 0)

#define AD_CF_AD23_FLP_B 9
#define AD_CF_DEC6SEL_B  7
#define AD_CF_AD3FS_B    5
#define AD_CF_ADFS_B     3
#define AD_CF_DEC6ENA_B  2
#define AD_CF_AD3ENA_B   1
#define AD_CF_ADENA_B    0


#define AD_CF_AD23_FLP (1<<9)
#define AD_CF_DEC6SEL  (1<<7)
#define AD_CF_AD3FS    (1<<5)
#define AD_CF_ADFS     (1<<3)
#define AD_CF_DEC6ENA  (1<<2)
#define AD_CF_AD3ENA   (1<<1)
#define AD_CF_ADENA    (1<<0)
/* Bit pattern definitions */
#define AD_CF_ADFS_24K  (3<<AD_CF_ADFS_B)
#define AD_CF_ADFS_48K  (2<<AD_CF_ADFS_B)
#define AD_CF_ADFS_96K  (1<<AD_CF_ADFS_B)
#define AD_CF_ADFS_192K (0<<AD_CF_ADFS_B)
#define AD_CF_ADFS_MASK (3<<AD_CF_ADFS_B)
#define AD_CF_AD3FS_24K  (3<<AD_CF_AD3FS_B)
#define AD_CF_AD3FS_48K  (2<<AD_CF_AD3FS_B)
#define AD_CF_AD3FS_96K  (1<<AD_CF_AD3FS_B)
#define AD_CF_AD3FS_192K (0<<AD_CF_AD3FS_B)
#define AD_CF_AD3FS_MASK (3<<AD_CF_AD3FS_B)
#define AD_CF_24K  (3)
#define AD_CF_48K  (2)
#define AD_CF_96K  (1)
#define AD_CF_192K (0)
#define AD_CF_MASK (3)
#define AD_CF_DEC6SEL_MASK (3<<AD_CF_DEC6SEL_B)
#define AD_CF_DEC6SEL_MONO (2<<AD_CF_DEC6SEL_B)
#define AD_CF_DEC6SEL_STEREO (1<<AD_CF_DEC6SEL_B)
#define AD_CF_DEC6SEL_FM (0<<AD_CF_DEC6SEL_B)




/* FM receiver */

#define FMPLL_LO    0xFE42
#define FMPLL_HI    0xFE43
#define FMCCF_LO    0xFE44
#define FMCCF_HI    0xFE45
#define FM_LEFT     0xFE4C
#define FM_RIGHT    0xFE4D
#define RDS_DATA    0xFE52
#define RDS_CHK     0xFE53
#define RDS_CHK_REG_BITS 13
#define FM_IQLEV    0xFE54
#define FM_PILOTLEV 0xFE55
#define FM_BASEBAND 0xFE56
#define FM_SUBCARR  0xFE57
#define FM_BB_DCLEV 0xFE58
#define FM_I_RDC    0xFE59
#define FM_Q_RDC    0xFE5A
#define FM_PHSCL    0xFE5B

#define FM_PHSCL_I_B 8
#define FM_PHSCL_Q_B 0

#define FM_PHSCL_I (1<<8)
#define FM_PHSCL_Q (1<<0)

#define FM_IQLEV_IQ_B   9
#define FM_IQLEV_SENA_B 8
#define FM_IQLEV_SIMG_B 0

#define FM_IQLEV_IQ   (1<<9)
#define FM_IQLEV_SENA (1<<8)
#define FM_IQLEV_SIMG (1<<0)




/* Radio Data System (RDS) */

#define RDS_CHK_CHKW_B 3
#define RDS_CHK_ST_B   2
#define RDS_CHK_BLK_B  0

#define RDS_CHK_CHKW (1<<3)
#define RDS_CHK_ST   (1<<2)
#define RDS_CHK_BLK  (1<<0)




/* I2S peripheral */

#define I2S_LEFT_LSB  obsoleted("see I2S_OUT_LEFT_LSB,I2S_IN_LEFT_LSB")
#define I2S_LEFT      obsoleted("see I2S_OUT_LEFT,I2S_IN_LEFT")
#define I2S_RIGHT_LSB obsoleted("see I2S_OUT_RIGHT_LSB,I2S_IN_RIGHT_LSB")
#define I2S_RIGHT     obsoleted("see I2S_OUT_RIGHT,I2S_IN_RIGHT")
#define I2S_OUT_LEFT_LSB     0xFD20
#define I2S_OUT_LEFT         0xFD21
#define I2S_OUT_RIGHT_LSB    0xFD22
#define I2S_OUT_RIGHT        0xFD23
#define I2S_IN_LEFT_IN_LSB   0xFD24
#define I2S_IN_LEFT_IN       0xFD25
#define I2S_IN_RIGHT_LSB     0xFD26
#define I2S_IN_RIGHT         0xFD27
#define I2S_STATUS           0xFD28 /* */
#define I2S_STATUS_REG_BITS 6

#define I2S_ST_RXRFULL_B     5
#define I2S_ST_RXLFULL_B     4
#define I2S_ST_RXORUN_B      3
#define I2S_ST_TXRFULL_B     2
#define I2S_ST_TXLFULL_B     1
#define I2S_ST_TXURUN_B      0
#define I2S_ST_RXRFULL       (1<<5)
#define I2S_ST_RXLFULL       (1<<4)
#define I2S_ST_RXORUN        (1<<3)
#define I2S_ST_TXRFULL       (1<<2)
#define I2S_ST_TXLFULL       (1<<1)
#define I2S_ST_TXURUN        (1<<0)


#define I2S_CF           0xFE60 /* */
#define I2S_CF_REG_BITS 11
#define I2S_TDM_LEN      0xFE61 /* */
#define I2S_TDM_CFN      0xFE62


#define I2S_CF_SELMCK24M_B 10 /*I2S Master clock select, with 24MHz xtal only*/
#define I2S_CF_FRM_B        8 /* 9:8 data format selection */
#define I2S_CF_32B_B        7 /* 32-bit mode (1), 16-bit mode (0) */
#define I2S_CF_INTENA_B     6 /* interrupt enable */
#define I2S_CF_FS_B         4 /* 5:4 */
#define I2S_CF_MODE_B       3 /* output mode DSP (1) or SRC (0) out */
#define I2S_CF_ENA_B        2 /* I2S enable */
#define I2S_CF_ENAMCK_B     1 /* I2S master clock pad driver enable */
#define I2S_CF_MASTER_B     0 /* Master (1) or slave (0) mode select */

#define I2S_CF_FRM_TDM_DSP (3<<8) /* TDM mode, left-justified */
#define I2S_CF_FRM_TDM_I2S (2<<8) /* TDM mode, I2S sync */
#define I2S_CF_FRM_DSP     (1<<8) /* left-justified */
#define I2S_CF_FRM_I2S     (0<<8) /* I2S sync */
#define I2S_CF_16B        (0<<7)
#define I2S_CF_32B        (1<<7)
#define I2S_CF_INTENA     (1<<6)
#define I2S_CF_FS192K     (2<<4) /*192kHz in 16-bit mode,192kHz in 32-bit mode*/
#define I2S_CF_FS96K      (1<<4) /* 96kHz in 16-bit mode, 96kHz in 32-bit mode*/
#define I2S_CF_FS48K      (0<<4) /* 48kHz in 16-bit mode, 48kHz in 32-bit mode*/
#define I2S_CF_FS         (1<<4)
#define I2S_CF_MODE       (1<<3)
#define I2S_CF_ENA        (1<<2)
#define I2S_CF_ENAMCK     (1<<1)
#define I2S_CF_MASTER     (1<<0)
#define I2S_CF_FS192K16B  (I2S_CF_16B|I2S_CF_FS192K)
#define I2S_CF_FS96K16B   (I2S_CF_16B|I2S_CF_FS96K )
#define I2S_CF_FS48K16B   (I2S_CF_16B|I2S_CF_FS48K )
#define I2S_CF_FS192K32B  (I2S_CF_32B|I2S_CF_FS192K)
#define I2S_CF_FS96K32B   (I2S_CF_32B|I2S_CF_FS96K)
#define I2S_CF_FS48K32B   (I2S_CF_32B|I2S_CF_FS48K )
#define I2S_CF_FS24K32B   obsoleted("N/A, I2S fixed")

#define I2S_CF_FS_MASK    (3*I2S_CF_FS) /* Mask for sample rate */
#define I2S_CF_FS_BITS_MASK (I2S_CF_FS_MASK|I2S_CF_32B) /* Mask for sample rate and number of bits */




/* Timer peripheral */

#define TIMER_CF 	0xfe80
#define TIMER_CF_REG_BITS	7
#define TIMER_ENA 	0xfe81
#define TIMER_ENA_REG_BITS 	3
#define TIMER_T0L    	0xfe84
#define TIMER_T0H    	0xfe85
#define TIMER_T0CNTL 	0xfe86
#define TIMER_T0CNTH 	0xfe87
#define TIMER_T1L    	0xfe88
#define TIMER_T1H    	0xfe89
#define TIMER_T1CNTL 	0xfe8a
#define TIMER_T1CNTH 	0xfe8b
#define TIMER_T2L    	0xfe8c
#define TIMER_T2H    	0xfe8d
#define TIMER_T2CNTL 	0xfe8e
#define TIMER_T2CNTH 	0xfe8f

#define TIMER_CF_CLKDIV_B 0
#define TIMER_CF_CLKDIV 1

#define TIMER_ENA_T0_B	0
#define TIMER_ENA_T1_B	1
#define TIMER_ENA_T2_B	2

#define TIMER_ENA_T0	1
#define TIMER_ENA_T1	2
#define TIMER_ENA_T2	4




/* Real time clock (RTC) */

#define RTC_LOW          0xFEA0
#define RTC_HIGH         0xFEA1
#define RTC_CF           0xFEA2
#define RTC_CF_REG_BITS 5

#define RTC_CF_GSCK_B    4
#define RTC_CF_EXEC_B    3
#define RTC_CF_RDBUSY_B  2
#define RTC_CF_DBUSY_B   1
#define RTC_CF_IBUSY_B   0

#define RTC_CF_GSCK    (1<<4)
#define RTC_CF_EXEC    (1<<3)
#define RTC_CF_RDBUSY  (1<<2)
#define RTC_CF_DBUSY   (1<<1)
#define RTC_CF_IBUSY   (1<<0)

// RTC instructions
#define RTC_I_RESET      0xEB00
#define RTC_I_LOADRTC    0x5900
#define RTC_I_READRTC    0x5600
#define RTC_I_LOADSLP    0xBD00
#define RTC_I_READSLP    0xB200
#define RTC_I_DIV128     0xC700
#define RTC_I_SLPMODE    0xA300
#define RTC_I_ALARM      0xAC00
#define RTC_I_MEM_WR     0x3500
#define RTC_I_MEM_RD     0x3A00




/* 10-bit successive approximation register analog-to-digital converter (SAR)*/

// DAC_IF (analog control i.e. 12MHz clock domain)
#define SAR_DAT          0xFECD
#define SAR_CF           0xFED6
#define SAR_CF_REG_BITS 12

#define SAR_CF_SEL_B       8    // bits 11:8
#define SAR_CF_ENA_B       7
#define SAR_CF_CONTMODE_B  6 //1=continuous, 0=once
#define SAR_CF_CK_B        0

#define SAR_SEL_AUX0   (12<<8)
#define SAR_SEL_VHIGH  (10<<8)
#define SAR_SEL_AUX1    (7<<8)
#define SAR_SEL_RTC     (6<<8)
#define SAR_SEL_CVDD    (5<<8)
#define SAR_SEL_AUX4    (4<<8)
#define SAR_SEL_VCO     (3<<8)
#define SAR_SEL_AUX3    (2<<8)
#define SAR_SEL_DLL     (1<<8)
#define SAR_SEL_AUX2    (0<<8)
#define SAR_CF_SEL      (1<<8)    // bits 11:8
#define SAR_CF_ENA      (1<<7)
#define SAR_CF_CONTMODE (1<<6)    // 1=continuous, 0=once
#define SAR_CF_CK       (1<<0)




/* Pulse width modulation unit */

// DAC_IF (analog control i.e. 12MHz clock domain)
#define PWM_FRAMELEN     0xFED4
#define PWM_PULSELEN     0xFED5
#define PWM_PULSE obsoleted("PWM_PULSE, use PWM_FRAMELEN")
#define PWM_FRAME obsoleted("PWM_FRAME, use PWM_PULSELEN")



/* GPIO definitions */
#define GPIO0_READY	(1<< 8)
#define GPIO0_RD	(1<< 9)
#define GPIO0_WR	(1<<10)
#define GPIO0_CS1	(1<<11)
#define GPIO0_SPDIF_IN 	(1<<12)
#define GPIO0_SPDIF_OUT	(1<<13)
#define GPIO0_CLE	(1<<14)
#define GPIO0_ALE	(1<<15)

#define GPIO0_READY_B		( 8)
#define GPIO0_RD_B		( 9)
#define GPIO0_WR_B		(10)
#define GPIO0_CS1_B		(11)
#define GPIO0_SPDIF_IN_B 	(12)
#define GPIO0_SPDIF_OUT_B	(13)
#define GPIO0_CLE_B		(14)
#define GPIO0_ALE_B		(15)


#define GPIO1_SPI0_XCS   (1<< 0)
#define GPIO1_SPI0_SCLK  (1<< 1)
#define GPIO1_SPI0_MISO  (1<< 2)
#define GPIO1_SPI0_MOSI  (1<< 3)
#define GPIO1_SPI1_XCS   (1<< 4)
#define GPIO1_SPI1_SCLK  (1<< 5)
#define GPIO1_SPI1_MISO  (1<< 6)
#define GPIO1_SPI1_MOSI  (1<< 7)
#define GPIO1_UART_RX    (1<< 8)
#define GPIO1_UART_TX    (1<< 9)
#define GPIO1_I2S_DI     (1<<10)
#define GPIO1_I2S_DO     (1<<11)
#define GPIO1_I2S_BITCLK (1<<12)
#define GPIO1_I2S_FRAME  (1<<13)
#define GPIO1_I2S_MCLK   (1<<14)

#define GPIO1_SPI0_XCS_B   ( 0)
#define GPIO1_SPI0_SCLK_B  ( 1)
#define GPIO1_SPI0_MISO_B  ( 2)
#define GPIO1_SPI0_MOSI_B  ( 3)
#define GPIO1_SPI1_XCS_B   ( 4)
#define GPIO1_SPI1_SCLK_B  ( 5)
#define GPIO1_SPI1_MISO_B  ( 6)
#define GPIO1_SPI1_MOSI_B  ( 7)
#define GPIO1_UART_RX_B    ( 8)
#define GPIO1_UART_TX_B    ( 9)
#define GPIO1_I2S_DI_B     (10)
#define GPIO1_I2S_DO_B     (11)
#define GPIO1_I2S_BITCLK_B (12)
#define GPIO1_I2S_FRAME_B  (13)
#define GPIO1_I2S_MCLK_B   (14)


#define GPIO2_VCOOUT  (1<<4)
#define GPIO2_DBGREQ  (1<<4)
#define GPIO2_TCK     (1<<3)
#define GPIO2_TDO     (1<<2)
#define GPIO2_TDI     (1<<1)
#define GPIO2_TMS     (1<<0)

#define GPIO2_VCOOUT_B  (4)
#define GPIO2_DBGREQ_B  (4)
#define GPIO2_TCK_B     (3)
#define GPIO2_TDO_B     (2)
#define GPIO2_TDI_B     (1)
#define GPIO2_TMS_B     (0)





#ifdef ASM

#ifdef PERIP_IN_X
#define STP stx
#define LDP ldx
#else
#define STP sty
#define LDP ldy
#endif

#define MR_FRACT 0

#else

#define PERIP(a) USEY(a)
#define PERIP32(a) (*(__mem_y volatile u_int32 *)(u_int16)(a))

/*
  SpiLoad() do not return!
   m24 = 0 for 16-bit SPI EEPROM address.
   m24 > 0 for 24-bit SPI EEPROM address.
   m24 < 0 for 32-bit SPI EEPROM address.
 */
void SpiLoad(register __i2 s_int16 startAddr, register __i0 s_int16 m24);
void SpiLoadLong(register __reg_a u_int32 startAddr, register __i0 s_int16 m24);
void SpiDelay(register __a0 u_int16 wait);
auto u_int16 SpiSendReceive(register __a0 u_int16 data);

void Restart(void);

/* These are called through IRAM vectors, by default FAT16/32. */
auto u_int16 InitFileSystem(void);
auto s_int16 OpenFile(register __c0 u_int16 fileNum);
auto s_int16 ReadFile(register __i3 u_int16 *buf,
		      register __c1 s_int16 byteOff,
		      register __c0 s_int16 byteSize
		      /*<0 for little-endian target buffer order*/);
u_int32 Seek(register __reg_a u_int32 pos); /**< Sets pos, returns old pos */
u_int32 Tell(void); /**< Gets pos */
auto u_int16 ReadDiskSector(register __i0 u_int16 *buffer,
			    register __reg_a u_int32 sector);

void IdleHook(void);
void Disable(void);
void Enable(void);
void Halt(void);
void Sleep(void);
void NullHook(void);
void *SetHookFunction(register __i0 u_int16 hook, register __a0 void *newFunc);

void BootFromX(register __i0 u_int16 *start);
void SinTest(void);
void MemTests(register short __b0 muxTestResult);
void TestMode(void); /* New test mode boot routine */

/* power.c */
enum voltIdx {
    /* for VS2000A */
    voltCorePlayer = 0, voltIoPlayer,  voltAnaPlayer,
    voltCoreUSB,        voltIoUSB,     voltAnaUSB,
    voltCoreSuspend,    voltIoSuspend, voltAnaSuspend,
    voltCoreUser,       voltIoUser,    voltAnaUser,
    voltEnd
};

enum powerConv {
  powerConvCore = 0, powerConvIo, powerConvAna, powerConvIntFlash
};


extern u_int16 voltages[voltEnd];
extern s_int16 coreVoltageOffset;
/* PowerSetVoltages() updates SCI_SYSTEM, and loads into Regulator. */
void BusyWait10(void); /* 10ms at 1.0x 12MHz */
void BusyWait1(void);  /* 1ms at 1.0x 12MHz */
void PowerSetVoltages(u_int16 volt[3]);
void PowerSetFlash(u_int16 flashVolt);
auto s_int16 PowerMillivoltsToReg(register enum powerConv pConv,
				  register s_int16 mV);
auto s_int16 PowerRegToMillivolts(register enum powerConv pConv,
				  register s_int16 regVal);
void PowerShutDownFlash(void);
void PowerOff(void);
void RealPowerOff(void);
void LoadCheck(struct CodecServices *cs, s_int16 n);
void RealLoadCheck(struct CodecServices *cs, s_int16 n);

#if 0
extern s_int16 osWorkSpace[OS_WORK_SPACE_SIZE];
#endif

/*
  Mapper using Y-RAM (physical is a pointer to Y memory),
  cacheSize gives the disk size in blocks (max 255).
 */
struct FsMapper *FsMapRamCreate(struct FsPhysical *physical,
				u_int16 cacheSize);

void putch(register __a0 s_int16 ch); /**< raw polled UART send */
s_int16 getch(void);                  /**< raw polled UART receive */

#if 0
u_int32 ReadIMem(register __i0 void *addr);
void WriteIMem(register __i0 void *addr, register __reg_a u_int32 ins);
#endif

extern u_int16 memPoolX[MEMPOOL_X_SIZE];
extern u_int16 __mem_y memPoolY[MEMPOOL_Y_SIZE];

enum serialNumberParam {
  snSerial,
  snLot,
  snCustomer
};

auto s_int32 ReadSerialNumber(register __c0 u_int16 bit);

#endif /*!ASM*/

#endif /*__VS1005G_H__*/
