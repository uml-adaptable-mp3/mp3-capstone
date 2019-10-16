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
  etEnd,
  etVolumeId
};

/* Return back to initial state: current file is "0", so next will return
   first file. */
auto s_int16 ResetFindFile(FILE *fp);
/* Reset sorted file search. Must be called before FindFileSorted. */
auto s_int16 ResetFindFileSorted(register FILE *fp, 
				 register struct SortedFiles *sortedFiles,
				 register s_int16 *sortCache,
				 register u_int16 sortCacheSize,
				 register u_int16 flags);

/* Find next file. Returns index to file (base 1), or <= 0 for error. */
auto s_int16 FindFile(FILE *fp, char *name, s_int16 nameSize,
		      const char *suffixes,
		      enum OpenFileMode openFileMode,
		      enum EntryType entryType);

/* Find next file in ascending case-insensitive ACII order.
   Returns index to file (base 1), or <= 0 for error.
   Note that indexes returned are for the index in the file system
   (compatible with FindFile() openFileMode parameter type, and thus
   not necessarily ascending. */
auto s_int16 FindFileSorted(register FILE *fp,
			    register struct SortedFiles *sortedFiles,
			    register char *name, register s_int16 nameSize,
			    const char *suffixes,
			    register enum EntryType entryType);


//enum EntryType CheckEntryType(FILE *fp, unsigned char *name);



#define FAT_DIR_EXT_CHARS 256

struct FatDirExt {
  u_int16 lastGoodEntry;
  char currentDir[FAT_DIR_EXT_CHARS];
};

struct SortedFiles {
  s_int16 firstIdx, currIdx;
  s_int16 n;
  u_int16 flags;
  s_int16 *index;
};

auto s_int16 InitDirectory(FILE **fpp); //< Call this first

auto s_int16 SetDirectory(FILE **fpp, const char *s); //< Call with s = NULL to reset directory read. In first call fp may be NULL; the function will rewrite it.

auto const char *GetDirectory(FILE **fpp); //< Return current directory name string

auto s_int16 EndDirectory(FILE **fpp); //< End directory operations

/** Find first instance of '/' or '\' in string s.
    Returns pointer to slash, or NULL if not found. */
auto const char *FindFirstSlash(const char *s);

/** Find last instance of '/' or '\' in string s.
    Returns pointer to slash, or NULL if not found. */
auto const char *FindLastSlash(const char *s);
 
#endif /* !__VO_FAT_DIR_OPS__ */
