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
u_int16 alsoFree[3] = {0};
u_int16 afterInit = 0;

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
	u_int16 istart = 0;	

	entries = *p++;
	refCount = *p++;
	entry_main = *p++;
	fini = *p++;
	entrylist = p;
	p += entries;
	libfileptr = *(u_int32*)p;
	p+= 2;
	nSections = *p++;
	
	//fprintf(vo_stderr,"Lib %ld, has ",libfileptr);
	//fprintf(vo_stderr,"%d entries:",entries);
	//for (;entries;entries--) {
	//	fprintf(vo_stderr," %p",*entrylist++);
	//}
	//fprintf(vo_stderr," and %d sections:\n",nSections);
	for (;nSections;nSections--) {
		u_int16 size = (*p >> 2) << 1;		
		u_int16 page = *p&3;
		sizes[page] += size;

		p++;
		if (page==0 && istart==0) {
			fprintf(of,"%04x: ",*p);
			istart = *p;
		}
		p++;		
	}
	p++;
	{
		char *s = (char*)p;
		if (strstr(s,"INIT.")==s) {
			afterInit = 1;
		}
		while (*s && *s != '.') fputc(*s++, of);
		while (s < p + 8) {
			fputc(' ',of); s++;
		}
	}
	
	fprintf(of," %5di +%5dx +%5dy \n",sizes[0],sizes[1],sizes[2]);
	if (afterInit) {
		u_int16 i;
		for (i=0; i<3; i++) {
			alsoFree[i] += sizes[i]; //report size allocated for INIT and following libraries also as free memory
		}
	}
}


int main(void) {
	u_int16 i;
	fprintf(vo_stdout,"\n%d libs loaded:\n",loadedLibs);
	for (i=0; i<MAX_LIB; i++) {
		if (loadedLib[i]) PrintLibInfo2(vo_stdout,loadedLib[i]);		
	}
	PrintFreeMem();
}
