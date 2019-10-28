#include <vstypes.h>
#include <stdarg.h>
#include <vsos.h>
#include <vo_stdio.h>

int PrintFmt(const char *fmt, __near va_list ap,
             __near void *fp, int (*cf)(int, __near void *));


static int SPutc(int c, __near void *strPtr) {
    *(*(char **)strPtr)++ = c;
    return 1;
}

int sprintf(char *str, const char *fmt, ...) {
    char *strPtr = str;
    __near va_list ap;
    int r;
    va_start(ap, fmt);
    r = PrintFmt(fmt, ap, &strPtr, SPutc);
    *strPtr = '\0';
    return r;
}


