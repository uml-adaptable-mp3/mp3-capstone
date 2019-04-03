/**
   \file dos.h Dos structures.
*/

#ifndef DOS_H
#define DOS_H

/** Basic dos pointer. */
typedef void * APTR;

#include <vstypes.h>
#include <messages.h>
#include <stdio.h>

#define MODE_OLDFILE 1
#define MODE_NEWFILE 2
#define MODE_READWRITE 4

#define DOS_FALSE	0
#define DOS_TRUE	1

enum DosError {
  /** No error */
  deNone,
  /** Read error */ 
  deRead,
  /** Write error */ 
  deWrite,
  /** Directory not found */ 
  deDirNotFound,
  /** Object not found */ 
  deObjectNotFound,
  /** Component contains illegal characters */ 
  deInvalidComponentName,
  /** Invalid lock */ 
  deInvalidLock,
  /** Object is the wrong type for the operation */ 
  deObjectWrongType,
  /** Disk is not in a consistent state */ 
  deDiskNotValidated,
  /** Disk is write protected */ 
  deDiskWriteProtected,
  /** Renaming was attempted across devices */ 
  deRenameAcrossDevices,
  /** Directory is not empty */ 
  deDirectoryNotEmpty,
  /** Too many directory levels */ 
  deTooManyLevels,
  /** Seek couldn't be performed */ 
  deSeekError,
  /** Disk is full */ 
  deDiskFull,
  /** Object is delete protected */ 
  deDeleteProtected,
  /** Object is write protected  */ 
  deWriteProtected,
  /** Object is read protected */ 
  deReadProtected,
  /** Object is execute protected */ 
  deExecuteProtected,
  /** No disk in drive */ 
  deNoDisk,
  /** No more entries allowed in a directory */ 
  deNoMoreEntries,
  /** Dos packet is not implemented */ 
  deNotImplemented,
  /** Disk is not in a known format */ 
  deNotADosDisk,
  /** Parent directory is protected */ 
  deParentProtected,
  /** You are not the owner of the file */ 
  deNotOwner,
  /** Too many open files or locks */
  deTooManyOpen,
  /** The operation timed out */
  deTimeOut
};

#define DOS_MODE_RAW 1
#define DOS_MODE_CONSOLE 2
#define DOS_MODE_NOCRLF 4

#define DOS_ACCESS_READ 1
#define DOS_ACCESS_WRITE 2

#define DOS_OFFSET_CURRENT SEEK_CUR
#define DOS_OFFSET_BEGINNING SEEK_SET
#define DOS_OFFSET_END SEEK_END

/**
   Contains message types for struct DosPacket.
   More precise documentation for the
   functions exists in a separate file interfaces.pdf.
   Parameter order is presented along this enumeration. They are bound
   to args in the order arg1, arg2, arg3, arg4.
   Functions that return values do so in the result2 field.
 */
enum DpType {
  dpOpen,	/**< APTR lock, const char *fileName, u_int16 mode */
  dpClose,	/**< APTR handle */
  dpRead,	/**< APTR handle, u_int16 *buffer, u_int16 size */
  dpWrite,	/**< APTR handle, u_int16 *buffer, u_int16 size */
  dpSetMode,	/**< APTR handle, u_int16 mode */
  dpSeek,	/**< APTR handle, u_int16 offHi, u_int16 offLo, u_int16 whence */
  dpDeleteFile,	/**< APTR lock, const char *file */
  dpRename,	/**< APTR lock, const char *newName, const char *oldName */
  dpLock,	/**< APTR lock, const char *name, u_int16 mode */
  dpUnlock,	/**< APTR lock */
  dpDupLock,	/**< APTR oldLock */
  dpSameLock,	/**< APTR lockA, APTR lockB */
  dpExamine,	/**< APTR lock, struct FileInfoBlock *fib */
  dpExNext,	/**< APTR lock, struct FileInfoBlock *fib */
  dpExamineEnd,	/**< APTR lock, struct FileInfoBlock *fib */
  dpCreateDir,	/**< APTR lock, const char *name */
  dpParentDir,	/**< APTR lock */
  dpSetProtection,/**< APTR lock, const char *name, u_int16 protection */
  dpSetOwner,	/**< APTR lock, const char *name, u_int16 userId, u_int16 groupId */
  dpFormat,	/**< const char *disk, u_int16 flags */
  dpDiskInfo,	/**< APTR lock, struct DiskInfoData *info */
  dpSetFileDate,/**< APTR lock, const char *name, const struct DateStamp *date */
  dpIsInteractive,/**< APTR handle */
  dpRelabel,	/**< const char *newname */
  dpWaitForChar	/**< APTR handle, u_int16 time */
};

/**
   Dos packets are sent between the user process and the file handler
   process. Their interpretation depends on the action type that is
   defined for enum DpType.
*/
struct DosPacket {
  struct MESSAGE message;
  enum DpType type;	/**< Name of the action, enum DpType. */

#ifdef __VSDSP__
  u_int16 arg1;		/**< Argument 1 */
  u_int16 arg2;		/**< Argument 2 */
  u_int16 arg3;		/**< Argument 3 */
  u_int16 arg4;		/**< Argument 4 */
#else
  u_int32 arg1;		/**< Argument 1 (VSDSP version is u_int16) */
  u_int32 arg2;		/**< Argument 2 (VSDSP version is u_int16) */
  u_int32 arg3;		/**< Argument 3 (VSDSP version is u_int16) */
  u_int32 arg4;		/**< Argument 4 (VSDSP version is u_int16) */
#endif

#ifdef __VSDSP__
  u_int16 result;	/**< Return value for the action. DOS_TRUE = ok,
				DOS_FALSE = error. */
#else
  u_int32 result;	/**< Return value for the action. DOS_TRUE = ok,
				DOS_FALSE = error. VSDSP version is u_int16. */
#endif
  s_int32 result2;	/**< Secondary result - the reason for failure or
				the result of a successful operation. */
};


/** Returned by Lock() or DupLock() */
struct FileLock {
  struct MINNODE node;	 /** Makes it possible to link in a list */
  u_int32        key;    /** Handler-private (disk block number) */
  u_int16        access; /** exclusive or shared */
  struct MSGPORT *task;  /** handler task's port */
  APTR           volume; /** ptr to DLT_VOLUME DosList entry */
};

#define DOS_MAX_FIB_NAME_CHARS 108

/** File information that can be obtained with FsDosExamine() */
struct FileInfoBlock {
  s_int16 dirEntryType;	/**< Type of Directory. If < 0, then a plain file,
			   if > 0 a directory. */
  char    fileName[DOS_MAX_FIB_NAME_CHARS];
			/**< Null terminated. Max N chars used for now. */
  u_int16 protection;	/**< Bit mask of protection, rwxd are 3-0.      */
  u_int32 size;		/**< Number of bytes in file. */
  u_int32 date;		/**< Date file last changed. */
  u_int16 ownerID;	/**< Owner's UID. */
  u_int16 groupID;	/**< Owner's GID. */

  u_int16 reserved[2];	/**< Reserved for handler internal use */
}; /* FileInfoBlock */


#define DOS_DS_READ 1
#define DOS_DS_WRITE 2

/** Returned by DiskInfo() */
struct DiskInfoData {
   u_int16 diskState;         /**< DOS_DS_READ, DOS_DS_WRITE */
   u_int32 blocks;            /**< Number of blocks on disk */
   u_int32 blocksUsed;        /**< Number of block in use */
   u_int16 bytesPerBlock;     /**< Block size */
   u_int16 diskType;          /**< Disk Type code */
   u_int16 inUse;             /**< Flag, zero if not in use */
}; /* DiskInfoData */


#define DOS_PROTECT_DIRECTORY	128
#define DOS_PROTECT_VOL_ID	 64
#define DOS_PROTECT_SYSTEM	 32
#define DOS_PROTECT_HIDDEN	 16
#define DOS_PROTECT_ARCHIVE	  8
#define DOS_PROTECT_READ	  4
#define DOS_PROTECT_WRITE	  2
#define DOS_PROTECT_EXECUTE	  1

#endif
