#ifndef APPLOADER_H
#define APPLOADER_H

#include <vo_stdio.h>
#include <vstypes.h>
#include <mutex.h>
#include <vs1005h.h>
#include <vsos.h>

#define _RESTYPE_STRING 0x0100
#define _RES_TITLE 1
#define _RESTYPE_BITMAP 0x0200
#define _RES_ICON 1
#define _RESTYPE_MEMORY 0x0300
#define _RESTYPE_RELOCATION 0x0400
#define _RESTYPE_RELOCATION_SECTION 0x0401
#define _RESTYPE_RELOCATION_SYMNAME 0x0402
#define _RESTYPE_RELOCATION_INTERNAL 0x0403
#define _RESTYPE_RELOCATION_EXTERNAL 0x0404
#define _RESTYPE_SYMBOL 0x0800
#define _RESTYPE_ENTRYLIST 0x0900
#define _RESTYPE_LOADINFO 0x0000
#define _RESTYPE_ROMCHECK 0x000a
#define _RESTYPE_CALLPRELOAD 0x000b

#define ENTRY_MAIN 0
#define ENTRY_FINI 1
#define ENTRY_1 2
#define ENTRY_2 3
#define ENTRY_3 4
#define ENTRY_4 5
#define ENTRY_5 6
#define ENTRY_6 7
#define ENTRY_7 8
#define ENTRY_8 9
#define ENTRY_9 10
#define ENTRY_10 11
#define ENTRY_11 12
#define ENTRY_12 13
#define ENTRY_13 14
#define ENTRY_14 15
#define ENTRY_15 16
#define ENTRY_16 17

#define MAX_LIB 16
extern void *loadedLib[MAX_LIB];
extern u_int16 loadedLibs;

extern VO_FILE *appFile;

typedef struct sectionMemInfoStruct {
	u_int16 address;
	u_int16 page;
} SectionMemInfo;

struct resourceHeaderStruct {
	u_int16 resourceType;
	u_int16 subType;
	u_int32 length;
};
typedef struct resourceStruct {
	u_int32 pos;
	struct resourceHeaderStruct header;
} Resource;
extern u_int16 loadLibraryMutex;

int RunAppFile (int i);
int RunAppFile3 (void *parameters);
extern void **entryList;
#define LoadLibraryInit() (InitMutex(&loadLibraryMutex))
void *LoadLibraryFile(FILE *f, char *paramStr);
void *LoadLibraryFile3(FILE *f);
void *LoadLibrary(const char *filename);
void *LoadLibraryP(const char *filename, void *parameters);
void DropLibrary(u_int16 *lib);
void PrintLibInfo(u_int16 *lib);
int RunLibraryFunction(const char *filename, u_int16 entry, int i);

#define RunProgram(dl3name,params) (RunLibraryFunction((dl3name),ENTRY_MAIN,(int)(params)))
#define RunLoadedFunction(lib, num, arg) ( ((s_int16 (*)(s_int16))(*((u_int16 *)(lib)+2+(num))))(arg) )

#endif
