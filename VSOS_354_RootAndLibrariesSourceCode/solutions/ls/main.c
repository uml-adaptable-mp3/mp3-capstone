/* For free support for VSIDE, please visit www.vsdsp-forum.com */

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <consolestate.h>
#include <vo_fat.h>
#include <string.h>
#include <swap.h>
#include <kernel.h>


#define NAMELENGTH 255

u_int32 FoundFileSize(VO_FILE *hSearch) {
   DirectoryEntry *de = &hSearch->sectorBuffer[((hSearch->pos-32) >> 1) & 0xff];
   return Swap32Mix(*(u_int32*)(&de->fileSizeLo));
}

void ListFiles(char *path) {
	u_int16 n=0;
	static char filename[256];		
	FILE *f = fopen(path,"s"); //Get search handle for the disk
	if (f) {
		path += 2; //Remove drive letter and colon from path
		if (FatFindFirst(f,path,filename,NAMELENGTH) == S_OK) {
			do {
				n++;
				if (f->ungetc_buffer & 0x10) {
					printf("[%s] ",f->extraInfo);
				}
			} while (S_OK == FatFindNext(f,filename,255));
		} else {
			printf("Path not found\n");
			return;
		}
		fclose(f);
		path -= 2;
	}
	f = fopen(path,"s"); //Get search handle for the disk
	if (f) {
		path += 2; //Remove drive letter and colon from path
		if (FatFindFirst(f,path,filename,NAMELENGTH) == S_OK) {
			do {
				n++;
				if (!(f->ungetc_buffer & 0x10)) {
					printf("%s ",f->extraInfo);
				}
			} while (S_OK == FatFindNext(f,filename,255));
		} else {
			printf("Path not found\n");
		}
		fclose(f);
	}
}


ioresult main(char *parameters) 
{
	if (strlen(parameters)>1) 
    {
		char e = parameters[strlen(parameters)-1];
		//printf("E:%c ",e);
		if ((e != ':') && (e != '/')) 
        {
			printf("'%s' is not a valid path (did you mean '%s/'?)\n",parameters,parameters);
			return S_ERROR;
		}
		ListFiles(parameters);
	} 
    else 
    {
		ListFiles(currentDirectory);
	}
	printf("\n");
	return S_OK;
}

