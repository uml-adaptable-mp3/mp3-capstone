/* For free support for VSIDE, please visit www.vsdsp-forum.com */

#include <vo_stdio.h>
#include <volink.h>
#include <apploader.h>
#include <lcd.h>
#include <stdbuttons.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <apploader.h>

const char ixy[] = "IXY";

u_int16 NextResource(register Resource *resource, register FILE *fp) {
  if (resource->pos) {
    fp->pos = resource->pos + 8 + resource->header.length;
    if (fp->pos & 1) fp->pos++;
  }
  resource->pos = fp->pos;	
  fread(&resource->header, 1, sizeof(resource->header), fp);
  return resource->header.resourceType;
}

int PrintLibSymbolRaw(register const char *libName, register u_int32 libFilePtr, register const char *symName) {
  FILE *fp;
  Resource resource = {0};

  if (!(fp = fopen(libName, "rb"))) {
    return 0;
  }

  fp->pos=0;
  if (fgetc(fp) != 'V') return S_ERROR; //First byte of VDE1

	//printf("#3");

	
  fp->pos=4;
  while (resource.header.resourceType != _RESTYPE_ENTRYLIST) {
    int type = NextResource(&resource, fp);
    if ((type & ~0xf) == _RESTYPE_SYMBOL) {
      char ch;
      printf(":%04x:%04x:%08lx:%08lx: ",
	     type, resource.header.subType, resource.header.length,
	     resource.pos);

      while (fread(&ch, 1, 1, fp) == 1 && ch) {
	fputc(ch,stdout);
      }
      printf("\n");
    }
  }
	
 finally:
  fclose(fp);
  fp = NULL;
}

u_int32 FindLibName(register char *s, register u_int16 *lib) {
  u_int32 res;
  lib += *lib + 4;
  res = *(u_int32*)lib;
  printf("libfileptr = %08lx\n", res);
  lib += 2;
  lib += *lib*2 + 2;
  strcpy(s, lib);

  s = strchr(s,'.');
  if (s) *s = '\0';

  return res;
}


extern u_int16 myX;
extern u_int16 myY;

extern __align u_int16 myAlignX[5];
extern __align u_int16 myAlignY[5];

extern u_int16 myBufX[7];
extern u_int16 myBufY[7];

int PrintLibSymbol(const char *s) {
  u_int16 i;
  u_int16 page = 0, addr = 0;

  i = myX+myY+(int)myAlignX+(int)myAlignY+(int)myBufX+(int)myBufY;

  printf("IROM", i);
#if 0
  SymbolFilePrintSymbol("S:SYS/VS1005*.SYM", s);
  SymbolFilePrintSymbol("S:SYS/KERNEL.SYM", s);
#endif

  for (i=0; i<MAX_LIB; i++) {
    if (loadedLib[i]) {
      static char libName[6+8+4+1] = "S:SYS/";
      u_int32 libFilePtr = FindLibName(libName+6, loadedLib[i]);
      strcat(libName, ".dl3");
      printf("%2d %s\n", i, libName);
      PrintLibSymbolRaw(libName, libFilePtr, s);
    }
  }

  return S_OK;
}

