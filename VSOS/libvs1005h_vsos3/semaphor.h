/**
   \file semaphor.h Semaphores for resource allocation.
 */
#ifndef	EXEC_SEMAPHORES_H
#define	EXEC_SEMAPHORES_H
#include <vstypes.h>
#include "lists.h"
#include "exec.h"

/****** SignalSemaphore *********************************************/

/** Signal Semaphore data structure */
typedef struct SIGNALSEMAPHORE {
    struct NODE ss_Node;      /**< Exec linking, ln_Type=NT_SEMAPHORE */
    /** Can be used to implement public semaphores' reference counts. */
    u_int16 ss_Public;
    s_int16 ss_NestCount;     /**< Number of locks on this semaphore. */
    s_int16 ss_QueueCount;    /**< Number of tasks in the queue. */
    struct LIST ss_WaitQueue; /**< List for waiters. */
    /** The current owner of the semaphore or NULL for shared lock. */
    __near struct TASK *ss_Owner;
    u_int16 ss_SigRelease;    /**< If non-zero, sent to ss_Owner when
				 another write lock wanted. */
} SIGNALSEMAPHORE;

/**
   Initializes the semaphore structure.
   \param ss Initialized signal semaphore.
 */
__near void InitSemaphore(__near struct SIGNALSEMAPHORE *ss);

/**
   Tries to get an exclusive (write) lock on the semaphore without blocking.
   \param ss Initialized signal semaphore.
   \return 0 for failure.
 */
__near s_int16 AttemptSemaphore(__near struct SIGNALSEMAPHORE *ss);
/**
   Tries to get a shared (read) lock on the semaphore without blocking.
   \param ss Initialized signal semaphore.
   \return 0 for failure.
 */
__near s_int16 AttemptSemaphoreShared(__near struct SIGNALSEMAPHORE *ss);

/**
   Gets an exclusive (write) lock on the semaphore.
   Waits until all readers and writer have released the semaphore.
   \param ss Initialized signal semaphore.
   \note The same task can get multiple exclusive locks on the same semaphore.
*/
__near void ObtainSemaphore(__near struct SIGNALSEMAPHORE *ss);
/**
   Gets a shared (read) lock on the semaphore.
   Waits until writer has released the semaphore.
   \param ss Initialized signal semaphore.
*/
__near void ObtainSemaphoreShared(__near struct SIGNALSEMAPHORE *ss);
/*
   Releases an obtained lock on the semaphore, whether shared or exclusive.
   When a semaphore is released other tasks waiting on the semaphore can
   gain access.
   \param ss Initialized signal semaphore.
 */
__near void ReleaseSemaphore(__near struct SIGNALSEMAPHORE *ss);

#endif
