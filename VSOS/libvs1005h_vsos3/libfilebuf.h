#ifndef LIB_FILE_BUF_H
#define LIB_FILE_BUF_H

#include <vo_stdio.h>
#include <vsos.h>

struct AutoBufferElement;

/* Returns struct AutoBuffer * ok, NULL for error */
#define CreateFileBuf(lib, fp, bufWords) ( ((struct AutoBufferElement * (*)(register FILE *, register u_int16))(*((u_int16 *)(lib)+2+(ENTRY_1))))((fp), (bufWords)) )

#endif /* !LIB_FILE_BUF_H */
