#ifndef CRC32_HASH
#define CRC32_HASH

#ifndef __VSDSP__
typedef unsigned int u_int32;
typedef unsigned short u_int16;
#define __mem_y
#define memsetY memset
#define memcpyYY memcpy
#else
#include <vstypes.h>
#endif

#define CRC32_HASH_BITS 8
#define CRC32_HASH_SIZE (1<<CRC32_HASH_BITS)

struct ExtSymbol {
  u_int32 crc32;
  void *lib;
  u_int16 addr;
};

struct ExtSymbolRom {
  u_int32 crc32;
  u_int16 addr;
};

extern struct ExtSymbol __mem_y extSymbol[CRC32_HASH_SIZE];
extern u_int16 __mem_y extSymbolElements;
extern u_int16 __mem_y extSymbolSearchRom;
extern u_int16 extSymbolRomSize;
extern struct ExtSymbolRom extSymbolRom[];


#define InitExtSymbols() {extSymbolSearchRom=1;extSymbolElements=0;memsetY(extSymbol, 0, sizeof(extSymbol));}
/* Adds element to CRC32 hash table.
   Returns pointer to new element if successful, NULL for failure. */
struct ExtSymbol __mem_y *SymbolAdd(register const char *name, register void *lib, register u_int16 addr);
/* Returns 32-bit CRC for symbol name string */
u_int32 SymbolCalcCrc32String(register const char *name);
/* Returns 32-bit CRC for symbol name string; crunch string to max 15
   characters if needed before calclating CRC. */
u_int32 SymbolCrunchStringCalcCrc32(const char *name);
/* Finds element with given CRC.
   Returns pointer to element if successful, NULL if not found. */
struct ExtSymbol __mem_y *SymbolFindByCrc(register u_int32 crc);
#define SymbolFind(s) (SymbolFindByCrc(SymbolCalcCrc32String(s)))
/* Finds ROM element with given CRC.
   Returns pointer to element if successful, NULL if not found. */
struct ExtSymbolRom *SymbolFindRomByCrc(register u_int32 crc);
#define SymbolFindRom(s) (SymbolFindRomByCrc(SymbolCalcCrc32String(s)))
/* Delete an element with given CRC.
   Returns number of elements deleted (0 = fail). */
u_int16 SymbolDelete(register u_int32 crc);
/* Delete all elements that belong to a given library.
   Returns number of elements deleted. */
u_int16 SymbolDeleteLib(register void *lib);

#endif /* CRC32_HASH */
