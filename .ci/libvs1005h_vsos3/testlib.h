#ifndef __TESTLIB_H__
#define __TESTLIB_H__

#include <vstypes.h>


void TEST_ClearIFlash(void);
void TEST_WriteIFlashOpenRecord(void);
void TEST_ProductionTest(void);
void TEST_ClearRamAndAllowDebug(register u_int16 clearRam);
u_int16 SpiWaitStatus(void);


#endif /*!__TESTLIB_H__*/
