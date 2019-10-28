#include <vo_stdio.h>
#include <vstypes.h>
#include <exec.h>
#include <stdlib.h>
#include <sysmemory.h>
#include "taskandstack.h"

struct TaskAndStack *CreateTaskAndStack(register void(*func)(void),
					register const char *name,
					register int stackSize,
					register int priority) {
  struct TaskAndStack *tas = calloc(1, sizeof(struct TaskAndStack));
  if (tas) {
    tas->stack = AllocMemXY(stackSize, 1);
    if (tas->stack) {
      struct TASK *task = &tas->task;
      task->tc_Node.pri = priority;
      task->tc_Node.name = name;
      task->tc_Stack = tas->stack;
      task->tc_StackSize = stackSize;
      task->tc_SigWait = 0;
      task->tc_SigRecvd = 0;
      AddTask(task, func);
      Yield();
      tas->stackSize = stackSize;
    } else {
      free(tas);
      tas = NULL;
    }
  }

 finally:
  return tas;
}

void FreeTaskAndStack(register struct TaskAndStack *tas) {
  if (tas) {
    if (tas->stack) {
      FreeMemXY(tas->stack, tas->stackSize);
    }
    free(tas);
  }
}
