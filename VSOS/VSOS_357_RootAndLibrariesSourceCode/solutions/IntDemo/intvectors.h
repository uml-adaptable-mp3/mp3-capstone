#ifndef INT_VECTORS_H
#define INT_VECTORS_H

#include <vstypes.h>

#ifndef ASM

extern u_int16 IntSarVector;
extern u_int16 IntPwmVector;
extern u_int16 IntReguVector;
extern u_int16 IntStxVector;
extern u_int16 IntSrxVector;
extern u_int16 IntRdsVector;
extern u_int16 IntRtcVector;
extern u_int16 IntDaOSetVector;
extern u_int16 IntSrcVector;
extern u_int16 IntFmVector;
extern u_int16 IntTimer2Vector;
extern u_int16 IntTimer1Vector;
extern u_int16 IntTimer0Vector;
extern u_int16 IntUartRxVector;
extern u_int16 IntUartTxVector;
extern u_int16 IntI2sVector;
extern u_int16 IntMac2Vector;
extern u_int16 IntGpio2Vector;
extern u_int16 IntGpio1Vector;
extern u_int16 IntGpio0Vector;
extern u_int16 IntMac0Vector;
extern u_int16 IntMac1Vector;
extern u_int16 IntSpi1Vector;
extern u_int16 IntSpi0Vector;
extern u_int16 IntXPeripVector;
extern u_int16 IntUsbVector;
extern u_int16 IntDacVector;

#endif /* !ASM */

#endif /* !INT_VECTORS */
