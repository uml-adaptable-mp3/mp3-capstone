/* For free support for VSIDE, please visit www.vsdsp-forum.com */

/// \file find_err.c Find an error location in a libfile
/// \author Panu-Kristian Poiksalo, VLSI Solution Oy

/// This library is typically called by libtrace.dl3, which passes a libname[8],
/// segment and offset to this find_err, which then tries to open the libfile by
/// extending libname to s:sys/libname.dl3 or, that failing, to s:libname*
/// It then scans the libfile's symbol table and prints the name of the function
/// in whose address space the specified location (error point) is.

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <vo_fat.h>
#include <string.h>
#include "trace.h"



char lfn[20];
FILE *f;
static Resource resource;


u_int16 NextResource(register FILE *f){
	if (resource.pos) {
		f->pos = resource.pos + 8 + resource.header.length;
		if (f->pos & 1) f->pos++;
	}
	resource.pos = f->pos;	
	fread(&resource.header, 1, sizeof(resource.header), f);
	return resource.header.resourceType;
}


ioresult find_err(register struct offend_info *offend) {
	u_int32 s_pos = 0;
	u_int16 s_ofs = 0;
	u_int16 best_ofs = 0xffffu;
	ioresult result = S_OK;
	//fprintf(vo_stderr,"Find offender %s:%d.%d: ",offend->libname,offend->segment,offend->offset);

	//printf("#1");
	memset (&resource, 0, sizeof(Resource));

#if 0
	//Open library based of libarary name and file name	
	sprintf(lfn,"S:SYS/%s.DL3",offend->libname);
	f=fopen(lfn,"rb");
	if (!f) {
		sprintf(lfn,"S:%s*",offend->libname);
		f=fopen(lfn,"rb");
		if (!f) {
			fprintf(vo_stderr,"Hmm, seems not %s, sorry.\n",lfn);
			return S_ERROR;
		}
	}

#else		
	//Open library based on file start cluster and assume FatFileSystem
	f=fopen("S:","s");
	{
	  FatFileInfo *fi = (FatFileInfo *)(f->fileInfo);
		fi->startCluster = offend->libfileptr;
		fi->currentFragment.startSector = 0;
		fi->currentFragment.sizeBytes = 0;
		f->flags = __MASK_FILE | __MASK_PRESENT | __MASK_OPEN | __MASK_SEEKABLE | __MASK_READABLE;
		f->fileSize = -1;
		//printf("SC:%ld lfp:%ld fl:%04x",fi->startCluster,offend->libfileptr,f->flags);
	}
#endif

	//printf("#2");

	f->pos=0;
	if (fgetc(f) != 'V') return S_ERROR; //First byte of VDE1

	//printf("#3");

	
	f->pos=4;
	while(resource.header.resourceType != _RESTYPE_ENTRYLIST) {
		if (NextResource(f) == _RESTYPE_SYMBOL + offend->segment) {
			if ((offend->offset >= resource.header.subType) 
			&& (offend->offset - resource.header.subType < best_ofs)) {			
				best_ofs = offend->offset - resource.header.subType;
				s_pos = f->pos;
				s_ofs = resource.header.subType;
			}
		}
	}
	
	//printf("#4");

	if (s_pos) {
		char ch;
		printf(":");
		f->pos = s_pos;
		fread(&ch, 1, 1, f);
		if (ch != '_') fputc(ch,stdout);
		while(1) {
			fread(&ch, 1, 1, f);
			if (ch) {
				fputc(ch,stdout);
			} else {
				break;
			}
		}
		if (offend->offset - s_ofs) {
			printf("[0x%x]",offend->offset - s_ofs);
		}
		result = S_OK;	
		goto finally;
	}
	
	//printf("#5");

	printf("?");
	finally:
	fclose(f);
	
	//printf("#6");

	return result;
}


