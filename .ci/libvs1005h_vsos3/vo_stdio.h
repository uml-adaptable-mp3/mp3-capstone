/*
  VCC Stdio package using VSOS console

  Last edited 2015-04-17 Henrik Herranen
  
  History:
  2015-04-17 Added vprintf() and vfprintf() macros
  2012-10-15 First release
  
 */

#ifndef _VO_STDIO_H_
#define _VO_STDIO_H_

#include <stddef.h>
#include <stdio.h>
#include <vstypes.h>
#include <stdarg.h>

#define fpos_t vo_fpos_t
typedef u_int32 vo_fpos_t;

#define fopen vo_fopen
#define freopen ERROR_FREOPEN_NOT_IMPLEMENTED
#define fflush vo_fflush
#define fclose vo_fclose
#define remove ERROR_REMOVE_NOT_IMPLEMENTED
#define rename ERROR_RENAME_NOT_IMPLEMENTED
#define tmpfile ERROR_TMPFILE_NOT_IMPLEMENTED
#define tmpnam ERROR_TMPNAM_NOT_IMPLEMENTED
#define setvbuf ERROR_SETVBUF_NOT_IMPLEMENTED
#define setbuf ERROR_SETBUF_NOT_IMPLEMENTED
#define fprintf vo_fprintf
#define printf vo_printf
#define vprintf(fmt, args) PrintFmt((fmt), (args), vo_stdout, (int(*)(int, __near void *))vo_fputc)
#define vfprintf(fp, fmt, args) PrintFmt((fmt), (args), (fp), (int(*)(int, __near void *))vo_fputc)
/* sprintf is in stdio.h */
#define fscanf ERROR_FSCANF_NOT_IMPLEMENTED
#define scanf ERROR_SCANF_NOT_IMPLEMENTED
#define fgetc vo_fgetc
#define fgets vo_fgets
#define fputc vo_fputc
#define fputs vo_fputs
#define getc vo_getc
#define getchar vo_getchar
#define gets vo_gets
#define putc vo_putc
#define putchar(c) (vo_fputc((c),vo_stdout))
#define puts vo_puts
#define ungetc vo_ungetc
#define fread vo_fread
#define fwrite vo_fwrite
#define fseek vo_fseek
#define ftell vo_ftell
#define rewind(fp) (fseek((fp),0L,SEEK_SET); clearerr(fp))
#define fgetpos(fp, p) ((*(p) = (fp)->pos),0)
#define fsetpos(fp, p) (((fp)->pos = *(p)),0)
#define clearerr(fp) ((fp)->flags &= ~(__MASK_EOF | __MASK_ERROR))
#define feof vo_feof
#define ferror vo_ferror
#define perror ERROR_PERROR_NOT_IMPLEMENTED

#define FILE VO_FILE
#ifdef stdin
#undef stdin
#endif
#ifdef stdout
#undef stdout
#endif
#ifdef stderr
#undef stderr
#endif

#define stderr vo_stderr
#define stdout vo_stdout
#define stdin vo_stdin

/// Used internally by printf.
int PrintFmt(const char *fmt, __near va_list ap, __near void *fp, int (*cf)(int, __near void *));

#include "vsos.h"

#endif /* _VO_STDIO_H_ */
