/**
   \file exec.h Multitasking core.
*/
#ifndef EXEC_H
#define EXEC_H

#include <vstypes.h>
#include "lists.h"
#include "messages.h"

#ifndef __VSDSP__
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
//#include <pthread.h>
#endif /*!__VSDSP__*/

/** Disable interrupts. Use with care and only when you absolutely need to!
    Usually Forbid() is enough. Use semaphores for general locking problems. */
__near void Disable( void );
/** Enable interrupts. */
__near void Enable( void );
/** Wait for the next interrupt. For multiple-task applications, use Yield()
    instead (round-robin with equal-priority tasks) or proper signals. */
__near void Halt(void);

/** Sets an interrupt handler routine. Returns the old handler. */
__near void *AddIntServer( register __i0 short vector,
			   register __a0 __near void *is_Code );

/** Performs timesliced round-robin scheduling between tasks with equal
    priorities. ScheduleHander() should be called TICKS_PER_SEC times
    per second.
    ScheduleHandler() also updates the active timers in the software
    timer list and signals the appropriate task if a timer expires.
 */
extern __near short ScheduleHandler(void);

struct TASK {
    struct NODE tc_Node;   /**< For exec linking. */
    s_int16 tc_State;      /**< Task state TS_RUN 2, TS_READY 3, TS_WAIT 4. */
    __near void *tc_SPReg; /**< Stack pointer save. */
    unsigned short tc_SigWait;     /**< Signals we are waiting for. */
    unsigned short tc_SigRecvd;    /**< Signals we have received. */
    __near void *tc_Stack;       /**< Stack start. */
    short tc_StackSize;   /**< Stack size. */
    unsigned short *allocedX; /**< Allocated X memory for automatic free. */
    __y unsigned short *allocedY; /**< Allocated Y memory for automatic free. */
#ifndef __VSDSP__
    pid_t pid;
    //pthread_t thread;
#endif/*!__VSDSP__*/
    /** \todo: accounting/profiling */
};

extern __near struct TASK * __near thisTask;  /**< Active (TS_RUN) task pointer */
extern __near short *__curStack;
extern __near short useQuantum; /**< Quantum used for time slicing. */
/** Sets the stack for the first running task.
    Must be done as the first thing in the main() routine. */
__near void SetupMainStackSize(register __a0 short size);
/** Allocates separate interrupt stack. Otherwise task stacks are used
    for interrupt handlers. If called, must be called after
    SetupMainStackSize() and before other tasks are created. */
__near void SetupInterruptStackSize(register __a0 short size);

/*----- Task States ----------------------------------------*/
#define TS_INVALID      0
#define TS_ADDED        1
#define TS_RUN          2
#define TS_READY        3
#define TS_WAIT         4
#define TS_EXCEPT       5
#define TS_REMOVED      6



/**
   Adds a task to the system. See also CreateTask().
   \param task Already filled task structure.
   \param fun Code to start running.
 */
__near void AddTask( __near register __i2 struct TASK *task,
		     register __a0 __near void *fun );
#if 1
/**
   Allocates stack for a task, initializes the task structure and
   adds the task into the system.

   \param task Pre-allocated task structure.
   \param name Optional task name.
   \param fun Task code to be executed.
   \param priority Task priority
   \param stack_size Size of the task stack to allocate.

   \return 0 for success.
*/
__near s_int16 CreateTask( __near struct TASK *task, __near const char *name,
			   __near void (*fun)( void ),
			   s_int16 priority, s_int16 stack_size);
#endif

/** Sets task priority. Higher is better. Does not cause a reschedule.
    Call Yield() if you want immediate reschedule. */
__near register __a1 s_int16 SetTaskPri(__near register __i0 struct TASK *task,
					register __a0 short priority );

/** Waits for a combination of signals. The signals that caused the wakeup
    are returned and cleared from the received signals. */
__near register __a1 u_int16 Wait(register __a0 u_int16 sigMask);

/** Sends one or a combination of signals to a task.
    Can be called from interrupts! */
__near void Signal(__near register __i0 struct TASK *task,
		   register __a0 u_int16 sigmask );

/** Disables task switching. If the task goes into the wait state
    when inside Forbid(), the forbid state is broken and other tasks
    can run until the original task wakes up and the forbid state is
    restored. */
__near void Forbid( void );
/** Enables task switching */
__near void Permit( void );
/** Causes a reschedule. Because Yield() does not put the task into
    wait state, it causes round-robin scheduling between tasks with the
    same priority. */
__near void Yield( void );

/** Finds a task by name.
    \param name Task name to search for or NULL for the current task.
    \bugs Does not care about the name. Only works with NULL input parameter.
    \todo Implement name search.
 */
__near register __i0 struct TASK *FindTask( __near register __a0 const char *name );

/** Sets or resets signals */
__near register __a0 u_int16 SetSignal( register __b0 u_int16 newsigs,
					register __b1 u_int16 sigmask );

#endif /* EXEC_H */
