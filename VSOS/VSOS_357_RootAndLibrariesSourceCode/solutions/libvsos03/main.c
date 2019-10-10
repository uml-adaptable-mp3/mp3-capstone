#include <vstypes.h>
#include <stdarg.h>
#include <vsos.h>
#include <vo_stdio.h>

int PrintFmt(const char *fmt, __near va_list ap,
             __near void *fp, int (*cf)(int, __near void *));

int vo_fprintf(FILE *fp, const char *fmt, ...) {
    __near va_list ap;
    va_start(ap, fmt);
#pragma msg 30 off //vo_fputc triggers pointer to a different object warning
    return PrintFmt(fmt, ap, (__near void *)fp, vo_fputc);
#pragma msg 30 on 
}

int vo_printf(const char *fmt, ...) {
	__near va_list ap;
	va_start(ap, fmt);
	if (!vo_stdout) {
		return 0;
	}
#pragma msg 30 off //vo_fputc triggers pointer to a different object warning
	return PrintFmt(fmt, ap, vo_stdout, vo_fputc);
#pragma msg 30 on 
}

