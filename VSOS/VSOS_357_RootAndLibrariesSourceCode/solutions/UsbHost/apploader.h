#ifndef APPLOADER_H
#define APPLOADER_H

#include "vo_stdio.h"
#include <vstypes.h>
#include "vs1005h.h"
#include "vsos.h"

#define _RESTYPE_STRING 0x0100
#define _RES_TITLE 1
#define _RESTYPE_BITMAP 0x0200
#define _RES_ICON 1


extern VO_FILE *appFile;

struct appHeaderStruct {
	u_int32 VLE5;
	u_int32 bootRecordStart;
	u_int16 entryAddress;
	u_int16 compatibility;
};

struct resourceHeaderStruct {
	u_int16 resourceType;
	u_int16 subType;
	u_int32 length;
};

int RunAppFile (int i);

#endif
