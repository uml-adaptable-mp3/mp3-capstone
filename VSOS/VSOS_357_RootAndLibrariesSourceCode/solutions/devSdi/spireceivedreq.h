#ifndef __SPIRECEIVEDREQ_H__
#define __SPIRECEIVEDREQ_H__


/** These routines use interrupt-driven SPI receive into a FIFO in Y memory.
    For initialization (if done more than once)... reset FIFO read and write
    pointers to point to SpiFifoY.
 **/
#define SPIFIFOY_SIZE_WORDS (4*512/2) //Size of the FIFO in words
#define SPIFIFOY_DREQ_THRESHOLD_BYTES 512 //
#define GPIO_DREQ_BIT  0 //The bit number of the GPIO to be used as DREQ
#define GPIO_DREQ_PORT 2 //The GPIO port to be used for DREQ (in ASM only!)
#define GPIO_CANCEL_BIT 3 //

#ifndef ASM

extern __align __mem_y u_int16 SpiFifoY[SPIFIFOY_SIZE_WORDS];
extern __mem_y struct {
    __mem_y u_int16 *wr; //initialized to SpiFifoY
    __mem_y u_int16 *rd; //initialized to SpiFifoY
    u_int16 modulo;
    void *int_glob_ena;
} SpiFIFOYVars;

/** The interrupt routine, preceded by its interrupt vector.
    The configured DREQ pin will be turned low if there is less room than
    SPIFIFOY_DREQ_THRESHOLD_BYTES (+2 because the FIFO can never be filled
    completely).

    The GPIO pin must be configured as output in initialization. For example:
    SpiFIFOVars.wr = SpiFifoY;
    SpiFIFOVars.rd = SpiFifoY;
    WriteIMem((void *)(32+INTV_SPI1), ReadIMem((void *)((u_int16)SpiFifoYInt-1)));
    PERIP(GPIO2_MODE) &= ~(1<<GPIO_DREQ_BIT);
    PERIP(GPIO2_DDR)  |= (1<<GPIO_DREQ_BIT);
    PERIP(GPIO2_SET_MASK) = (1<<GPIO_DREQ_BIT);
    ... SPI receiver configuration
    PERIP(INT_ENABLE_H0) |= INTF_SPI1;
 */
void SpiFifoYInt(void);
/** Returns the fill state of the FIFO in words*/
s_int16 SpiFifoYFill(void);
/** Read words from the FIFO. Note that is you read more data than exists,
    the read pointer will wrap and you'll get ghost data. So, don't do that,
    always use SpiFifoYFill() first.
    SpiFifoGet() will also set the configured DREQ pin high if there is room
    for SPIFIFOY_DREQ_THRESHOLD_BYTES+16 bytes.
*/
void SpiFifoYGet(register __i3 u_int16 *buf, register __a0 s_int16 words);


#endif/*!ASM*/

#endif/* __SPIRECEIVEDREQ_H__*/
