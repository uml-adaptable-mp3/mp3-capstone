/// \file vsos.h Interface definitions for VsOs operating system.
/// \author Panu-Kristian Poiksalo, VLSI Solution OY

#ifndef VSOS_H
#define VSOS_H

#include <vstypes.h>
#include <vo_stdio.h>
#include <timers.h>


//#define IOCTL_HAS_32_BITS
#define IOCTL_RESULT s_int16
#define IOCTL_ARGUMENT char*

/*
#define IOCTL_HAS_32_BITS
#define IOCTL_RESULT s_int32
#define IOCTL_ARGUMENT s_int32
*/

#define debug_puts puts
#define debug_printf printf

// Define common IOCTL requests
#define IOCTL_START_FRAME 1
#define IOCTL_END_FRAME 2
#define IOCTL_WAIT_UNTIL_TX_IDLE 3
#define IOCTL_TEST 4 
#define IOCTL_RESTART 5
#define IOCTL_GET_GEOMETRY 6
#define IOCTL_FLUSH 7

typedef struct {
	u_int16 sectorsPerBlock; //for 512-byte blocks this is 1. For 4K blocks this is 8.
	u_int32 totalSectors; //number of 512-byte sectors in the disk. For a 8 MB disk this is 16384 regardless of sectorsPerBlock
} DiskGeometry;



#define IOCTL_OPEN_DEVICE_FILE 106
#define IOCTL_CLOSE_DEVICE_FILE 107



typedef s_int16 ioresult;
typedef u_int16 vo_flags;
struct file_operations;
struct directory_operations;
struct filesystem_descriptor;
struct device_descriptor;
//struct device2_descriptor;
struct file_descriptor;

/*
typedef struct file_operations FILEOPS;
typedef struct directory_operations DIROPS;
typedef struct device_operations DEVOPS;
typedef struct filesystem_descriptor FILESYSTEM;
typedef struct device_descriptor DEVICE;
typedef struct file_descriptor VO_FILE;
typedef struct directory_descriptor DIRECTORY;
*/

#define FILEOPS struct file_operations
#define DIROPS struct directory_operations
#define DEVOPS struct device_operations
#define FILESYSTEM struct filesystem_descriptor
#define DEVICE struct device_descriptor
#define VO_FILE struct file_descriptor
#define SIMPLE_FILE struct simple_file_descriptor
#define DIRECTORY struct directory_descriptor
#define TIMESTRUCT struct time_struct

#ifndef EOF
#define EOF -1
#endif

#ifndef S_OK
#define S_OK 0
#endif

#ifndef S_ERROR
#define S_ERROR -1
#endif

#define E_FILE_NOT_FOUND -2

#define S_UNKNOWN_MESSAGE -3
#define E_UNKNOWN_MESSAGE -3

#define E_ILLEGAL_FILE_NAME -4
#define E_FILE_ALREADY_EXISTS -5
#define E_CANNOT_CREATE_FILE -6
#define E_CANNOT_SET_DIRECTORY -7

// FAT errors
#define E_DEVICE_NOT_SEEKABLE -10
#define E_MBR_NOT_FOUND -11
#define E_BPB_NOT_FOUND -12

// SD card errors
#define E_TIMEOUT 2
#define E_CARD_NOT_FOUND 200
#define E_CANNOT_IDENTIFY_CARD 201
#define E_CANNOT_ENUMERATE_CARD 202

// Miscellaneous errors
#define E_OUT_OF_MEMORY -100





/// This file or device is initialized and in use.
#define __BIT_PRESENT 0

/// This file or device is open.
#define __BIT_OPEN 1

/// End of File.
#define __BIT_EOF 2

/// This file or device is in error condition.
#define __BIT_ERROR 3

/// This file or device is seekable; Block devices must have this bit as 1.
#define __BIT_SEEKABLE 4

/// This file or device is readable. 
#define __BIT_READABLE 5

/// This file or device is writable.
#define __BIT_WRITABLE 6

/// This file or device has unwritten data in its buffer.
#define __BIT_DIRTY 7

/// For files: This file has a character in its ungetc buffer.
/// for devices: This bit has no meaning and can be used by the driver for whatever purpose.
#define __BIT_UNGETC 8

/// For devices: Writes to this device should return as soon as possible, 
/// even before the write is completed. 
#define __BIT_OVERLAPPED 9

/// This object is a character device and StartFileSystem can associate DeviceFS with it.
#define __BIT_CHARACTER_DEVICE 10

/// This object is a file.
#define __BIT_FILE 11

/// This bit is free for device drivers and custom filesystems to use as it likes.
#define __BIT_APPLICATION_SPECIFIC 12


#define __MASK_PRESENT (1 << __BIT_PRESENT)
#define __MASK_OPEN (1 << __BIT_OPEN)
#define __MASK_EOF (1 << __BIT_EOF)
#define __MASK_ERROR (1 << __BIT_ERROR)
#define __MASK_SEEKABLE (1 << __BIT_SEEKABLE)
#define __MASK_READABLE (1 << __BIT_READABLE)
#define __MASK_WRITABLE (1 << __BIT_WRITABLE)
#define __MASK_DIRTY (1 << __BIT_DIRTY)
#define __MASK_UNGETC (1 << __BIT_UNGETC)
#define __MASK_OVERLAPPED (1 << __BIT_OVERLAPPED)
#define __MASK_CHARACTER_DEVICE (1 << __BIT_CHARACTER_DEVICE)
#define __MASK_FILE (1 << __BIT_FILE)

#define __F_PRESENT(f)((f)->flags & __MASK_PRESENT)
#define __F_OPEN(f)((f)->flags & __MASK_OPEN)
#define __F_EOF(f)((f)->flags & __MASK_EOF)
#define __F_ERROR(f)((f)->flags & __MASK_ERROR)
#define __F_SEEKABLE(f)((f)->flags & __MASK_SEEKABLE)
#define __F_READABLE(f)((f)->flags & __MASK_READABLE)
#define __F_WRITABLE(f)((f)->flags & __MASK_WRITABLE)
#define __F_DIRTY(f)((f)->flags & __MASK_DIRTY)
#define __F_UNGETC(f)((f)->flags & __MASK_UNGETC)
#define __F_OVERLAPPED(f)((f)->flags & __MASK_OVERLAPPED)
#define __F_CHARACTER_DEVICE(f)((f)->flags & __MASK_CHARACTER_DEVICE)
#define __F_FILE(f)((f)->flags & __MASK_FILE)

#define __F_CAN_READ(f)  (((f)->flags & (__MASK_PRESENT | __MASK_OPEN | __MASK_ERROR | __MASK_READABLE | __MASK_EOF)) == (__MASK_PRESENT | __MASK_OPEN | __MASK_READABLE))
#define __F_CAN_WRITE(f) (((f)->flags & (__MASK_PRESENT | __MASK_OPEN | __MASK_ERROR | __MASK_WRITABLE | __MASK_EOF)) == (__MASK_PRESENT | __MASK_OPEN | __MASK_WRITABLE))
#define __F_CAN_SEEK_AND_READ(f)  (((f)->flags & (__MASK_PRESENT | __MASK_OPEN | __MASK_ERROR |__MASK_SEEKABLE | __MASK_READABLE | __MASK_EOF)) == (__MASK_PRESENT | __MASK_OPEN | __MASK_SEEKABLE | __MASK_READABLE))
#define __F_CAN_SEEK_AND_WRITE(f) (((f)->flags & (__MASK_PRESENT | __MASK_OPEN | __MASK_ERROR |__MASK_SEEKABLE | __MASK_WRITABLE | __MASK_EOF)) == (__MASK_PRESENT | __MASK_OPEN | __MASK_SEEKABLE | __MASK_WRITABLE))
#define __F_CAN_SEEK(f)  (((f)->flags & (__MASK_PRESENT | __MASK_OPEN | __MASK_ERROR | __MASK_SEEKABLE)) == (__MASK_PRESENT | __MASK_OPEN | __MASK_SEEKABLE))
#define __F_CAN_UNGETC(f)(((f)->flags & (__MASK_PRESENT | __MASK_OPEN | __MASK_ERROR | __MASK_READABLE | __MASK_UNGETC)) == (__MASK_PRESENT | __MASK_OPEN | __MASK_READABLE))

struct file_operations {
	ioresult (*Open) (register __i0 VO_FILE *self, const char *name, const char *mode); ///< Find and open a file
	ioresult (*Close)(register __i0 VO_FILE *self); ///< Flush and close file
	IOCTL_RESULT (*Ioctl)(register __i0 VO_FILE *self, s_int16 request, IOCTL_ARGUMENT arg); ///< For extensions, used e.g. for special operations on device files.
	u_int16  (*Read) (register __i0 VO_FILE *self, void *buf, u_int16 destinationIndex, u_int16 bytes); ///< Read bytes to *buf, at byte index
	u_int16  (*Write)(register __i0 VO_FILE *self, void *buf, u_int16 sourceIndex, u_int16 bytes); ///< Write bytes from *buf at byte index
};

//Note: nobody yet implements any directory operations, this is for future reference and may change.
struct directory_operations {
	ioresult (*FindFile) (register __i0 const DIRECTORY *startDir, VO_FILE *f, const char *name); ///< Find one file from a directory
	char*    (*Identify) (register __i0 DIRECTORY *dir, char *buf, u_int16 bufsize); ///< Return some kind of decorative name for this object.
	ioresult (*FindFirst)(register __i0 DIRECTORY *dir, char *name, void *searchRec);
	ioresult (*FindNext) (register __i0 DIRECTORY *dir, void *searchRec);
	ioresult (*ChDir)    (register __i0 DIRECTORY *dir, const char *name);
	ioresult (*MkDir)    (register __i0 DIRECTORY *dir, const char *name);	
	ioresult (*RmDir)    (register __i0 DIRECTORY *dir, const char *name);
};

struct filesystem_descriptor {
	vo_flags flags; ///< VSOS Flags
	char*    (*Identify)(register __i0 void *self, char *buf, u_int16 bufsize); ///< Return some kind of decorative name for this object.
	ioresult (*Create)(register __i0 DEVICE *self, const char *name, u_int16 *sectorBuffer); ///< Try to start the filesystem for device
	ioresult (*Delete)(register __i0 DEVICE *self); ///< Flush and stop the filesystem, free the descriptor
	IOCTL_RESULT (*Ioctl) (register __i0 DEVICE *self, s_int16 request, IOCTL_ARGUMENT arg);///< For generic extensions
	FILEOPS  *op;	///< Opened file inherits this set of file operations.
	// Filesystem object does not have any state information of it's own.
	// Its info is stored in device and file descriptors.
};


struct device_descriptor {
	vo_flags   flags; ///< VSOS Flags
	const char*    (*Identify)(register __i0 void *self, char *buf, u_int16 bufsize); ///< Return some kind of decorative name for this object.
	ioresult (*Create)(register __i0 DEVICE *self, const void *name, u_int16 extraInfo); ///< Start device, populate descriptor, find and start filesystem.
	ioresult (*Delete)(register __i0 DEVICE *self); ///< Flush, Stop, Clean and Free the descriptor of the device
	IOCTL_RESULT(*Ioctl)(register __i0 DEVICE *self, s_int16 request, IOCTL_ARGUMENT arg); ///< Reset, Start, Stop, Restart, Flush, Check Media etc
	u_int16  (*Read) (register __i0 DEVICE *self, void *buf, u_int16 destinationIndex, u_int16 bytes); ///< Read bytes to *buf, at byte index
	u_int16  (*Write)(register __i0 DEVICE *self, void *buf, u_int16 sourceIndex, u_int16 bytes); ///< Write bytes from *buf at byte index
	ioresult (*BlockRead)(register __i0 DEVICE *self, u_int32 firstBlock, u_int16 blocks, u_int16 *data); ///< Read block at LBA
	ioresult (*BlockWrite)(register __i0 DEVICE *self, u_int32 firstBlock, u_int16 blocks, u_int16 *data); ///< Write block at LBA
	FILESYSTEM *fs; ///< pointer to the filesystem driver for this device
	u_int16  deviceInstance; ///< identifier to detect change of SD card etc
	u_int16  hardwareInfo[6]; ///< Device driver's info of this hardware
	u_int16  deviceInfo[16]; ///< Filesystem driver's info of this device
};



struct file_descriptor {
	vo_flags flags; ///< VSOS Flags
	char*   (*Identify)(register __i0 void *self, char *buf, u_int16 bufsize); ///< Return some kind of decorative name for this object.
	FILEOPS *op; ///< pointer to file operations (inherited from filesystem)
	u_int32 pos; ///< current byte position
	u_int16 ungetc_buffer; ///< for ungetc
	DEVICE *dev;	///< pointer to device descriptor, custom file info or pointer to custom file info
	DIROPS *dirop;
	u_int16 *sectorBuffer; ///< for block devices, pointer to memory where the filesystem can store one disk block of data
	void *extraInfo; ///< For future extensions (to allow extra info about file buffers)
	u_int32 currentSector; ///< the sector number, which the sectorBuffer currently holds
	u_int32 fileSize; ///< size of a seekable file
	u_int16 deviceInstance;
	u_int16 fileInfo[11]; // For filesystem use, size TBD. FAT needs 11 words.
	void *task;
};

struct simple_file_descriptor {
	vo_flags flags; ///< VSOS Flags
	char*   (*Identify)(register __i0 void *self, char *buf, u_int16 bufsize); ///< Return some kind of decorative name for this object.
	FILEOPS *op; ///< pointer to file operations (inherited from filesystem)
	u_int32 pos; ///< current byte position
	u_int16 ungetc_buffer; ///< for ungetc
	DEVICE *dev;	///< pointer to device descriptor, custom file info or pointer to custom file info
};


struct time_struct {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	// below items are for C compatibility extensions and currently not used
	int tm_wday;
	int tm_yday;
	int tm_isdst;
};



/* C File operations */	
int vo_fgetc(VO_FILE *stream);
/**< Gets the next character (an unsigned char) from the specified stream and advances the position indicator for the stream.	
On success the character is returned. If the end-of-file is encountered, then EOF is returned and the end-of-file indicator is set. If an error occurs then the error indicator for the stream is set and EOF is returned. */

char *vo_fgets(char *str, int n, VO_FILE *stream);
/**< Reads a line from the specified stream and stores it into the string pointed to by str. It stops when either (n-1) characters are read, the newline character is read, or the end-of-file is reached, whichever comes first. The newline character is copied to the string. A null character is appended to the end of the string.
On success a pointer to the string is returned. On error a null pointer is returned. If the end-of-file occurs before any characters have been read, the string remains unchanged. */

int vo_fputc(int ch, VO_FILE *stream);
/**< Writes a character (an unsigned char) specified by the argument char to the specified stream and advances the position indicator for the stream.
On success the character is returned. If an error occurs, the error indicator for the stream is set and EOF is returned. */

int vo_fputs(const char *str, VO_FILE *stream);
/**< Writes a string to the specified stream up to but not including the null character.
On success a nonnegative value is returned. On error EOF is returned. */

int vo_getc(VO_FILE *stream);
/**< Gets the next character (an unsigned char) from the specified stream and advances the position indicator for the stream.
This may be a macro version of fgetc.
On success the character is returned. If the end-of-file is encountered, then EOF is returned and the end-of-file indicator is set. If an error occurs then the error indicator for the stream is set and EOF is returned. */

int vo_getchar(void);
/**< Gets a character (an unsigned char) from stdin.
On success the character is returned. If the end-of-file is encountered, then EOF is returned and the end-of-file indicator is set. If an error occurs then the error indicator for the stream is set and EOF is returned. */

char *vo_gets(char *str);
/**< Reads a line from stdin and stores it into the string pointed to by str. It stops when either the newline character is read or when the end-of-file is reached, whichever comes first. The newline character is not copied to the string. A null character is appended to the end of the string.
On success a pointer to the string is returned. On error a null pointer is returned. If the end-of-file occurs before any characters have been read, the string remains unchanged. */

int vo_putc(int ch, VO_FILE *stream);
/**< Writes a character (an unsigned char) specified by the argument char to the specified stream and advances the position indicator for the stream.
This may be a macro version of fputc.
On success the character is returned. If an error occurs, the error indicator for the stream is set and EOF is returned. */

int vo_putchar(int ch);
/**< Writes a character (an unsigned char) specified by the argument char to stdout.
On success the character is returned. If an error occurs, the error indicator for the stream is set and EOF is returned. */
	
//int vo_puts(const char *str);
/**< Writes a string to stdout up to but not including the null character. A newline character is appended to the output.
On success a nonnegative value is returned. On error EOF is returned. */

int vo_ungetc(int ch, VO_FILE *stream);
/**< Pushes the character char (an unsigned char) onto the specified stream so that the this is the next character read. The functions fseek, fsetpos, and rewind discard any characters pushed onto the stream.
Multiple characters pushed onto the stream are read in a FIFO manner (first in, first out).
On success the character pushed is returned. On error EOF is returned. */


auto IOCTL_RESULT ioctl(register void *p, register int request, register IOCTL_ARGUMENT arg);
auto int vo_puts(register const char *s);



VO_FILE *vo_fopen(const char *filename, const char *mode);
u_int16 vo_fread(void *ptr, u_int16 size, u_int16 nobj, VO_FILE *stream);
ioresult vo_fseek(VO_FILE *stream, s_int32 offset, s_int16 origin);
u_int16 vo_fwrite(const void *ptr, u_int16 size, u_int16 nobj, VO_FILE *stream);
u_int32 vo_ftell(VO_FILE *stream);
ioresult vo_fflush(VO_FILE *stream);
ioresult vo_fclose(register __i0 VO_FILE *stream);
ioresult SysError(const char *errorMsg, ...);
ioresult SysReport(const char *msg, ...);
/// Start the OS, using supplied memory for buffers
ioresult vo_StartOS(u_int16 *osMemPtr, u_int16 osMemSizeWords);
ioresult vo_kernel_init();// was vo_StartOS. From v3.07, buffers are allocated dynamically
ioresult vo_feof(VO_FILE *stream);
ioresult vo_ferror(VO_FILE *stream);
ioresult CommonOkResultFunction();
void PostBoot(void);
ioresult CommonErrorResultFunction();
auto void AutoVoidNull(void);
FILESYSTEM *StartFileSystem(DEVICE *dev, char *name);
int CallKernelModule(register u_int16 appIdent, int service, void *data);
int SysCall(register const char *sysName, register int service, register void *data);

//extern ioresult lastError;
extern const char *lastErrorMessagePtr;


#define VODEV(a) (vo_pdevices[(a)-'A'])
#define __FOPEN_MAX_FILES 6
extern u_int16 vo_max_num_files;
extern VO_FILE vo_files[/*__FOPEN_MAX_FILES*/]; ///< pool of file descriptors from which vo_fopen returns pointers to
extern DEVICE *vo_pdevices[26]; ///< pointers to system devices A..Z
extern u_int16 __nextDeviceInstance;
extern TIMESTRUCT currentTime;
extern FILESYSTEM *vo_filesystems[];
#ifndef BYTESWAP
#define BYTESWAP(a) (((u_int16)(a)>>8)|((a)<<8))
#endif
#define UART_DISABLE_INTERRUPT() {USEY(INT_ENABLEL) &= ~(INTF_UART_RX);}
#define UART_ENABLE_INTERRUPT() {USEY(INT_ENABLEL) |= (INTF_UART_RX);}
extern const SIMPLE_FILE consoleFile;
extern DEVICE console;

extern void (*vo_sys_error_hook)(char *msg);
extern void (*vo_sys_report_hook)(char *msg);
extern void (*vo_get_time_from_rtc_hook)(void); ///<vo_fclose uses this hook to ask for the current time.

//extern void *sys_error_hook_ptr; ///< api pointer to SysError hook
//extern void *sys_report_hook_ptr; ///< api pointer to SysReport hook
//extern void *get_time_from_rtc_hook_ptr; ///< api pointer to system get real time hook

// struct TouchInfo declared in touch.h
extern struct TouchInfo *pTouchInfo;

int vo_fprintf(VO_FILE *fp, const char *fmt, ...);
int vo_printf(const char *fmt, ...);


// system standard files
extern VO_FILE *vo_stdout;
extern VO_FILE *vo_stdin;
extern VO_FILE *vo_stderr;
extern VO_FILE *appFile;
extern VO_FILE *stdaudioout;
extern VO_FILE *stdaudioin;
extern s_int16 kernelDebugLevel;

int RomFileNameCompare(register const char *filespec, register const char *candidate, register s_int16 n);
int FileNameCompare(register const char *filespec, register const char *candidate, register s_int16 n);

void *SetHookFunction(register __i0 u_int16 hook, register __a0 void *newFunc);
//void ModelCallbackHook(s_int16 index, u_int16 message, u_int32 value);
void *SetHandler(register __i0 void *hook, register __a0 void *newFunc);

void AudioIdleHook(void);

auto u_int16 BiosService(u_int16 service,...);


extern struct SysTask *pSysTasks; ///< pointer to sysTasks array

extern u_int16 *vo_osMemoryStart; ///< pointer to file buffers
extern u_int16 vo_osMemorySize; ///< size of file buffers in total
u_int16 GetI6();
u_int16 GetLR0();
u_int16 GetMR0();
extern u_int16 osVersion;
extern u_int16 fopen_retries;
extern char votemp80[];
extern ioresult voResult;
extern u_int16 voBootState;
extern u_int16 skipNFiles;
extern char *appParameters;

#if 1
#define MODEL_CALLBACKS 4
#define MODEL_CALLBACK_PLAYER 0
#define MODEL_CALLBACK_VIEW 1
#define MODEL_CALLBACK_AUX1 2
#define MODEL_CALLBACK_AUDEC 3
extern void *modelCallbacks[MODEL_CALLBACKS];
#else
#define MODEL_CALLBACKS 5
#define MODEL_CALLBACK_PLAYER 0
#define MODEL_CALLBACK_VIEW 1
#define MODEL_CALLBACK_AUX1 2
#define MODEL_CALLBACK_AUX2 3
#define MODEL_CALLBACK_AUDEC 4
extern void *modelCallbacks[MODEL_CALLBACKS];
#endif

u_int16 GetSymbolAddress(const char *s);
struct ExtSymbol __mem_y *AddSymbol(register const char *name, register void *lib, register u_int16 addr);

void Halt(void);

extern u_int16 appIXYStart[3];

/* GetDivider() returns cpuKHz / speedKHz / 2 - 1, rounded upwards. */
u_int16 GetDivider(register u_int16 maxSpeedKHz, register u_int16 maxVal);
extern u_int16 timeCountAdd;

#ifdef LCC
#define LCC_REGISTER register
#else
#define LCC_REGISTER
#endif


#endif

