#include <vo_stdio.h>
#include <vstypes.h>
#include <exec.h>
#include <stdlib.h>
#include <sysmemory.h>
#include "taskandstack.h"

void FreeTaskAndStack(register struct TaskAndStack *tas) {
  if (tas) {
    if (tas->stack) {
      FreeMemXY(tas->stack, tas->stackSize);
    }
    free(tas);
  }
}
