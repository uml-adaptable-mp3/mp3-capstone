/// \file vsos.c C interface for VsOs Operating System Framework
/// \author Panu-Kristian Poiksalo, VLSI Solution OY


#include <vo_stdio.h>
#include <string.h>
#include "vsos.h"
#include "vo_fat.h" // FAT Filesystem
#include "devCharFS.h" // Character device filesystem
//#include <usblowlib.h>
#include <kernelservices.h>
//#include "forbid_stdout.h"
#include "apploader.h"

#include <stdarg.h>
#include <sysmemory.h>
#include <mutex.h>
#include <ctype.h>
//#include "power.h"


#if 1
#  define USE_MALLOC_SECTOR_BUFFERS
#endif

#ifdef __FOPEN_MAX_FILES
#undef __FOPEN_MAX_FILES
#endif
#define __FOPEN_MAX_FILES 10
#define __FOPEN_SIMPLE_FILES 6
#define __SECTOR_BUFFERS 8

u_int16 fopen_retries;
u_int16 fopenclose_mutex, fclose_gettime_mutex;
//s_int16 kernelDebugLevel;

//ioresult lastError;
extern const char *lastErrorMessagePtr;
VO_FILE vo_files[__FOPEN_MAX_FILES]; //reserve space for some file desctiptors (is 4 enough?)
extern DEVICE *vo_pdevices[26]; //pointers to system devices A..Z
extern u_int16 *vo_osMemoryStart;
extern u_int16 vo_osMemorySize;
extern u_int16 vo_max_num_files;
extern u_int16 __nextDeviceInstance;
extern TIMESTRUCT currentTime;



// nullFile can be used when a nonzero pointer is needed to a file with no functionality.
// e.g. for reserving a sector buffer for temporary use, filesystem initialization etc
const FILEOPS nullFileOperations = {
	(void*)CommonOkResultFunction,
	(void*)CommonOkResultFunction,
	(void*)CommonOkResultFunction,
	(void*)CommonOkResultFunction,
	(void*)CommonOkResultFunction,
};
char* NullFileIdentify(register __i0 void *self, char *buf, u_int16 bufsize) {return "FNUL";}
const SIMPLE_FILE nullFile = {__MASK_PRESENT | __MASK_OPEN | __MASK_READABLE | __MASK_WRITABLE | __MASK_FILE, 
	NullFileIdentify, &nullFileOperations};


SIMPLE_FILE vo_simple_files[__FOPEN_SIMPLE_FILES];

#if 0
FILESYSTEM *vo_filesystems[] = {
	&FatFileSystem,
	//&characterDeviceFS,
	0,
	0,0,0	
};
#else
extern FILESYSTEM *vo_filesystems[]; //Initializer in kernel_abs.s
#endif
//void (*vo_sys_error_hook)(char *msg) = 0; //User can use this to report errors on screen
//void (*vo_sys_report_hook)(char *msg) = 0; //User can use this to report status/warnings on screen
void (*vo_get_time_from_rtc_hook)(void); //User can provide this to read current time to currentTime.


/// Returns a human-readable description of any kind of VSOS object (file name, device name etc.)
const char *Identify(VO_FILE *f) {
	static char s[6];
	sprintf(s,"@%x",f);
	if (!f) return "NIL";
	if (f->Identify) return f->Identify(f,0,0);
	if (!(f->dev)) return "NUL";	
	if ((f->dev->Identify)) return f->dev->Identify(f,0,0);
	return s;
}




typedef struct SectorBufferStruct {
	u_int16 data[256];
	FILE *f;
	u_int32 sector;
} SectorBuffer;

#ifdef USE_MALLOC_SECTOR_BUFFERS
struct SectorBufferStruct *sbsP[__SECTOR_BUFFERS+5]; //+5 if we want to temporarily add buffers
#else
SectorBuffer sbs[__SECTOR_BUFFERS];
#endif
u_int16 nSectorBuffers;

#ifdef USE_MALLOC_SECTOR_BUFFERS
void FreeBuffer(SectorBuffer *b) {
  u_int16 i;

  if (!b) return;

#if 0
  printf("FreeBuf frags%p, fileP %p, Ident %s.\n", b, b->f, Identify(b->f));
#endif

  for (i=0; i<nSectorBuffers; i++) {
    if (sbsP[i] == b) {
      sbsP[i] = NULL;
    }
  }
  free(b);
}
#else
void FreeBuffer(SectorBuffer *b) {
	if (!b) return;
	//printf("Free %p from %s. ",b,Identify(b->f));
	memset(b,0,sizeof(SectorBuffer));
}
#endif

SectorBuffer *GetBuffer(VO_FILE *f) {
	u_int16 i;
	static int ii=0;
#if 0
	printf("#%d GetBuf fp %p ", ii++, f);
#endif
	for (i=0; i<nSectorBuffers; i++) {
		// Iterate over all buffers to free those left hanging by abnormal file closures.
#ifdef USE_MALLOC_SECTOR_BUFFERS
		if (sbsP[i] && (((sbsP[i]->f->flags & __MASK_PRESENT) == 0) || (sbsP[i]->f==f))) {
			// Found a buffer allocated for nonpresent file or to the file descriptor,
			// which is currently being created (this actually happens!)
			FreeBuffer(sbsP[i]);
		}
#else
		if (sbs[i].f && (((sbs[i].f->flags & __MASK_PRESENT) == 0) || (sbs[i].f==f))) {
			// Found a buffer allocated for nonpresent file or to the file descriptor,
			// which is currently being created (this actually happens!)
			FreeBuffer(&sbs[i]);
		}
#endif
	}
	//Allocate a new buffer
	for (i=0; i<nSectorBuffers; i++) {
		// Find a free sectorBuffer (its file pointer is NULL)
#ifdef USE_MALLOC_SECTOR_BUFFERS
		if (sbsP[i] && !sbsP[i]->f) {
			sbsP[i]->f = f;
			return sbsP[i];		
		}
#else
		if (!sbs[i].f) {
			sbs[i].f = f;
			return &sbs[i];		
		}
#endif
	}

#ifdef USE_MALLOC_SECTOR_BUFFERS
	for (i=0; i<nSectorBuffers; i++) {
		if (!sbsP[i]) {
			if (!(sbsP[i] = calloc(1, sizeof(*sbsP[i])))) {
				SysError("Out of mem");
			} else {
				sbsP[i]->f = f;
			}
#if 0
			printf("sbsP[%d] = %p (%d wrd)\n", i, sbsP[i], ((u_int16 *)sbsP[i])[-1]);
#endif
			return sbsP[i];
		}
	}
#endif
	SysError("Out of bufs");
	return NULL;
}




VO_FILE *MakeFileDescriptor(DEVICE *dev) {
	VO_FILE *f = NULL;
	if (!dev) return NULL;
	
	#ifdef __FOPEN_SIMPLE_FILES
	if (!dev->fs) {
		u_int16 i;
		for (i=0; i<__FOPEN_SIMPLE_FILES; i++) {
			if (!__F_PRESENT(&vo_simple_files[i])) {
				f = (VO_FILE*)&vo_simple_files[i];
				memset(f,0,sizeof(SIMPLE_FILE));
				f->flags = __MASK_PRESENT;
				if (kernelDebugLevel) fprintf(vo_stderr,"Simple[%d:%p] ",i,f);
				goto fd_found;
			}
		}
		goto no_fd;
	} else
	#endif //ifdef __FOPEN_SIMPLE_FILES
	{
		u_int16 i;
		for (i=0; i<vo_max_num_files; i++) {
			if (!__F_PRESENT(&vo_files[i])) {
				f = &vo_files[i];
				memset(f,0,sizeof(VO_FILE));
				if (kernelDebugLevel) fprintf(vo_stderr,"[%d:%p] ",i,f);
				if (!__F_CHARACTER_DEVICE(dev)) {
					if (!(f->sectorBuffer = (u_int16*)GetBuffer(f))) {
						goto no_fd;
					}
					if (kernelDebugLevel) fprintf(vo_stderr,"Buf[%p] ",f->sectorBuffer);
				}
				f->flags = __MASK_PRESENT;
				f->dev = dev;
				goto fd_found;
			}
		}
		no_fd:
		SysError("Out of descriptors");
		return 0;			
	}
	
	fd_found:	
	f->flags = __MASK_PRESENT;
	f->dev = dev;
	f->Identify = dev->Identify;
	if (dev->fs) {
		f->op = dev->fs->op;
		f->Identify = dev->fs->Identify;
	} else if (__F_CHARACTER_DEVICE(dev)) {
		f->op = (void*)((u_int16)dev + 2); 
	}
	if (!f->op) {
		FreeBuffer((SectorBuffer*)(f->sectorBuffer));
		f->flags = 0;
		return 0; //the file would have no operations.
	}
	return f;		
}






/* C File operations */	

auto IOCTL_RESULT ioctl(register void *p, register int request, register IOCTL_ARGUMENT arg) {
	int res = 0;
	if (!p) {
		return S_ERROR;
	}
	if (__F_FILE((VO_FILE*)p)) {
		VO_FILE *fp = p;
		if (!fp->op || !fp->op->Ioctl) {
			return S_ERROR;
		}
		res = fp->op->Ioctl(fp, request, arg);
	} else {
		DEVICE *dp = p;
		if (!dp->Ioctl) {
			return S_ERROR;
		}
		res = dp->Ioctl(dp, request, arg);	
	}
  	
	 
	return res;
}

int vo_fgetc(VO_FILE *stream) {
/**< Gets the next character (an unsigned char) from the specified stream and advances the position indicator for the stream.	
On success the character is returned. If the end-of-file is encountered, then EOF is returned and the end-of-file indicator is set. If an error occurs then the error indicator for the stream is set and EOF is returned. */
	u_int16 c = 0;
	if (!__F_CAN_READ(stream)) {
		//printf("Cannot read (flags=0x%04x)\n", stream->flags);
		return EOF;
	}
	if (__F_UNGETC(stream)) {
		stream->flags &= ~__MASK_UNGETC;
		return (int)stream->ungetc_buffer;
	}
	stream->op->Read(stream, &c, 1, 1);
	if (__F_ERROR(stream) || __F_EOF(stream)) {
		return EOF;
	}
	return c;
}

char *vo_fgets(char *str, int n, VO_FILE *stream) {
/**< Reads a line from the specified stream and stores it into the string pointed to by str. It stops when either (n-1) characters are read, the newline character is read, or the end-of-file is reached, whichever comes first. The newline character is copied to the string. A null character is appended to the end of the string.
On success a pointer to the string is returned. On error a null pointer is returned. If the end-of-file occurs before any characters have been read, the string remains unchanged. */
	char *org = str;
	char c;
	while ((n>1) && ((c = vo_fgetc(stream)) != EOF)) {
		//printf("[C=%04x]",c);
		n--;
		*str++ = c;		
		if (c == '\n') {
			break;
		}
	}
	if (org != str) {
		*str++ = '\0';
	} else {
		org = NULL;
	}
	return org;	
};

u_int32 vo_ftell(VO_FILE *stream) {
	return stream->pos - ((__F_UNGETC(stream)) ? 1 : 0);
}
	
ioresult vo_fseek(VO_FILE *stream, s_int32 offset, s_int16 origin) {
  s_int32 newpos;
  if (!(__F_CAN_SEEK(stream))) {
    return -1;
  }
#if 0
  if (__F_CAN_WRITE(stream)) {
    static const char *sss[3] = {"SET", "CUR", "END"};
    printf("fseek(%p, %lx, SEEK_%s)\n", stream, offset, sss[origin]);
  }
#endif
  if (__F_CAN_WRITE(stream)) {
    fflush(stream);
  }
  if (origin == SEEK_SET) {
    newpos = offset;
  } else if (origin == SEEK_END) {
    newpos = stream->fileSize + offset;
  } else if (origin == SEEK_CUR) {
    newpos = stream->pos + offset - ((__F_UNGETC(stream)) ? 1 : 0);
  } else {
    return -1;
  }
  if (newpos < 0 || newpos > stream->fileSize) {
    return -1;
  }
  stream->flags &= ~(__MASK_UNGETC | __MASK_EOF);
  stream->pos = newpos;
  return 0;
}



	
int vo_fputc(int ch, VO_FILE *stream) {
	/**< Writes a character (an unsigned char) specified by the argument char to the specified stream and advances the position indicator for the stream.
	On success the character is returned. If an error occurs, the error indicator for the stream is set and EOF is returned. */
	u_int16 c = ch;	
	stream->op->Write(stream, &c, 1, 1);
	if (__F_ERROR(stream)) {
		return EOF;
	}
	return ch;
}
	
int vo_fputs(const char *str, VO_FILE *stream) {
	/**< Writes a string to the specified stream up to but not including the null character.
	On success a nonnegative value is returned. On error EOF is returned. */
	while(*str) {
		if (vo_fputc(*str++, stream) < 0) {
			return EOF;
		}
	}
	return 0;
}
	
auto int vo_puts(register const char *s) {
	if (fputs(s, vo_stdout) == EOF)
	  return EOF;
	return fputc('\n', vo_stdout);
}

	
int vo_ungetc(int ch, VO_FILE *stream) {
	/**< Pushes the character char (an unsigned char) onto the specified stream so that the this is the next character read. The functions fseek, fsetpos, and rewind discard any characters pushed onto the stream.
	Multiple characters pushed onto the stream are read in a FIFO manner (first in, first out).
	On success the character pushed is returned. On error EOF is returned. */
	if (!(__F_CAN_UNGETC(stream))) {
		return EOF;
	}
	stream->flags |= __MASK_UNGETC;
	stream->ungetc_buffer = ch;
	return ch;
}

u_int16 vo_fread(void *ptr, u_int16 size, u_int16 nobj, VO_FILE *stream) {
	u_int16 sizeBytes = nobj*size*2;
	u_int16 readBytes = 0;
	if (!stream) {
		SysError("DevNullRd");
	}
	if (!(__F_CAN_READ(stream)) || (!sizeBytes)) {
		return 0;
	}
#if 0
	if (__F_CAN_WRITE(stream)) {
	  printf("fread(%p, %d, %d, %p)\n", ptr, size, nobj, stream);
	}
#endif
	if (stream->flags & __MASK_UNGETC) {
		u_int16 *p = ptr;
		*(u_int16*)p = (vo_fgetc(stream) << 8);
		*(u_int16*)p++ |= (vo_fgetc(stream));
		//printf("unget in fread\n");
		ptr = p;
		sizeBytes -= 2;	
		readBytes = 2;	
	}
	//ObtainMutex(&fopenclose_mutex);
	readBytes += stream->op->Read(stream, ptr, 0, sizeBytes);
	//ReleaseMutex(&fopenclose_mutex);
	//printf("Flags %04x, pos %ld, readbytes %d\n",stream->flags,stream->pos,readBytes);
	return readBytes / (size * 2);
}

u_int16 vo_fwrite(const void *ptr, u_int16 size, u_int16 nobj, VO_FILE *stream) {
	u_int16 sizeBytes = nobj*size*2;
	if (!__F_CAN_WRITE(stream)) {
		return 0;
	}
#if 0
	printf("fwrite(%p, %d, %d, %p)\n", ptr, size, nobj, stream);
#endif
	return stream->op->Write(stream, ptr, 0, sizeBytes) / (size * 2);
}


/// Start a file system for device. For FAT filesystem, the name string refers to a primary partition, "0" is the first partition.
FILESYSTEM *StartFileSystem(DEVICE *dev, char *name) {
	FILESYSTEM *fs;
	u_int16 i;
	u_int16 *fatBuffer = (u_int16*)GetBuffer((void*)&nullFile);
	if (!fatBuffer) {
		SysError("Out of buffers");
		return 0;
	}
	i = 0;
	while ((fs=vo_filesystems[i])) {
		if (fs->Create(dev, name, fatBuffer) == S_OK) {
			dev->fs = fs; //Just in case... FS should also set this.
			FreeBuffer((SectorBuffer*)fatBuffer);
			return fs;
		} else {
			//printf("Filesystem %s cannot handle this device.\n",fs->Identify(fs,0,0));
		}
		i++;
	}
	FreeBuffer((SectorBuffer*)fatBuffer);
	//no fs found, this is not an error, some devices don't have filesystems.
	return 0;
}

//void FileInfo(VO_FILE *f) {
	//printf("\nf=%04x flags=%04x dev=%04x op=%04x\n",f,f->flags,f->dev,f->op);
//}

VO_FILE *vo_fopen(const char *filename, const char *mode) {	
	VO_FILE *f = NULL;
	DEVICE *dev;
	u_int16 i,d=0;
	u_int16 attempts = fopen_retries;

	if (kernelDebugLevel) { fprintf(stderr,"\nfopen %s ",filename); }
	
	if (filename[1] != ':') {
		// maybe we should append a "current path" to the filename,
		// but for now just fail, because we expect a full filename
		// starting with device letter and colon for now.
		SysError("no path in fname");
		goto finally;
	}
	d = toupper(filename[0])-'A'; //get device letter, lowercase to uppercase
	if ((d>25) || !vo_pdevices[d]) {
		SysError("No device for %s", filename);
		goto finally; //devices are 'A' to 'Z'.
	}
	dev = vo_pdevices[d];
	if (!__F_OPEN(dev)) dev->Ioctl(dev, IOCTL_RESTART, 0); //If device is not open, IOCTL_RESTART it.
	if (!__F_OPEN(dev)) {
		SysError("Device not open");
		goto finally;
	}									

	ObtainMutex(&fopenclose_mutex);
	f = MakeFileDescriptor(dev);
	if (!f) {
		//SysError("Cannot make descriptor");//Already reported		
		goto finallyReleaseMutex;
	}
	ReleaseMutex(&fopenclose_mutex);
	
	if (mode[0]=='s') { //fopen called for getting a file descriptor for finding a file
		f->flags = __MASK_PRESENT | __MASK_OPEN | __MASK_SEEKABLE | __MASK_READABLE;
		goto finally;
	}
			
	if (!f->op) {
		SysError("NullF");
		goto finally;
	}
	if (!f->op->Open) {
		SysError("NoOpen");
		goto finally;
	}
	
	while (attempts--) {
		u_int16 oldFlags = f->flags;
		ioresult r = f->op->Open(f, &filename[2], mode);
		if (kernelDebugLevel && r) { fprintf(vo_stderr,"Open=%d ",r); }
		if (r == E_FILE_NOT_FOUND) goto nofile;
		if (r != S_OK) {
			if (!attempts) {
			nofile:
				ObtainMutex(&fopenclose_mutex);
				f->flags = 0; // Open failed, mark the file as not present
				FreeBuffer((SectorBuffer*)(f->sectorBuffer));				
#ifdef USE_MALLOC_SECTOR_BUFFERS
				f->sectorBuffer = NULL;
#endif
				//SysReport("Open Failed.");
				//fprintf(vo_stderr,"Open Failed: %s ",filename);
				f = NULL;
				goto finallyReleaseMutex;
			}
			// Try to restart the device between open attempts
			f->flags = oldFlags;
			f->dev->Ioctl(f->dev, IOCTL_RESTART, 0);
		} else {
			//opened ok;
			file_open:
			// Set file's deviceInstance to be device's deviceInstance
			f->deviceInstance = f->dev->deviceInstance; 
			f->flags |= __MASK_FILE | __MASK_PRESENT | __MASK_OPEN;
			goto finally;
			break;
		}
	}
 finally:
	return f;
	finallyReleaseMutex:
	ReleaseMutex(&fopenclose_mutex);
	return f;
}


ioresult vo_fclose(register __i0 VO_FILE *stream) {	
	u_int16 errCode = S_OK;
	if (__F_WRITABLE(stream) && !AttemptMutex(&fclose_gettime_mutex)) {
		RunLibraryFunction("RTCREAD",ENTRY_MAIN,0);
		ReleaseMutex(&fclose_gettime_mutex);
	}

	if (kernelDebugLevel) {vo_fprintf(vo_stderr,"fclose[%p] ",stream);}
	if (!stream) return S_OK;
	if (!stream->op) return S_OK;
	ObtainMutex(&fopenclose_mutex);
	if (stream->op->Close) {	
		errCode = stream->op->Close(stream);		
	}
	FreeBuffer((SectorBuffer*)stream->sectorBuffer);
	stream->flags=0; //do this last (re-entrancy)
	ReleaseMutex(&fopenclose_mutex);
	return errCode;
}

//ioresult vo_StartOS(u_int16 *osMemPtr, u_int16 osMemSizeWords) {
ioresult vo_kernel_init(void) { //No buffer parameter in this version
	u_int16 i;
	fopen_retries = 2;
	InitMutex(&fopenclose_mutex);
	InitMutex(&fclose_gettime_mutex);
	nSectorBuffers = __SECTOR_BUFFERS;
	vo_max_num_files = sizeof(vo_files) / sizeof(vo_files[0]);
	vo_get_time_from_rtc_hook = NULL;
	memset(vo_files,0,sizeof(vo_files));
	memset(vo_simple_files,0,sizeof(vo_simple_files));
	memset(vo_pdevices,0,sizeof(vo_pdevices));
#ifdef USE_MALLOC_SECTOR_BUFFERS
	memset(sbsP,0,sizeof(sbsP));
#else
	memset(sbs,0,sizeof(sbs));
#endif
	return S_OK;
}

ioresult vo_feof(VO_FILE *stream) {
	return (ioresult) (stream->flags & __MASK_EOF);
}

ioresult vo_ferror(VO_FILE *stream) {
	return (ioresult) (stream->flags & __MASK_ERROR);
}

ioresult CommonOkResultFunction(void) {
	return S_OK;
}

auto void AutoVoidNull(void) {
}

ioresult CommonErrorResultFunction() {
	return S_ERROR;
}


void BusyWait(u_int32 n) {
	while(n--);
}



ioresult vo_fflush(VO_FILE *stream) {
	if (!__F_DIRTY(stream)) {
		return S_OK; //Not dirty
	}
	if (!stream->dev->BlockWrite) {
		return S_ERROR; //Dirty but no BlockWrite
	}
	stream->flags &= ~(__MASK_DIRTY);
	return stream->dev->BlockWrite(stream->dev, stream->currentSector, 1, stream->sectorBuffer);		
}

void RemovedInterface(void) {
	SysError("Too old app for this kernel");
	while(1);
}

auto u_int16 BiosService(u_int16 service,...) {
	return S_ERROR;
}

void vs3emubreak(register __i1 s_int16 fe) {
}

void ZeroPtrCall(register __i1 u_int16 lr0, register __i0 u_int16 i0) {
	vo_stdout = vo_stderr;
	Disable();
	fprintf(vo_stderr,"ZeroPtrCall from %u(0x%04x) ",lr0,lr0);
	RunLibraryFunction("TRACE",ENTRY_1,lr0);
	fprintf(vo_stderr,"\ni0=%u(0x%04x) ",i0,i0);
	RunLibraryFunction("TRACE",ENTRY_2,i0);
	fputc('\n',vo_stderr);
	RunProgram("TASKS","-v");
	fprintf(vo_stderr,"\nStarting POST-CRASH SHELL, be careful.\n");
	Enable();Enable();
	RunProgram("S:SHELL.AP3",NULL);

	vs3emubreak(0xfe);
	while(1);
}
