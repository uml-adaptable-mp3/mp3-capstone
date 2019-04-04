/**
   \file timers.h Software timers.
 */
#ifndef TIMERS_H
#define TIMERS_H
#include <vstypes.h>
#include "lists.h"
/*#include <exec.h>*/

struct TIMER {
    /** For exec linking. */
    struct MINNODE tm_Node;
    u_int16 tm_Flags;   /** Start/stop flags. */
    u_int16 tm_Count;   /** The timer counter. */
    u_int16 tm_Load;    /** The reload time for circular timer or 0. */
    __near struct TASK *tm_Task;      /** Task to signal or NULL. */
    u_int16 tm_SigMask; /** Signal(s) to send */
};


/**
   Initializes a software timer. Does not add it to the system list.

   \param timer Pre-allocated timer structure.
   \param initial Timer initial interval.
   \param looptime Timer next interval or 0 for a single-shot timer.
   \param task Task to be signaled.
   \param signalmask Signals to be sent.
 */
__near void InitTimer( __near struct TIMER *timer, u_int16 initial,
		       u_int16 loop_time, __near struct TASK *task,
		       u_int16 signal_mask );
/**
   Adds a software timer to the system software timer list.
   Must be called with task switching disabled (in Forbid() state).

   \param timer Timer to add to the system.
*/
__near void AddTimer( __near struct TIMER * register __i0 timer );
/**
   Removes a software timer from the system software timer list.
   Must be called with task switching disabled (in Forbid() state).

   \param timer Timer to remove from the system.
*/
__near void RemoveTimer( __near struct TIMER * register __i0 timer );
/**
   Enables software timer counting. The timer must be in the system
   software timer list.

   \param Timer to enable.
 */
__near void StartTimer( __near struct TIMER *timer );
/**
   Disables software timer counting. Counting can be continued with
   StartTimer().

   \param Timer to disable.
 */
__near void StopTimer( __near struct TIMER *timer );

/** The signal used for software timer by Delay(). */
#define SIGF_DELAY (1<<15)
/** The number of HandleTimer() calls per second. */
#define TICKS_PER_SEC 1000
/**
   Easy-to-use delay function that does not busy-loop.
   \param ticks The number of system ticks to sleep.

   \note Delay() uses the signal mask SIGF_DELAY for waiting.
 */
__near void Delay( u_int16 ticks ); /* 1/100th seconds */

#endif /* TIMERS_H */
