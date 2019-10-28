/// This is a deprecated left-over from VSOS2 console device START FRAME ioctl
#include "consoleops.h"
#include <vo_stdio.h>
#include <vsos.h>

IOCTL_RESULT ConsoleIoctl (register __i0 VO_FILE *self, s_int16 request, IOCTL_ARGUMENT arg) {
	if (request == IOCTL_START_FRAME) {
		printf("\n\n-----< %s >-----\n",(char*)arg);
	}
};

