// Import kernel symbols for mutex

#ifndef KERNELMUTEX_H
#define KERNELMUTEX_H

#include <mutex.h>
#include <volink.h>

DLLIMPORT(InitMutexN)
DLLIMPORT(ObtainMutex)
DLLIMPORT(AttemptMutex)
DLLIMPORT(ReleaseMutex)

#endif