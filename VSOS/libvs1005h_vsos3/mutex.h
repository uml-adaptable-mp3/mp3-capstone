/**
   \file mutex.h Simple Mutex allocation using an u_int16 as store.
   \author Henrik Herranen, VLSI Solution OY

   First initialize a mutex with \a InitMutex() or \a InitMutexN().
   To obtain a mutex call \a ObtainMutex(), or
   \a AttemptMutex(), to release it call \a ReleaseMutex().

*/

#ifndef MUTEX_H
#define MUTEX_H

#include <vstypes.h>
#include <exec.h>

#ifndef ASM
/**
   Initializes the Mutex variable.
   Must be called for a new lock before it is used.
   \param mutexVal Pointer to the mutex variable.
*/
#define InitMutex(mutexVar) (InitMutexN((mutexVar),1))

/**
   Initializes the Mutex variable.
   Must be called for a new lock before it is used.
   \param mutexVal Pointer to the mutex variable.
   \param initial How many simultaneous resources may be allocated (1 for
	true mutex)
*/
auto void InitMutexN(register __i3 u_int16 *mutexVar,
		     register __d1 u_int16 initial);


/**
   Obtain a mutex. If the mutex is allocated,
   wait until it's freed. May NOT be called from interrupts.
   \param mutexVal Pointer to the mutex variable.
*/
auto void ObtainMutex(register __i3 u_int16 *mutexVar);

/**
   Attempt to obtain a mutex. If the mutex is already allocated,
   return error code. May be called from interrupts.
   \param mutexVal Pointer to the mutex variable.
   \return 0 on success, non-zero on error.
*/

auto u_int16 AttemptMutex(register __i3 u_int16 *mutexVar);

/**
   Release the mutex allocated earlier with \a ObtainMutex() or
   \a AttemptMutex(). May be called from an interrupt.
   \param mutexVal Pointer to the mutex variable.
*/
auto void ReleaseMutex(register __i3 u_int16 *mutexVar);

#endif /* !ASM */

#endif /*MUTEX_H*/
