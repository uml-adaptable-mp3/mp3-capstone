/**
   \file handler.h File system layer 4: Handler layer.
   This is the part of the file system that is usually called "the
   file system".
 */

#ifndef HANDLER_H
#define HANDLER_H

#include <vstypes.h>

/**
   Current version number. 8 MSBs contain version number, 8 LSBs size of
   the structure in words.

   Version number history:
   <OL>
	<LI>0x0108
   </OL>
*/
#define FS_HANDLER_VERSION 0x010A

struct FsMapper;

/** The file handler type */
enum HandlerType {
  FS_FH_NULL,
  FS_FH_FAT
};

/**
   Basic structure of a handler. Each handler's internal structure
   should begin with this basic structure, and it should also keep
   these numbers updated.
 */
struct FsHandler {
  /** Version number. 8 MSBs contain version number, 8 LSBs size of
      the structure in words. */
  u_int16 version;
  /** The type of the handler */
  enum HandlerType type;
  /** Current number of open files */
  s_int16 openFiles;
  /** Maximum number of open files */
  s_int16 maxOpenFiles;
  /** Current number of open files */
  s_int16 openLocks;
  /** Maximum number of open files */
  s_int16 maxOpenLocks;
  /** Block size of the file system */
  s_int16 blockSize;
  /** Pointer to this Handler's Mapper layer */
  struct FsMapper *mapper;
};

#endif /* !HANDLER_H */
