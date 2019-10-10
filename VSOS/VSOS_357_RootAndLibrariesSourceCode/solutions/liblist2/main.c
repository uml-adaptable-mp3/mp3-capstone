/* For free support for VSIDE, please visit www.vsdsp-forum.com */

/// \file liblist.c Print out a list of loaded libraries and free mem to stdout
/// \author Panu-Kristian Poiksalo, VLSI Solution Oy


#include <vo_stdio.h>
#include <volink.h>
#include <apploader.h>
#include <lcd.h>
#include <stdbuttons.h>
#include <string.h>
#include <kernel.h>
#include "memstore.h"

const char ixy[] = "IXY";


void PrintLibInfo2(FILE *of, u_int16 *lib) {
	u_int16 *p = lib;
	u_int16 entries;
	u_int16 *entrylist;
	u_int32 libfileptr;
	u_int16 fini;
	u_int16 entry_main;
	u_int16 nSections;	
	u_int16 refCount;
	u_int16 sizes[3]={0};
	char *ixy = "IXY";

	entries = *p++;
	refCount = *p++;
	entry_main = *p++;
	fini = *p++;
	entrylist = p;
	p += entries;
	libfileptr = *(u_int32*)p;
	p+= 2;
	nSections = *p++;
	
	fprintf(of,"\nLib %p has entry points",lib);
	for (;entries;entries--) {
		fprintf(of," %p",*entrylist++);
	}
	fprintf(of," and %d sections:\n",nSections);
	for (;nSections;nSections--) {
		u_int16 size = (*p >> 2) << 1;		
		u_int16 page = *p&3;
		sizes[page] += size;

		p++;
		fprintf(of,"  %c:%04x..%04x (%d words)\n",ixy[page],*p,*p+size-1,size);
		p++;		
	}
	p++;
	{
		char *s = (char*)p;
		while (*s && *s != '.') fputc(*s++, of);
		while (s < p + 8) {
			fputc(' ',of); s++;
		}
	}
	
	fprintf(of," %5di +%5dx +%5dy \n",sizes[0],sizes[1],sizes[2]);
}


int main(void) {
	u_int16 i;
	fprintf(vo_stderr,"\n%d libs loaded:\n",loadedLibs);
	for (i=0; i<MAX_LIB; i++) {
		if (loadedLib[i]) PrintLibInfo2(vo_stderr,loadedLib[i]);		
	}
	PrintFreeMem();
}
