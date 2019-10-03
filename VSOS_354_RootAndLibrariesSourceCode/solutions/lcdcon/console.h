#ifndef CONSOLE_H
#define CONSOLE_H

u_int16  ConsoleWrite (register __i0 VO_FILE *self, void *buf, u_int16 sourceIndex, u_int16 bytes);
s_int32 ConsoleIoctl (register __i0 VO_FILE *self, s_int16 request, IOCTL_ARGUMENT arg);
void ConsoleFrame(const char *caption);

#endif