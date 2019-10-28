#ifndef CONSOLEOPS_H
#define CONSOLEOPS_H

#include "vsos.h"

IOCTL_RESULT ConsoleIoctl(register __i0 VO_FILE *self, s_int16 request, IOCTL_ARGUMENT arg);

#endif