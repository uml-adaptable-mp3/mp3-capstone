/// \file apploader.c Relocating Loader for VS1005 VSOS3
/// \author Panu-Kristian Poiksalo, VLSI Solution Oy 2012-2013

#include <vstypes.h>
#include "vo_stdio.h"
#include <string.h>
#include "apploader.h"
#include "power.h"
#include <vsos.h>
#include <timers.h>
#include <sysmemory.h>
#include "memstore.h"
#include "getmemory.h"
#include "vo_fat.h"
#include "lowlevels.h"
#include "extSymbols.h"
#include <mutex.h>
#include <ctype.h>

static Resource resource;
u_int16 ovlChecksum = 0;

u_int16 NextResource(register FILE *f) {
	if (!f) {
		SysError("0Res");
	}
	if (resource.pos) {
		f->pos = resource.pos + 8 + resource.header.length;
		if (f->pos & 1) f->pos++;
	}
	resource.pos = f->pos;	
	{
		u_int16 i;
		u_int16 *p = &resource.header;
		fread(p, 1, sizeof(resource.header), f);
		if (resource.header.resourceType != _RESTYPE_CHECKSUM_CHECK) {
			ovlChecksum += *p++;
			ovlChecksum += *p++;
			ovlChecksum += *p++;
			ovlChecksum += *p++;
		}
	}
	return resource.header.resourceType;
}

u_int16 ResGet16(register FILE *f) {
	u_int16 res;
	fread(&res, 1, 1, f);
	ovlChecksum += res;
	//printf("%04x ",res);
	return res;
}
u_int32 ResGet32(register FILE *f) {
	u_int32 res;
	fread(&res, 1, 2, f);
	ovlChecksum += (res >> 16);
	ovlChecksum += (res);
	//printf("%08lx ",res);
	return res;
}

#define VDE1 0x45315644
#define VDE2 0x45325644
#define MAX_SECTIONS 20

void RemoveLibFromList(void *lib) {
	u_int16 i;
	for (i=0; i<MAX_LIB; i++) {
		if (loadedLib[i] == lib) {
			u_int16 j;
			for (j=i; j<MAX_LIB-1; j++) {
				loadedLib[j] = loadedLib[j+1];
			}
			loadedLib[MAX_LIB-1] = 0;			
		}
	}
	loadedLibs--;
}

void CallToDroppedLibrary() {
	ioresult (*zerocall)(void) = NULL;
	Disable();
	fprintf(stderr,"\nCall to dropped lib! Interrupt level (encount): %d, last int vector: %d\n",PERIP(INT_ENCOUNT)-1,PERIP(INT_VECTOR));
	
	
	zerocall();	
}

// Original contents of int vectors after bootload - these should all point to ROM
__mem_y const u_int32 defaultIntVector[32] = {
	0x2a24e14e, 0x2a202a4e, 0x2a2026ce, 0x2a2026ce, 0x2a2026ce, 0x2a2026ce, 0x2a2026ce, 0x2a2026ce, 
	0x2a2026ce, 0x2a2026ce, 0x2a2026ce, 0x2a2026ce, 0x2a2026ce, 0x2a2026ce, 0x2a24f14e, 0x2a00834e, 
	0x2a2026ce, 0x2a2026ce, 0x2a2026ce, 0x2a2026ce, 0x2a2026ce, 0x2a2026ce, 0x2a2026ce, 0x2a2026ce, 
	0x2a2026ce, 0x2a2026ce, 0x2a2026ce, 0x2a2026ce, 0x2a2026ce, 0x2a2026ce, 0x2a2026ce, 0x2a2026ce,
};

auto void DisableIntVector(register u_int16 addr) {
  if ((addr-=0x20U) < 0x20U) { /* Is range 0x20 - 0x3f? */
    u_int16 intBase = INT_ENABLE0_LP + (addr>>4); /* addr = 32-47 -> INT_ENABLE0_LP, addr = 48-63 ->- INT_ENABLE1_LP */
    u_int16 intMask = ~(1U<<(addr&15));
    PERIP(intBase) &= intMask;
    intBase += INT_ENABLE0_HP - INT_ENABLE0_LP;
    PERIP(intBase) &= intMask;
  }
}


void DropLibrary(u_int16 *lib) {
	u_int16 *p = lib;
	u_int16 entries;
	u_int16 *entrylist;
	u_int32 libfileptr;
	u_int16 fini;
	u_int16 nSections;	
	if (!lib) return;

	if (--lib[1]) {//Decrease reference count
		return; //Still references.
	}
	RemoveLibFromList(lib);
	
	entries = *p++;
	p++;p++;
	fini = *p++;
	entrylist = p;
	p += entries;
	libfileptr = *(u_int32*)p;
	p+= 2;
	nSections = *p++;
	{
		ioresult (*finalize)(void) = fini; 
		finalize();
	}	
	for (;nSections;nSections--) {		
		u_int16 page = *p & 3;
		u_int16 size = (*p++ >> 2) << 1;
		void *addr = *p++;
		switch(page) {
		case 0: 
			if ((u_int16)addr >= 256) { //Don't try to free interrupt vectors			
				u_int16 a;
				u_int16 i;
				for (i=0; i<0x20; i++) {
					if ((((ReadFromProgramRam(i+0x20)>>6L) & 0xffff) - (u_int16)addr) < size) {
						// fprintf(stderr,"fix vector %x\n",i+0x20);
						WriteToProgramRam(i+0x20,defaultIntVector[i]);
						DisableIntVector(i+0x20);
					}
				}


				//printf("Clear %d,%d ",addr,size);				
				for(a=(u_int16)addr; a<(u_int16)addr+size-1; a++) {
					//WriteIMem((void*)a, 0x2a000000UL); //direct jmpi to zero
					WriteIMem((void*)a, 0x2a000000+((u_int32)((u_int16)CallToDroppedLibrary) << 6));				
				}
				__FreeMemI(addr,size);
			}
			break;
		case 1:
			FreeMemX(addr, size);
			break;
		default:
			FreeMemY((__y void*)addr, size);
			break;
		}		
	}
	free(lib); //Bye bye!
	//PrintLibInfos();

}


void *LoadLibraryP(char *filename, void *parameters) {
	void *lib;
	FILE *f;
	char s[25];
	if (kernelDebugLevel) {
		fprintf(vo_stderr,"%s... ",filename);
	}

	if (filename[1] != ':') {
		sprintf(s,"S:SYS/%s.DL3",filename);		
		f = fopen(s,"rb");
	} else {
		f = fopen(filename,"rb");
	}	
	if (!f) {
		//SysError(filename);
		//SysError("noLibF");
		SysError("%s not found",filename);
		return 0;
	}	
	lib = LoadLibraryFile(f,parameters);
	fclose(f);
	if (!lib) {
		SysError("lib");
		return 0;
	}
	PERIP(ANA_CF1) |= ANA_CF1_BTNDIS; //Disable power button reset
	//SysError("ok");
	return lib;
}

void *LoadLibrary(char *filename) {
	return LoadLibraryP(filename,"");
}

#if 0
u_int16 LibNameCompare(register u_int16 *lib, register const char *name) {
	u_int16 *p = lib;
	u_int16 entries;
	u_int16 nSections;	
	u_int16 i=0;
	entries = *p++;
	p += entries + 5;	
	nSections = *p++;
	p += nSections * 2 + 1;
	{
		char *s = (char*)p;
		while (1) {
			if (*name == 0) {
				if ((i==8) || (*s=='.')) return 1; 
				return 0;
			}
			if (*s == 0) return 1;
			if (toupper(*s) != toupper(*name)) return 0;
			i++;
			s++; name++;
		}
	}
}

void *FindLib(register __i1 const char *name) {
	u_int16 i;
	for (i=0; i<loadedLibs; i++) {
		if (loadedLib[i] && LibNameCompare(loadedLib[i],name)) {
			return loadedLib[i];
		}
	}
	return NULL;
}
#endif


int RunLibraryFunction(const char *filename, u_int16 entry, int i) {
	int r = S_ERROR;
#if 0
	u_int16 *lib = FindLib(filename);
#else
	u_int16 *lib = NULL;
#endif
	if (!lib) {
		lib = LoadLibrary(filename);
	} else {
		lib[1]++; //ref count inc
	}
	if (lib) {
		if ((entry < lib[0]+2) && lib[entry+2]) {
			r = ((int(*)(int))(lib[entry+2]))(i);
		}
		DropLibrary(lib);
	}
	return r;
}


u_int16 VOGetSymbolAddress(const char *s) {
  struct ExtSymbol __mem_y *h = SymbolFindByCrc(SymbolCalcCrc32String(s));
  if (!h) return 0;
  return h->addr;
}



// Load a library from file f. Does not set file position.

ioresult ResourceLoadHook(Resource *r, FILE *f);

void *LoadLibraryFile(FILE *f, char *parameters) {
	u_int16 preloadResult;
	u_int16 *lib = NULL;
	static u_int16 secbase[MAX_SECTIONS];
	static __y u_int16 secpage[MAX_SECTIONS];
	u_int16 secn = 0;
	ioresult (*init)(void *parameters, void **lib) = NULL;
	static u_int16 mutex = 1;
	u_int16 fatalErrors = 0;
	s_int16 wrongRom = 0;
	
	ObtainMutex(&mutex);

	{
		u_int16 i;
		for (i=0; i<loadedLibs; i++) {
			u_int16 *p = loadedLib[i];
			u_int32 lptr;
			p += *p + 4;
			lptr = *(u_int32*)p;
			if (lptr == ((FatFileInfo*)f->fileInfo)->startCluster) {
				if (kernelDebugLevel) {
					fprintf(vo_stderr,"Already loaded(LIB%d).\n",i);
				}
				//PrintLibInfos();
				p = loadedLib[i];
				p[1]++; //Increse reference count
				ReleaseMutex(&mutex);
				return p;
			}
		}
	}

	memset(&resource,0,sizeof(resource));

	{
		u_int32 t32 = ResGet32(f);
		if (t32 != VDE1 && t32 != VDE2) {
			SysError("Too old APP");
			ReleaseMutex(&mutex);
			return 0;
		}
	}
			
	while (NextResource(f)) {
		//printf("Ap(%p)R%04x ",f,resource.header.resourceType);
		
		if (ResourceLoadHook(&resource, f)) continue;

#if 0
		printf("Typ %04x, ln %lx\n", resource.header.resourceType, resource.header.length);
#endif
		if (wrongRom) {
		  if ((resource.header.resourceType & 0xff00u) == _RESTYPE_LOADINFO && resource.header.resourceType == _RESTYPE_ROMCHECK) {
		  	u_int32 t = ResGet32(f);
			wrongRom = (t && t != ReadFromProgramRam(0xFFFF));
#if 0
		    printf("#2 Wr %d\n", wrongRom);
		  } else {
		    printf("Skip\n");
#endif
		  }
		  continue;
		}

		switch(resource.header.resourceType & 0xff00u) {
			
			case _RESTYPE_MEMORY: {
				u_int16 page = resource.header.resourceType & 0xf;
				u_int16 address = ResGet16(f);
				u_int16 align = ResGet16(f);
				u_int16 realSizeWords = ResGet16(f);
				//u_int16 sizeWords = (realSizeWords+1) & ~1;
				u_int16 sizeWords = (realSizeWords+2) & ~1;

				address = GetMemory(page,address,sizeWords,align);
				secbase[secn] = address;
				secpage[secn++] = page | (sizeWords << 1);

				//address = GetMemory(page,address,sizeWords,align);
				//secbase[secn] = address;
				//secpage[secn++] = page | ((((sizeWords+1)&~1)) << 1);
			
				if (page==1) {
					*(u_int16 *)(address+sizeWords-1) = 0xbeef;
				} else if (page==2) {
					*(__y u_int16 *)(address+sizeWords-1) = 0xbeef;
				}

				while (realSizeWords--) {
					u_int16 text = 0;
					if (!(resource.header.subType)) text = ResGet16(f); // Load data only if not BSS section
					switch (page) {
						case 0: DisableIntVector(address); WriteToProgramRam16Swapped(address, text, ResGet16(f)); break;
						case 1: *(u_int16 *)address = text; break;
						default: *(__y u_int16 *)address = text; break;						
					}
					address++;
				}
				break;
			}
			

			case _RESTYPE_RELOCATION: {
				static __y u_int16 rBase = 0;
				static u_int16 rPage = 0;
				static u_int16 extAddr = 0;				
				if (resource.header.resourceType == _RESTYPE_RELOCATION_SECTION) {
					rBase = secbase[resource.header.subType];
					rPage = secpage[resource.header.subType]&3;
				}
				if (resource.header.resourceType == _RESTYPE_RELOCATION_SYMNAME) {
					u_int16 i;
					char s[16];
					u_int16 *p=s;
					
					for (i=0; i<resource.header.length/2; i++) *p++ = ResGet16(f);
					extAddr = GetSymbolAddress(s);
					if (!extAddr) {
						static const char *e = "E: Symbol %-15s (CRC32 %08lx) not found.\n";
						u_int32 crc32 = SymbolCalcCrc32String(s);
						fatalErrors++;
#if 0
						printf(e, s, crc32);						
						Delay(100);
#endif
						vo_fprintf(vo_stderr, e, s, crc32);
#if 0
						Delay(100);
#endif
						Forbid();
						//ReleaseMutex(&mutex);
						//return NULL;
					}
				}
				if (resource.header.resourceType == _RESTYPE_RELOCATION_EXTERNAL) {
					u_int16 n = ResGet16(f);
					while(n--) { //423w
						u_int16 addr = rBase + ResGet16(f);					
						switch(rPage) {
							case 0: {				
								u_int32 text = ReadFromProgramRam(addr);
								s_int16 ro = text>>6;
								u_int16 to = extAddr + ro; //target offset
								text &= ~(0xffffL << 6);
								text |= ((u_int32)to << 6);
								WriteToProgramRam(addr,text);
								break;
							}
							case 1: *(u_int16*)addr += extAddr; break; 
							default: *(__y u_int16*)addr += extAddr; break;
						}
					}
				}
				if (resource.header.resourceType == _RESTYPE_RELOCATION_INTERNAL) {
					u_int16 ts = resource.header.subType; //target section
					u_int16 tBase = secbase[ts];
					u_int16 n = ResGet16(f);
					while(n--) { //423w
						u_int16 addr = rBase + ResGet16(f);					
						switch(rPage) {
							case 0: {				
								u_int32 text = ReadFromProgramRam(addr);
								s_int16 ro = text>>6;
								u_int16 to = tBase + ro; //target offset
								text &= ~(0xffffL << 6);
								text |= ((u_int32)to << 6);
								WriteToProgramRam(addr,text);
								break;
							}
							case 1: *(u_int16*)addr += tBase; break; 
							default: *(__y u_int16*)addr += tBase; break;
						}
					}
				}
				break;
			}
			

			case _RESTYPE_LOADINFO: {
				if (resource.header.resourceType == _RESTYPE_ROMCHECK) {
					u_int32 t = ResGet32(f);
					wrongRom = (t && t != ReadFromProgramRam(0xFFFF));
				} else if (resource.header.resourceType == _RESTYPE_CALLPRELOAD) {
					void* (*preload)(void) = secbase[secn-1];
					preloadResult = preload();	
				}
				break;
			}
			

			case _RESTYPE_ENTRYLIST: {
				int i;
				u_int16 n = resource.header.length / 6;
				u_int16 *p = lib = malloc(1+9+7+n+secn*2);				
				if (lib) {									
					*p++ = n; //nEntries
					*p++ = 1; //refCount (1 for newly loaded library)
					*p++ = (u_int16)CommonOkResultFunction; //main()
					*p++ = (u_int16)CommonOkResultFunction; //fini()
					for (i=1; i<=n; i++) {
						u_int16 ident = ResGet16(f);
						u_int16 sect = ResGet16(f);
						u_int16 value = ResGet16(f);
						u_int16 a = (value + secbase[sect]);
						*p++ = a;
						if (ident == 0x6d61) {//"ma"
							lib[2]=a; //Main
						}
						if (ident == 0x6669) {//"fi"
							lib[3]=a;
						}
						if (ident == 0x696e) {//"in"
							init = a;
						}
						if (ident == 0x7072) {//"pr"
							*((u_int16*)a)=preloadResult;
						}
					}
					*(u_int32*)p = ((FatFileInfo*)f->fileInfo)->startCluster;
					p += 2;
					
					*p++ = secn;
					for (i=0; i<secn; i++) {
						*p++ = secpage[i];
						*p++ = secbase[i];
					}
					*p++=0; //For upcoming extension
					{//File name
						strncpy(p,f->Identify(f,NULL,0),8);
						p += 8;
						*p++ = 0; //Ending zero
					}
				}
				break;
			}
			
			case _RESTYPE_CHECKSUM: {
				if (resource.header.resourceType == _RESTYPE_CHECKSUM_CHECK && resource.header.subType != ovlChecksum) {
					SysError("Corrupt lib %s",f->Identify(f,NULL,0));
					fatalErrors++;
				}
				ovlChecksum = 0;
				break;
			}

			default: {
				//printf("Unknown R%04x/%04x ",resource.header.resourceType,resource.header.subType);
				break;
			}
		}
	}
	if (lib) loadedLib[loadedLibs++] = lib;

	ReleaseMutex(&mutex);

	if (fatalErrors) {
		const char *e = "%d fatal errors. Stop.\n";
		vo_fprintf(vo_stderr, e, fatalErrors);
	  	while (1) {
			//Stop
		}
	}

	if (init) {
		u_int16 stackSave = GetI6();
	
		init(parameters, lib);
	
		if (GetI6() != stackSave) {
			printf("\nStack trashed.\n");
			fprintf(vo_stderr,"\nStack trashed.\n");
			while(1);
		}
	}
	return lib;
}



int RunAppFile3(void *parameters) {		
	u_int16 *lib;	
	int r = -1;
	appFile->pos = 0;
	lib = LoadLibraryFile(appFile, parameters);
	if (lib) {
		r = ((int(*)(void*))(lib[2]))(parameters); //main()
		DropLibrary(lib);
	}
	return r;
}


