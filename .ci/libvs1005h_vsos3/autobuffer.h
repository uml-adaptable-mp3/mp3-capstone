#ifndef AUTO_BUFFER_H
#define AUTO_BUFFER_H

#include <vstypes.h>
#include <vo_stdio.h>
#include <vsos.h>

#define MAX_AUTO_BUFFERS 2

#define AUTO_BUF_TMP_BUF_WORDS 512

struct AutoBufferElement {
  VO_FILE *origFp;	/* Original file pointer, modified content */
  u_int16 __mem_y *buf;		/* Ring buffer */
  u_int16 bufBytes;	/* Ring buffer size in 8-bit bytes */
  u_int16 rdPtr;	/* Ring buffer byte read pointer */
  u_int16 wrPtr;	/* Ring buffer write read pointer */
  s_int16 bytesInBuffer;/* Bytes in ring buffer */
  u_int32 ioBlocked;	/* Blocked-by-under/overflow counter */
  s_int16 resync;	/* Request for AutoBuffer restart */
  u_int32 oldPos;	/* Position before last Read() */
  u_int16 debug;	/* Debug field */
  struct TASK *mainTask;/* Pointer to main task */
  VO_FILE fpCopy;	/* Copy of original file pointer */
};

#define SIGF_AUTO_BUFFER_READY (1<<1)

struct AutoBuffer {
  struct TaskAndStack *taskAndStack;
  u_int16 n;
  struct AutoBufferElement abElem[MAX_AUTO_BUFFERS];
  u_int16 tmpBuf[AUTO_BUF_TMP_BUF_WORDS];
};

extern struct AutoBuffer autoBuffer;

struct AutoBufferElement *BindAutoBuffer(register FILE *fp,
					 register u_int16 bufWords);
struct AutoBufferElement *FindAutoBufferElement(register __i0 VO_FILE *fp);

#endif /* !AUTO_BUFFER_H */
