#include <vstypes.h>
#include <mutex.h>
#include <timers.h>
#include <stdlib.h>


auto void InitMutexN (register __i3 u_int16 *mutexVar, register __d1 u_int16 initial) {
  *mutexVar = initial;
}

auto void ObtainMutex(register __i3 u_int16 *mutexVar) {
#if 1
  int dTime = 0;
  while (AttemptMutex(mutexVar)) {
    /* Make dTime grow, until the delay is pretty long: then clear it, making
       it look a bit pseudo-random. That way different clashes shouldn't
       easily make a situation where a task never gets a popular mutex.
       Still, the mutex mechanism is not really meant for popular objects. */
    Delay(++dTime);
    dTime &= 31;
  }
#else
  int dTime = 1;
  while (AttemptMutex(mutexVar)) {
    /* Make dTime grow, until the delay is pretty long: then clear it, making
       it look a bit pseudo-random. That way different clashes shouldn't
       easily make a situation where a task never gets a popular mutex.
       Still, the mutex mechanism is not really meant for popular objects. */
    Delay(dTime);
    dTime = (random() & 31) + 1;
  }
#endif
}

auto u_int16 AttemptMutex(register __i3 u_int16 *mutexVar) {
  u_int16 res = 1;
  Disable();
  if (*mutexVar) {
    (*mutexVar)--;
    res = 0;
  }
  Enable();
  return res;
}

auto void ReleaseMutex(register __i3 u_int16 *mutexVar) {
  Disable();
  (*mutexVar)++;
  Enable();
}
