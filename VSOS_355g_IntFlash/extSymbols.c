#include <vo_stdio.h>
#include <stdlib.h>
#include <string.h>
#include "extSymbols.h"

const u_int32 __mem_y encVOggCrc[256] = {
 0x00000000L, 0x04C11DB7L, 0x09823B6EL, 0x0D4326D9L,
 0x130476DCL, 0x17C56B6BL, 0x1A864DB2L, 0x1E475005L,
 0x2608EDB8L, 0x22C9F00FL, 0x2F8AD6D6L, 0x2B4BCB61L,
 0x350C9B64L, 0x31CD86D3L, 0x3C8EA00AL, 0x384FBDBDL,
 0x4C11DB70L, 0x48D0C6C7L, 0x4593E01EL, 0x4152FDA9L,
 0x5F15ADACL, 0x5BD4B01BL, 0x569796C2L, 0x52568B75L,
 0x6A1936C8L, 0x6ED82B7FL, 0x639B0DA6L, 0x675A1011L,
 0x791D4014L, 0x7DDC5DA3L, 0x709F7B7AL, 0x745E66CDL,
 0x9823B6E0L, 0x9CE2AB57L, 0x91A18D8EL, 0x95609039L,
 0x8B27C03CL, 0x8FE6DD8BL, 0x82A5FB52L, 0x8664E6E5L,
 0xBE2B5B58L, 0xBAEA46EFL, 0xB7A96036L, 0xB3687D81L,
 0xAD2F2D84L, 0xA9EE3033L, 0xA4AD16EAL, 0xA06C0B5DL,
 0xD4326D90L, 0xD0F37027L, 0xDDB056FEL, 0xD9714B49L,
 0xC7361B4CL, 0xC3F706FBL, 0xCEB42022L, 0xCA753D95L,
 0xF23A8028L, 0xF6FB9D9FL, 0xFBB8BB46L, 0xFF79A6F1L,
 0xE13EF6F4L, 0xE5FFEB43L, 0xE8BCCD9AL, 0xEC7DD02DL,
 0x34867077L, 0x30476DC0L, 0x3D044B19L, 0x39C556AEL,
 0x278206ABL, 0x23431B1CL, 0x2E003DC5L, 0x2AC12072L,
 0x128E9DCFL, 0x164F8078L, 0x1B0CA6A1L, 0x1FCDBB16L,
 0x018AEB13L, 0x054BF6A4L, 0x0808D07DL, 0x0CC9CDCAL,
 0x7897AB07L, 0x7C56B6B0L, 0x71159069L, 0x75D48DDEL,
 0x6B93DDDBL, 0x6F52C06CL, 0x6211E6B5L, 0x66D0FB02L,
 0x5E9F46BFL, 0x5A5E5B08L, 0x571D7DD1L, 0x53DC6066L,
 0x4D9B3063L, 0x495A2DD4L, 0x44190B0DL, 0x40D816BAL,
 0xACA5C697L, 0xA864DB20L, 0xA527FDF9L, 0xA1E6E04EL,
 0xBFA1B04BL, 0xBB60ADFCL, 0xB6238B25L, 0xB2E29692L,
 0x8AAD2B2FL, 0x8E6C3698L, 0x832F1041L, 0x87EE0DF6L,
 0x99A95DF3L, 0x9D684044L, 0x902B669DL, 0x94EA7B2AL,
 0xE0B41DE7L, 0xE4750050L, 0xE9362689L, 0xEDF73B3EL,
 0xF3B06B3BL, 0xF771768CL, 0xFA325055L, 0xFEF34DE2L,
 0xC6BCF05FL, 0xC27DEDE8L, 0xCF3ECB31L, 0xCBFFD686L,
 0xD5B88683L, 0xD1799B34L, 0xDC3ABDEDL, 0xD8FBA05AL,
 0x690CE0EEL, 0x6DCDFD59L, 0x608EDB80L, 0x644FC637L,
 0x7A089632L, 0x7EC98B85L, 0x738AAD5CL, 0x774BB0EBL,
 0x4F040D56L, 0x4BC510E1L, 0x46863638L, 0x42472B8FL,
 0x5C007B8AL, 0x58C1663DL, 0x558240E4L, 0x51435D53L,
 0x251D3B9EL, 0x21DC2629L, 0x2C9F00F0L, 0x285E1D47L,
 0x36194D42L, 0x32D850F5L, 0x3F9B762CL, 0x3B5A6B9BL,
 0x0315D626L, 0x07D4CB91L, 0x0A97ED48L, 0x0E56F0FFL,
 0x1011A0FAL, 0x14D0BD4DL, 0x19939B94L, 0x1D528623L,
 0xF12F560EL, 0xF5EE4BB9L, 0xF8AD6D60L, 0xFC6C70D7L,
 0xE22B20D2L, 0xE6EA3D65L, 0xEBA91BBCL, 0xEF68060BL,
 0xD727BBB6L, 0xD3E6A601L, 0xDEA580D8L, 0xDA649D6FL,
 0xC423CD6AL, 0xC0E2D0DDL, 0xCDA1F604L, 0xC960EBB3L,
 0xBD3E8D7EL, 0xB9FF90C9L, 0xB4BCB610L, 0xB07DABA7L,
 0xAE3AFBA2L, 0xAAFBE615L, 0xA7B8C0CCL, 0xA379DD7BL,
 0x9B3660C6L, 0x9FF77D71L, 0x92B45BA8L, 0x9675461FL,
 0x8832161AL, 0x8CF30BADL, 0x81B02D74L, 0x857130C3L,
 0x5D8A9099L, 0x594B8D2EL, 0x5408ABF7L, 0x50C9B640L,
 0x4E8EE645L, 0x4A4FFBF2L, 0x470CDD2BL, 0x43CDC09CL,
 0x7B827D21L, 0x7F436096L, 0x7200464FL, 0x76C15BF8L,
 0x68860BFDL, 0x6C47164AL, 0x61043093L, 0x65C52D24L,
 0x119B4BE9L, 0x155A565EL, 0x18197087L, 0x1CD86D30L,
 0x029F3D35L, 0x065E2082L, 0x0B1D065BL, 0x0FDC1BECL,
 0x3793A651L, 0x3352BBE6L, 0x3E119D3FL, 0x3AD08088L,
 0x2497D08DL, 0x2056CD3AL, 0x2D15EBE3L, 0x29D4F654L,
 0xC5A92679L, 0xC1683BCEL, 0xCC2B1D17L, 0xC8EA00A0L,
 0xD6AD50A5L, 0xD26C4D12L, 0xDF2F6BCBL, 0xDBEE767CL,
 0xE3A1CBC1L, 0xE760D676L, 0xEA23F0AFL, 0xEEE2ED18L,
 0xF0A5BD1DL, 0xF464A0AAL, 0xF9278673L, 0xFDE69BC4L,
 0x89B8FD09L, 0x8D79E0BEL, 0x803AC667L, 0x84FBDBD0L,
 0x9ABC8BD5L, 0x9E7D9662L, 0x933EB0BBL, 0x97FFAD0CL,
 0xAFB010B1L, 0xAB710D06L, 0xA6322BDFL, 0xA2F33668L,
 0xBCB4666DL, 0xB8757BDAL, 0xB5365D03L, 0xB1F740B4L
};

u_int32 SymbolCalcCrc32String(register const char *s) {
  unsigned long polyRes = 0;

  while (*s) {
#if 0
    polyRes = EncVGenOggCrcByte(polyRes, *s++);
#else
    polyRes = encVOggCrc[((u_int16)(polyRes>>24) ^ *s++) & 0xFF] ^ (polyRes << 8);
#endif
  }

  return polyRes;
}


struct ExtSymbol __mem_y extSymbol[CRC32_HASH_SIZE] = {{0}};
u_int16 __mem_y extSymbolElements = 0;

const unsigned char __mem_y crc6ToChar[64] =
  "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789__";

u_int32 SymbolCrunchStringCalcCrc32(const char *name) {
#define MAX_NAME_LEN 15
	char s[MAX_NAME_LEN+1];
	int len = strlen(name);
	u_int32 crc32;
	strncpy(s, name, MAX_NAME_LEN+1);

    if (len > MAX_NAME_LEN) {
		crc32 = SymbolCalcCrc32String(name);

		s[MAX_NAME_LEN-3] = crc6ToChar[(crc32 >> 12) & 63];
		s[MAX_NAME_LEN-2] = crc6ToChar[(crc32 >>  6) & 63];
		s[MAX_NAME_LEN-1] = crc6ToChar[(crc32      ) & 63];
		s[MAX_NAME_LEN-0] = '\0';
	}

#if 0
	printf("C:%s\n", s);
#endif

	return SymbolCalcCrc32String(s);
}
struct ExtSymbol __mem_y *SymbolAdd(register const char *name, register void *lib, register u_int16 addr) {
  u_int32 crc32 = /*SymbolCalcCrc32String(name)*/SymbolCrunchStringCalcCrc32(name);
  u_int16 idx;
  struct ExtSymbol __mem_y *h;

#if 0
  printf("A:%s\n", name);
#endif

  if (!crc32 || extSymbolElements >= CRC32_HASH_SIZE-1) {
    return NULL;
  }

  idx = (u_int16)crc32 & (CRC32_HASH_SIZE-1);
  while (h=extSymbol+idx,h->crc32) {
    if (h->crc32 == crc32) {
      /* Name clash */
      return NULL;
    }
    idx = (idx+1) & (CRC32_HASH_SIZE-1);
  }

  h->crc32= crc32;
  h->lib = lib;
  h->addr = addr;
  extSymbolElements++;

  return h;
}

struct ExtSymbol __mem_y *SymbolFindByCrc(register u_int32 crc32) {
  u_int16 idx = (u_int16)crc32 & (CRC32_HASH_SIZE-1);
  struct ExtSymbol __mem_y *h;
  while (h=extSymbol+idx,h->crc32) {
    if (h->crc32 == crc32) {
      return h;
    }
    idx = (idx+1) & (CRC32_HASH_SIZE-1);
  }
  return NULL;
}


u_int16 SymbolDelete(register u_int32 crc32) {
  struct ExtSymbol __mem_y *h = SymbolFindByCrc(crc32);
  int moveTo, candidate;
  struct ExtSymbol __mem_y *c;

  if (!h) {
    return 0; /* Could not find element to delete */
  }
  /* Found the element: delete it */
  extSymbolElements--;
  memsetY(h, 0, sizeof(*h));

  /* Start looking if it is possible to move another element to the
     empty space left by removed element. */
  moveTo = h-extSymbol;
#if 0
  printf("  moveTo = 0x%03x\n", moveTo);
#endif
  candidate = (moveTo+1) & (CRC32_HASH_SIZE-1);

  /* Continue as long as candidate is not an empty cell. */
  while (c=extSymbol+candidate,c->crc32) {
    int quit = 0, test = candidate;
#if 0
    printf("    moveTo = 0x%03x, candidate 0x%03x\n", moveTo, candidate);
#endif
    while (!quit) {
      struct ExtSymbol __mem_y *m = extSymbol+moveTo;
      struct ExtSymbol __mem_y *t = extSymbol+test;
      if (test == moveTo) {
	/* Candidate can be moved to empty space: do it */
#if 0
	printf("      Move 0x%03x to 0x%03x\n", candidate, moveTo);
#endif
	memcpyYY(m, c, sizeof(extSymbol[0]));
	memsetY(c, 0, sizeof(extSymbol[0]));
	/* Candidate is now moved, so we'll have to see whether something
	   can be moved to the new space left by it */
	moveTo = candidate;
	quit = 1;
      } else if ((c->crc32 & (CRC32_HASH_SIZE-1)) == test) {
	/* Candidate can not be moved, stop testing */
#if 0
	printf("      Cannot be moved 0x%03x, belongs to 0x%03x\n",
	       candidate, moveTo);
#endif
	quit = 1;
      } /* If none of the ifs match, continue testing */
      test = (test-1) & (CRC32_HASH_SIZE-1);
    }
    /* Next candidate, please */
    candidate = (candidate+1) & (CRC32_HASH_SIZE-1);
  }
  return 1;
}

u_int16 SymbolDeleteLib(register void *lib) {
  int i, res = 0;
  struct ExtSymbol __mem_y *h;

#if 0
  printf("SymbolDeleteLib(%x)\n", (int)lib);
#endif
  h = extSymbol;
  i = 0;
  for (i=0; i<CRC32_HASH_SIZE; i++) {
    /* If we can find an element belongin to a library, remove it.
       Because removing an element may shift other elements up, continue
       in a while loop until no match is found. */
    while (h->lib == lib) {
#if 0
      printf("  SymbolDeleteLib: delete index 0x%03x\n", i);
#endif
      res++;
      SymbolDelete(h->crc32);
    }
    h++;
  }

  return res;
}

#ifndef __VSDSP__

void SymbolPrint(void) {
  int i, totalDistance = 0, maxDistance = 0;
  struct ExtSymbol *h = extSymbol;
  printf("SymbolPrint()\n");
  for (i=0; i<CRC32_HASH_SIZE; i++) {
    if (h->crc32) {
      int distance = (i - (u_int16)h->crc32) & (CRC32_HASH_SIZE-1);
      totalDistance += distance;
      if (distance > maxDistance) {
	maxDistance = distance;
      }
      printf("  i 0x%03x, crc 0x%08lx, lib 0x%04x, addr 0x%04x, distance %2d\n",
	     i, (long)h->crc32, (int)h->lib, h->addr, distance);
    }
    h++;
  }
  printf("  Fill %3d of %3d (%5.1f%%), max distance %2d, mean %5.2f\n",
	 extSymbolElements, CRC32_HASH_SIZE,
	 100.0*extSymbolElements/CRC32_HASH_SIZE,
	 maxDistance,
	 (double)totalDistance/CRC32_HASH_SIZE);
}


/* Test vector */
int main(void) {
  printf("Add00 = %p\n", SymbolAdd("TestFunc0", (void *)0x0001, 0x0100));
  printf("Add01 = %p\n", SymbolAdd("TestFunc1", (void *)0x0001, 0x0101));
  printf("Add02 = %p\n", SymbolAdd("TestFunc2", (void *)0x0001, 0x0102));
  printf("Add03 = %p\n", SymbolAdd("TestFunc2", (void *)0x0001, 0x0103));
  printf("Add04 = %p\n", SymbolAdd("TestFunc4", (void *)0x0002, 0x0104));
  printf("Add05 = %p\n", SymbolAdd("TestFunc5", (void *)0x0002, 0x0105));
  printf("Add06 = %p\n", SymbolAdd("TestFunc6", (void *)0x0002, 0x0106));
  printf("Add07 = %p\n", SymbolAdd("Rimpula"  , (void *)0x0003, 0x0107));
  printf("Add08 = %p\n", SymbolAdd("OggVJurn" , (void *)0x0003, 0x0108));
  printf("Add09 = %p\n", SymbolAdd("Symbol"   , (void *)0x0003, 0x0109));
  printf("Add10 = %p\n", SymbolAdd("aestFunc0", (void *)0x0001, 0x0110));
  printf("Add11 = %p\n", SymbolAdd("aestFunc1", (void *)0x0001, 0x0111));
  printf("Add12 = %p\n", SymbolAdd("aestFunc2", (void *)0x0001, 0x0112));
  printf("Add13 = %p\n", SymbolAdd("aestFunc2", (void *)0x0001, 0x0113));
  printf("Add14 = %p\n", SymbolAdd("aestFunc4", (void *)0x0002, 0x0114));
  printf("Add15 = %p\n", SymbolAdd("aestFunc5", (void *)0x0002, 0x0115));
  printf("Add16 = %p\n", SymbolAdd("aestFunc6", (void *)0x0002, 0x0116));
  printf("Add17 = %p\n", SymbolAdd("aimpula"  , (void *)0x0003, 0x0117));
  printf("Add18 = %p\n", SymbolAdd("aggVJurn" , (void *)0x0003, 0x0118));
  printf("Add19 = %p\n", SymbolAdd("aymbol"   , (void *)0x0003, 0x0119));
  SymbolPrint();
#if 0
  printf("Find1 = %p\n", SymbolFind(0x12345678));
  printf("Find2 = %p addr %04x\n", SymbolFind(0x44bab1e6),
	 SymbolFind(0x44bab1e6)->addr);
  printf("Find3 = %p addr %04x\n", SymbolFind(0x3828eef8),
	 SymbolFind(0x3828eef8)->addr);
#endif
  SymbolDeleteLib((void *)2);
  SymbolDeleteLib((void *)1);
  SymbolPrint();
  return EXIT_SUCCESS;
}
#endif
