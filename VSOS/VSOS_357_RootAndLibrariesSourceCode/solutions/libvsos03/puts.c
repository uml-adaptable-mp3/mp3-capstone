#include <vo_stdio.h>
#include <vsos.h>
#include <vstypes.h>

auto int vo_puts(register const char *s) {
	if (fputs(s, vo_stdout) == EOF)
	  return EOF;
	return fputc('\n', vo_stdout);
}
