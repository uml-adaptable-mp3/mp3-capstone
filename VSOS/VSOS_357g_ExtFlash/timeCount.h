#ifndef __TIME_COUNT_H__
#define __TIME_COUNT_H__

#include <vstypes.h>

/*
  To activate 1 ms time counter, replace:
    AddIntServer(INTV_TIMER2, ScheduleHandler);
  with
    AddIntServer(INTV_TIMER2, TimeCountAndScheduleHandler);

*/

void TimeCountAndScheduleHandler(void);
extern __y u_int32 timeCount;

#endif
