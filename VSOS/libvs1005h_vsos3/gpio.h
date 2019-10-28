#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef VS1000

// GPIO pin usage in VS1000
#define GPIO_IS_HIGH(bitmask) (USEX(GPIO0_IDATA) & ((bitmask)))
#define GPIO_IS_LOW(bitmask) (~USEX(GPIO0_IDATA) & ((bitmask)))
#define GPIO_SET_HIGH(bitmask) {USEX(GPIO0_SET_MASK) = ((bitmask));}
#define GPIO_SET_LOW(bitmask) {USEX(GPIO0_CLEAR_MASK) = ((bitmask));}
#define GPIO_CONFIGURE_AS_OUTPUT(bitmask) {USEX(GPIO0_DDR) |= ((bitmask));}
#define GPIO_CONFIGURE_AS_INPUT(bitmask) {USEX(GPIO0_DDR) &= ~((bitmask));}
#define GPIO_CONFIGURE_AS_GPIO(bitmask) {USEX(GPIO0_MODE) |= ((bitmask));}
#define GPIO_CONFIGURE_AS_PERIPHERAL(bitmask) {USEX(GPIO0_MODE) &= ~((bitmask));}

#define GPIO1_IS_HIGH(bitmask) (USEX(GPIO1_IDATA) & ((bitmask)))
#define GPIO1_IS_LOW(bitmask) (~USEX(GPIO1_IDATA) & ((bitmask)))
#define GPIO1_SET_HIGH(bitmask) {USEX(GPIO1_SET_MASK) = ((bitmask));}
#define GPIO1_SET_LOW(bitmask) {USEX(GPIO1_CLEAR_MASK) = ((bitmask));}
#define GPIO1_CONFIGURE_AS_OUTPUT(bitmask) {USEX(GPIO1_DDR) |= ((bitmask));}
#define GPIO1_CONFIGURE_AS_INPUT(bitmask) {USEX(GPIO1_DDR) &= ~((bitmask));}
#define GPIO1_CONFIGURE_AS_GPIO(bitmask) {USEX(GPIO1_MODE) |= ((bitmask));}
#define GPIO1_CONFIGURE_AS_PERIPHERAL(bitmask) {USEX(GPIO1_MODE) &= ~((bitmask));}

#else

// GPIO pin usage in VS1001, VS1011, VS1002, VS1003, VS1053 and VS8053
#define GPIO_IS_HIGH(bitmask) (USEX(GPIO_IDATA) & ((bitmask)))
#define GPIO_IS_LOW(bitmask) (~USEX(GPIO_IDATA) & ((bitmask)))
#define GPIO_SET_HIGH(bitmask) {USEX(GPIO_ODATA) |= ((bitmask));}
#define GPIO_SET_LOW(bitmask) {USEX(GPIO_ODATA) &= ~((bitmask));}
#define GPIO_CONFIGURE_AS_OUTPUT(bitmask) {USEX(GPIO_DDR) |= ((bitmask));}
#define GPIO_CONFIGURE_AS_INPUT(bitmask) {USEX(GPIO_DDR) &= ~((bitmask));}
#define GPIO_CONFIGURE_AS_GPIO(bitmask) {} /* VS1053 does not have separate GPIO/PERIP modes for pins */
#define GPIO_CONFIGURE_AS_PERIPHERAL(bitmask) {}

#endif

#define DREQ_SET_HIGH {USEX(SER_DREQ) = ~0u;}
#define DREQ_SET_LOW {USEX(SER_DREQ) = 0;}


#endif
