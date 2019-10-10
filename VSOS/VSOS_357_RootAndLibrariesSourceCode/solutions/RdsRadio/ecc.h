#ifndef ECC_H
#define ECC_H

#include <vo_stdio.h>
#include <vs1005h.h>
#include "rds.h"



// returns 0 if data is valid, 1 if invalid
u_int16 CheckSyndrome(register u_int16 *data, register u_int16 check, register u_int16 offset);



// Legacy
void InitCrc(register struct rdsCrc *rdsCrc);

u_int16 RdsCrc(register u_int16 wData);

u_int16 FixBlock(register struct rdsBlock *rdsBlock, register struct rdsCrc *rdsCrc);

u_int16 RdsBlockFix (register u_int16 *data, register u_int16 check, register u_int16 offset);

#endif
