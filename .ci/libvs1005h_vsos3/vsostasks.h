#ifndef VSOS_TASKS_H
#define VSOS_TASKS_H

#include <exec.h>


#ifdef NEW_LAYOUT

#define SYS_TASKS 5

#define TASK_MAIN 0
#define TASK_UI 1
#define TASK_DECODER 2
#define TASK_NETWORK 3
#define TASK_INTERRUPT 4	// Not a real task but requires stack space


#else

#define SYS_TASKS 5

#define TASK_IO 0
#define TASK_INTERRUPT 1	// Not a real task but requires stack space
#define TASK_NETWORK 2
#define TASK_UI 3
#define TASK_DECODER 4



#endif


#ifndef ASM
struct SysTask {
  struct TASK task;
  const char *name;
  int stackSize;
  int priority;
};

int StartTask(int taskId, void(*func)(void));
#endif

#endif /* VSOS_TASKS_H */
