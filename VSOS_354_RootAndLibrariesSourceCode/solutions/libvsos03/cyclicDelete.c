#include <vo_stdio.h>
#include <lists.h>
#include <timers.h>
#include <audio.h>
#include <exec.h>
#include "cyclic.h"

/* Deletes the cyclic task */
struct Cyclic *DeleteCyclic(void) {
  cyclic.stop = 1;
  if (!cyclic.taskAndStack) {
    return NULL;
  }
  while (cyclic.taskAndStack->task.tc_State &&
	 cyclic.taskAndStack->task.tc_State != TS_REMOVED) {
    Delay(TICKS_PER_SEC/100);
  }
  FreeTaskAndStack(cyclic.taskAndStack);
  cyclic.taskAndStack = NULL;
  return &cyclic;
}
