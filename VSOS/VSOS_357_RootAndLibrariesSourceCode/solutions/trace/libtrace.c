/* For free support for VSIDE, please visit www.vsdsp-forum.com */

/// \file libtrace.c Find the library which contains the code address, which is given as a parameter
/// \author Panu-Kristian Poiksalo, VLSI Solution Oy

/// This library is called by the kernel entry point ZeroPtrCall. It resolves the library from which 
/// a call to address 0 was made. Usually such a function call is made by calling a method of an
/// uninitialized object. This library then finds which library is loaded to that address, prints out
/// the name of the library, segment and offset. That information is then passed to find_err.dl3,
/// which opens the libfile, scans its symbol table and prints out the name of the function that
/// made the erroneous method call.


#include <vo_stdio.h>
#include <volink.h>
#include <apploader.h>
#include <lcd.h>
#include <stdbuttons.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <kernel.h>
#include "trace.h"

DLLENTRY(TraceI)
DLLENTRY(TraceX)
DLLENTRY(TraceY)
DLLENTRY(TraceXShort)
DLLENTRY(TraceYShort)

const char ixy[] = "IXY";
//u_int16 offend_addr = 0;
char *offend_plibname = NULL;


struct offend_info off = {
	0, 0, 0, 0, 32768U, 0, ""
};
struct offend_info offR = {
	0, 0, 0, 0, 32768U, 0, ""
};



/// From all segments loaded in the system, find the one that has the smallest offset to the desired address
void LibScan2(register u_int16 *lib) {
	u_int16 *p = lib;
	u_int16 entries;
	u_int16 *entrylist;
	u_int32 libfileptr;
	u_int16 fini;
	u_int16 entry_main;
	u_int16 nSections;	
	u_int16 refCount;
	u_int16 sizes[3]={0};
	u_int16 istart = 0;	
	u_int16 i;
	
	entries = *p++;
	refCount = *p++;
	entry_main = *p++;
	fini = *p++;
	entrylist = p;
	p += entries;
	libfileptr = *(u_int32*)p;
	p += 2;
	nSections = *p++;
	
	for (i=0; i<nSections; i++) {
		u_int16 size = (*p >> 2) << 1;		
		u_int16 page = *p&3;
		sizes[page] += size;
		p++;
		//printf("\n%d:%p-%p:%p:%p",off.page,off.address,lib,page,*p);
		//if ((page==off.page) && (*p <= off.address) && (off.offset > off.address-*p)) {
		if ((page==off.page) && (*p <= off.address) && (*p+size > off.address)) {
			off.offset = off.address-*p;
			off.lib = lib;
			off.segment = i;
			off.libfileptr = libfileptr;
			//printf(" MATCH");
		}
		p++;		
	}
	p++;
	if (off.lib == lib) 	{
		char *d = off.libname;
		char *s = (char*)p;
		while (*s && *s != '.') {
			*d++ = *s++;
		}
		*d=0;
	}
}

void SymbolFileScan(register u_int16 addr, register char *filename) {
	static u_int16 i;
	u_int16 j;
	char c;
	FILE *f = fopen(filename,"r");
	if (!f) {
		printf("NoSym");
		return;
	}
	while (!feof(f)) {
		fread(&i,1,1,f);
		fgetc(f); //page, known to be 0
		if (i<=addr) {
			printf("::");
		}
		for (j=0; j<29; j++) {
			c = fgetc(f);
			if (c && i <= addr) {
				fputc(c,stdout);	
			}
		}
		if (i<addr) {
			printf("[0x%x]",addr-i);
			break;
		}
		if (i==addr) break;
	}
	fclose(f);
}

int trace(register s_int16 page, register u_int16 addr) {
	u_int16 i;
	u_int16 zpc = GetSymbolAddress("_ZPC");	

	off.page = abs(page);
	off.address = addr;
	if (!page) {
		if (addr >= 0x8000u) {
			printf("IROM");
			SymbolFileScan(addr,"S:SYS/VS1005*.SYM");
			goto print_value;
		}
		if (addr > zpc && addr < zpc+70) {
			printf("Zero Pointer Call Trap");
			goto print_value;
		}
		if (addr < appIXYStart[0]) {
			printf("KERNEL");
			SymbolFileScan(addr,"S:SYS/KERNEL.SYM");
			goto print_value;
		}
	}
	for (i=0; i<MAX_LIB; i++) {
		if (loadedLib[i]) LibScan2(loadedLib[i]);		
	}
	if (!off.lib) {
		printf("UNASSIGNED");
		goto print_value;
	}
	//fprintf(vo_stdout,"%s:%d.%d",off.libname,off.segment,off.offset);
	//RunLibraryFunction("FIND_ERR",0,(int)&off);
	//find_err(&off);
	printf("%s:",off.libname);
	if (S_ERROR == find_err(&off)) {
		printf("%d:%d",off.segment,off.offset);
	}
	print_value:
	if (page>0) {
		u_int16 v;
		if (page==1) {
			v = USEX(addr);
		} else {
			v = USEY(addr);
		}
		printf("=0x%04x(%d)",v,v);
		if (isprint(v) && v<255) printf("'%c'",v);
	}
	return S_OK;
}


ioresult main(char *p) {
	u_int16 page = 0;
	u_int16 addr = 0x7fff;
	u_int16 endaddr;
	char *e;
	if (p[1]==':') {
		char first = toupper(p[0]);
		if (first=='X') page=1;
		if (first=='Y') page=2;
		addr = (u_int16)strtoul(&p[2],&e,0);
	} else {
		addr = (u_int16)strtoul(p,&e,0);
	}
	endaddr = (u_int16)strtoul(e,&e,0);
	if (endaddr) {
		while (addr<=endaddr) {
			printf("%c:0x%04x ",ixy[page],addr);
			trace(page,addr++);
			memcpy(&off,&offR,sizeof(off));
			printf("\n");
		}
	} else {
		trace(page,addr);
		printf("\n");
	}
}

ioresult TraceI(u_int16 addr) {
	trace(0,addr);
}
ioresult TraceX(u_int16 addr) {
	trace(1,addr);
}
ioresult TraceY(u_int16 addr) {
	trace(2,addr);
}
ioresult TraceXShort(u_int16 addr) {
	trace(-1,addr);
}
ioresult TraceYShort(u_int16 addr) {
	trace(-2,addr);
}
