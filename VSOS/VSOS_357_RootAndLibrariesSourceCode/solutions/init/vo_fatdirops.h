#ifndef __VO_FAT_DIR_OPS__
#define __VO_FAT_DIR_OPS__

#include <vstypes.h>
#include <stdio.h>

enum OpenFileMode {
  /* Positive values are file numbers */
  ofmNone = 0,
  ofmName = -1,
  ofmNext = -2,
  ofmPrev = -3,
  ofmSame = -4,
  ofmFirst = -5,
  ofmLast = -6,
};

enum EntryType {
  etFile = 0,
  etDir,
  etLongName,
  etLongNameLast,
  etDeleted,
  etEnd
};

/* Return back to initial state: current file is "0", so next will return
   first file. */
auto s_int16 ResetFindFile(FILE *fp);

/* Used to find next file */
auto s_int16 FindFile(FILE *fp, char *name, s_int16 nameSize,
		      const char *suffixes,
		      enum OpenFileMode openFileMode,
		      enum EntryType entryType);


//enum EntryType CheckEntryType(FILE *fp, unsigned char *name);



#define FAT_DIR_EXT_CHARS 256

struct FatDirExt {
  u_int16 lastGoodEntry;
  char currentDir[FAT_DIR_EXT_CHARS];
};

auto s_int16 InitDirectory(FILE **fpp); //< Call this first

auto s_int16 SetDirectory(FILE **fpp, const char *s); //< Call with s = NULL to reset directory read. In first call fp may be NULL; the function will rewrite it.

auto const char *GetDirectory(FILE **fpp); //< Return current directory name string

auto s_int16 EndDirectory(FILE **fpp); //< End directory operations


 
#endif /* !__VO_FAT_DIR_OPS__ */
