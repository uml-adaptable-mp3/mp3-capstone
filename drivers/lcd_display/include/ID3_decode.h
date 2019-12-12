#ifndef _ID3_DECODE_H_
#define _ID3_DECODE_H_

#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <volink.h>
#include <uimessages.h>
#include <string.h>
#include <kernel.h>


void DecodeID3(register FILE* fp, register UICallback callback);
ioresult PrintID3f(FILE *fp);

#endif //
