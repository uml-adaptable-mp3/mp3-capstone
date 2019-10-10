#include <vo_stdio.h>
#include <timers.h>
#include <string.h>
#include <stdlib.h>
#include <sysmemory.h>
#include "taskandstack.h"
#include "autobuffer.h"
#include "string2.h"

/*
  AutoBuffer v0.02
  VLSI Solution 2014-09-25 HH

  This code creates an automatic Input or Output ring buffer for a file.
  It is intended to be used when playing/recording media files from/to a
  memory device that may occasionally freeze for a while. Typical examples
  of such memories are SD cards and USB devices. For instance, when
  writing to an SD card, writes are normally very fast (less than one
  millisecond), but occasional writes can take upto half a second. Without
  buffering recording to such a device would not be continuous.

  This code is used by the FILEBUF.DL3 driver.

  This code may be used freely with in any product containing one
  or more ICs by VLSI Solution.

  No guarantee is given for the usability of this code.

 */

/* NOTE! Task priority is very important to get right.
   Main task is at priority 5, and encoder/decoder task
   is at priority 10. The AutoBuffer task may have
   more priority than our main task, but it must have less
   than the encoder task. That way uninterrupted sound is
   guaranteed as well as possible. If priority is 5 or lower,
   the main task must act politely and not hog all CPU. */
#define AUTO_BUFFER_TASK_PRIORITY 8

struct AutoBuffer autoBuffer;

u_int16 AutoBufferBytesUntilSeam(register struct AutoBufferElement *abe,
				 register s_int16 origPtr,
				 register u_int16 bytes) {
  u_int16 untilSeam = abe->bufBytes-origPtr;
  if (bytes > untilSeam) {
    bytes = untilSeam;
  }
  if (bytes > AUTO_BUF_TMP_BUF_WORDS*2) {
    bytes = AUTO_BUF_TMP_BUF_WORDS*2;
  }

  return bytes;
}

u_int16 AddAutoBufferPtr(register struct AutoBufferElement *abe,
			 register u_int16 origPtr,
			 register s_int16 toAdd) {
  if ((origPtr += toAdd) >= abe->bufBytes) {
    origPtr -= abe->bufBytes;
  }
  return origPtr;
}

void AutoBufferTask(void) {
  while (autoBuffer.n) {
    int hadSomethingToDo = 0, i;
    struct AutoBufferElement *abe = &autoBuffer.abElem[0];
    for (i=0; i<MAX_AUTO_BUFFERS; i++) {
      if (abe->origFp) {
	if (abe->fpCopy.flags & __MASK_WRITABLE) {
	  /* Writable */
	  s_int16 canWriteFromBuffer = abe->bytesInBuffer;
	  if (canWriteFromBuffer) {
	    s_int16 toWrite = AutoBufferBytesUntilSeam(abe, abe->rdPtr,
						       canWriteFromBuffer);
	    s_int16 wrote;

	    MemCopyPackedBigEndianYX(autoBuffer.tmpBuf, 0,
				     abe->buf, abe->rdPtr, toWrite);

	    wrote = abe->fpCopy.op->Write(&abe->fpCopy, autoBuffer.tmpBuf, 0,
					  toWrite);
	    abe->rdPtr = AddAutoBufferPtr(abe, abe->rdPtr, wrote);

	    if (wrote == toWrite) {
	      hadSomethingToDo = 1;
	    }
	    Forbid();
	    abe->bytesInBuffer -= wrote;
	    Permit();
	  } else if (abe->resync > 0) {
	    abe->rdPtr = abe->wrPtr = 0;
	    abe->bytesInBuffer = 0;
	    abe->oldPos = abe->fpCopy.pos = abe->origFp->pos;
	    abe->fpCopy.flags &= ~__MASK_EOF;
	    abe->resync = 0;
	    Signal(abe->mainTask, SIGF_AUTO_BUFFER_READY);
	    hadSomethingToDo = 1;
	  } else if (abe->resync < 0) {
	    abe->fpCopy.op->Close(&abe->fpCopy);
	    freey(abe->buf);
	    abe->origFp = NULL;
	    abe->resync = 0;
	    Signal(abe->mainTask, SIGF_AUTO_BUFFER_READY);
	  } else {
	  }
	} else { /* else (!(abe->fpCopy.flags & __MASK_WRITABLE)) */
	  /* Not writable */
	  if (abe->resync > 0) {
	    abe->rdPtr = abe->wrPtr = 0;
	    abe->bytesInBuffer = 0;
	    abe->oldPos = abe->fpCopy.pos = abe->origFp->pos;
	    abe->fpCopy.flags &= ~__MASK_EOF;
	    abe->resync = 0;
	    Signal(abe->mainTask, SIGF_AUTO_BUFFER_READY);
	    hadSomethingToDo = 1;
	  } else if (abe->resync < 0) {
	    abe->fpCopy.op->Close(&abe->fpCopy);
	    freey(abe->buf);
	    abe->origFp = NULL;
	    abe->resync = 0;
	    Signal(abe->mainTask, SIGF_AUTO_BUFFER_READY);
	  } else {
	    u_int16 canPutToBuffer = abe->bufBytes - abe->bytesInBuffer;
	    if (canPutToBuffer) {
	      s_int16 toRead = AutoBufferBytesUntilSeam(abe, abe->wrPtr,
							canPutToBuffer);
	      s_int16 got;

	      got = abe->fpCopy.op->Read(&abe->fpCopy, autoBuffer.tmpBuf, 0,
					 toRead);
	      MemCopyPackedBigEndianXY(abe->buf, abe->wrPtr,
				       autoBuffer.tmpBuf, 0, toRead);
	      abe->wrPtr = AddAutoBufferPtr(abe, abe->wrPtr, got);
	      if (got == toRead) {
		hadSomethingToDo = 1;
	      }
	      Forbid();
	      abe->bytesInBuffer += got;
	      Permit();
	    } /* if (canPutToBuffer) */
	  } /* if (abe->resync == 0) */
	} /* if (!(abe->fpCopy.flags & __MASK_WRITABLE))*/
      }  /* if (abe->origFp) */
      abe++;
    } /* for (i=0; i<MAX_AUTO_BUFFERS; i++) */

    if (!hadSomethingToDo) {
      Wait(SIGF_AUTO_BUFFER_READY);
    }
  } /* while (autoBuffer.n) */
}

struct AutoBufferElement *FindAutoBufferElement(register __i0 VO_FILE *fp) {
  int i = 0;
  struct AutoBufferElement *abe = &autoBuffer.abElem[0];
  do {
    if (abe->origFp == fp) {
      return abe;
    }
    abe++;
  } while (++i < MAX_AUTO_BUFFERS);
  return NULL;
}

ioresult AutoBufferClose(register __i0 VO_FILE *self) {
  struct AutoBufferElement *abe = FindAutoBufferElement(self);
  if (!abe) {
    return S_ERROR;
  }
  abe->mainTask = thisTask;
  abe->resync = -1;
  Signal(&autoBuffer.taskAndStack->task, SIGF_AUTO_BUFFER_READY);
  while (abe->resync) {
    Wait(SIGF_AUTO_BUFFER_READY);
  }

  if (!(--autoBuffer.n)) {
    /* No autobuffers left, shut down autobuffer process and release
       resources. */
    while (autoBuffer.taskAndStack->task.tc_State &&
	   autoBuffer.taskAndStack->task.tc_State != TS_REMOVED) {
      Signal(&autoBuffer.taskAndStack->task, SIGF_AUTO_BUFFER_READY);
      Delay(1);
    }

    FreeTaskAndStack(autoBuffer.taskAndStack);
    autoBuffer.taskAndStack = NULL;
  }

  return S_OK;
}

IOCTL_RESULT AutoBufferIoctl(register __i0 VO_FILE *self, s_int16 request,
			     IOCTL_ARGUMENT arg) {
  struct AutoBufferElement *abe = FindAutoBufferElement(self);
  if (!abe) {
    return S_ERROR;
  }
  return ioctl(&abe->fpCopy, request, arg);
}

u_int16 AutoBufferRead(register __i0 VO_FILE *self, void *buf,
			u_int16 destinationIndex, u_int16 bytes) {
  struct AutoBufferElement *abe = FindAutoBufferElement(self);
  u_int16 bytesRead = 0;
  if (!abe || (abe->fpCopy.flags & __MASK_WRITABLE)) {
    /* This shouldn't happen; refuse to co-operate */
    return 0;
  }
  abe->mainTask = thisTask;
  if (abe->oldPos != abe->origFp->pos) {
    /* There has been a seek operation, force resync */
    abe->resync = 1;
    Signal(&autoBuffer.taskAndStack->task, SIGF_AUTO_BUFFER_READY);
    while (abe->resync) {
      Wait(SIGF_AUTO_BUFFER_READY);
    }
  }

  while (bytes) {
    int toRead = bytes;
    while (!abe->bytesInBuffer) {
      if (__F_EOF(&abe->fpCopy)) {
	abe->origFp->flags |= __MASK_EOF;
	goto finally;
      }
      abe->ioBlocked++;
      Signal(&autoBuffer.taskAndStack->task, SIGF_AUTO_BUFFER_READY);
      Delay(1);
    }
    if (abe->bytesInBuffer < toRead) {
      toRead = abe->bytesInBuffer;
    }
    toRead = AutoBufferBytesUntilSeam(abe, abe->rdPtr, toRead);
    MemCopyPackedBigEndianYX(buf, destinationIndex, abe->buf, abe->rdPtr,
			     toRead);
    abe->rdPtr = AddAutoBufferPtr(abe, abe->rdPtr, toRead);
    destinationIndex += toRead;
    bytesRead += toRead;
    bytes -= toRead;
    abe->origFp->pos += toRead;
    abe->oldPos = abe->origFp->pos;
    Forbid();
    abe->bytesInBuffer -= toRead;
    Permit();
    Signal(&autoBuffer.taskAndStack->task, SIGF_AUTO_BUFFER_READY);
  }

 finally:
  return bytesRead;
}

u_int16 AutoBufferWrite(register __i0 VO_FILE *self, void *buf,
			 u_int16 sourceIndex, u_int16 bytes) {
  struct AutoBufferElement *abe = FindAutoBufferElement(self);
  u_int16 bytesWritten = 0;
  if (!abe || !(abe->fpCopy.flags & __MASK_WRITABLE)) {
    /* This shouldn't happen; refuse to co-operate */
    return 0;
  }
  abe->mainTask = thisTask;
  if (abe->oldPos != abe->origFp->pos) {
    /* There has been a seek operation, force resync */
    abe->resync = 1;
    Signal(&autoBuffer.taskAndStack->task, SIGF_AUTO_BUFFER_READY);
    while (abe->resync) {
      Wait(SIGF_AUTO_BUFFER_READY);
    }
  }

  while (bytes) {
    u_int16 toWrite = bytes;
    u_int16 canWrite;
    while (!(canWrite = abe->bufBytes-abe->bytesInBuffer)) {
      if (__F_EOF(&abe->fpCopy)) {
	abe->origFp->flags |= __MASK_EOF;
	goto finally;
      }
      abe->ioBlocked++;
      Signal(&autoBuffer.taskAndStack->task, SIGF_AUTO_BUFFER_READY);
      Delay(1);
    }
    if (canWrite < toWrite) {
      toWrite = canWrite;
    }
    toWrite = AutoBufferBytesUntilSeam(abe, abe->wrPtr, toWrite);
    MemCopyPackedBigEndianXY(abe->buf, abe->wrPtr, buf, sourceIndex, toWrite);
    abe->wrPtr = AddAutoBufferPtr(abe, abe->wrPtr, toWrite);
    sourceIndex += toWrite;
    bytesWritten += toWrite;
    bytes -= toWrite;
    abe->origFp->pos += toWrite;
    abe->oldPos = abe->origFp->pos;
    Forbid();
    abe->bytesInBuffer += toWrite;
    Permit();
    Signal(&autoBuffer.taskAndStack->task, SIGF_AUTO_BUFFER_READY);
  }

 finally:
  return bytesWritten;

}



const FILEOPS autoBufferFileOps = {
  NULL,
  AutoBufferClose,
  AutoBufferIoctl,
  AutoBufferRead,
  AutoBufferWrite
};

struct AutoBufferElement *BindAutoBuffer(register FILE *fp,
					 register u_int16 bufWords) {
  int i;
  struct AutoBufferElement *abe = &autoBuffer.abElem[0];

  for (i=0; i<MAX_AUTO_BUFFERS; i++) {
    Forbid();
    if (!abe->origFp) {
      memset(abe, 0, sizeof(*abe));
      abe->buf = mallocy(bufWords);
      if (abe->buf) {
	abe->origFp = fp;
	memcpy(&abe->fpCopy, fp, sizeof(*fp));
	abe->bufBytes = bufWords*2;
	fp->op = &autoBufferFileOps;
	Permit();

	autoBuffer.n++;
	if (!autoBuffer.taskAndStack) {
	  autoBuffer.taskAndStack =
	    CreateTaskAndStack(AutoBufferTask, "AutoBuffer", 512,
			       AUTO_BUFFER_TASK_PRIORITY);
	  if (!autoBuffer.taskAndStack) {
	    freey(abe->buf);
	    abe->origFp = NULL;
	    autoBuffer.n--;
	    return NULL;
	  }
	}

	return abe;
      }
    }
    Permit();
    abe++;
  }

 finally:
  return NULL;
}
