#ifndef TASK_AND_STACK_H
#define TASK_AND_STACK_H

#include <vo_stdio.h>
#include <exec.h>

struct TaskAndStack {
  struct TASK task;
  void *stack;
  u_int16 stackSize;
};

struct TaskAndStack *CreateTaskAndStack(register void(*func)(void),
					register const char *name,
					register int stackSize,
					register int priority);
void FreeTaskAndStack(register struct TaskAndStack *taskAndStack);

#endif /* !TASK_AND_STACK_H */
