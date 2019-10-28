#if 0
#define DEBUG_IT
#endif
#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <vstypes.h>
#include <kernel.h>
#include <libaudiodec.h>

FILE *fp;

#define MKID1(a,b) (((u_int16)(a)<<8)|(b))
#define MKID(a,b,c,d) (((u_int32)(((u_int16)(a)<<8)|(b))<<16)|(((u_int16)(c)<<8)|(d)))


/* atoms with subatoms */
extern const u_int32 mp4SubAtomTypes[];

u_int32 ReadUInt(void) {
    u_int32 val = 0;
    if (fp->op->Read(fp, (void *)&val, 0, 4) < 4) {
#ifdef DEBUG_IT
	printf("ReadUInt failed!\n");
#endif
//	alac.invalid = 1;
	return 0;
    }
    return (val<<16)|(val>>16);
}
u_int16 ReadUWord(void) {
    u_int16 val = 0;
    if (fp->op->Read(fp, (void *)&val, 0, 2) < 2) {
#ifdef DEBUG_IT
	printf("ReadUWord failed!\n");
#endif
	//alac.invalid = 1;
	return 0;
    }
    return val;
}
u_int16 ReadUByte(void) {
    u_int16 val = 0;
    if (fp->op->Read(fp, (void *)&val, 1, 1) < 1) {
#ifdef DEBUG_IT
	printf("ReadUByte failed!\n");
#endif
	//alac.invalid = 1;
	return 0;
    }
    return val;
}





typedef unsigned char u_int8;
typedef struct ALACSpecificConfig {
    u_int32	frameLength;
    u_int8	compatibleVersion:8;
    u_int8	bitDepth:8;
    u_int8	pb:8;
    u_int8	mb:8;
    u_int8	kb:8;
    u_int8	numChannels:8;
    u_int16	maxRun;
    u_int32	maxFrameBytes;
    u_int32	avgBitRate;
    u_int32	sampleRate;
} ALACSpecificConfig;
#define AOT_ALAC 0xa1ac



enum AuDecFormat mp4StsdFile(register u_int32 *totalSize) {
    u_int16 header1, header2;
    __mem_y s_int32 entry_count;
    register int i;

    ReadUInt(); /* version 8 + flags 24 */

    entry_count = ReadUInt();
    *totalSize -= 8;

    for (i = 0; i < entry_count; i++) {
        u_int32 size;

	size    = ReadUInt();
	header1 = ReadUWord();
	header2 = ReadUWord();
	*totalSize -= size;
	size -= 8;
	if (header1 == MKID1('a','l') && header2 == MKID1('a','c')) {
#ifdef DEBUG_IT
	    printf("Handle alac\n");
#endif
	    /******** THIS IS ALAC ********/
	    return auDecFAlac;
        } else if (header1 == MKID1('m','p') && header2 == MKID1('4','a')) {
#ifdef DEBUG_IT
	    printf("Handle aac\n");
#endif
	    return auDecFAac;
	}

	if (size) {
#ifdef DEBUG_IT
	    printf("skip %ld\n", size);
#endif
	    fseek(fp, size, SEEK_CUR);
	}
    }
#ifdef DEBUG_IT
    printf("stsd end %ld\n", *totalSize);
#endif

    return auDecFGuess; /* Unknown */
}


s_int16 mp4AtomHasSubAtoms(register const u_int16 h1, register const u_int16 h2) {
    register const u_int32 *atomTypes = &mp4SubAtomTypes[0];
    for (; *atomTypes; atomTypes++)
	if (*atomTypes == (((u_int32)h1<<16)|h2))
	    return 1;
    return 0;
}

auto enum AuDecFormat mp4ParseSubAtomsFile(register u_int32 *totSize) {
    u_int32 size;
    u_int32 header;
    enum AuDecFormat fmt = auDecFGuess;

    while (*totSize) {
	size   = ReadUInt();
	header = ReadUInt();
	if (size > *totSize) {
	    return; //20130116 check validity..
	}
	*totSize -= size;
	size -= 8;
	if (header == MKID('s','t','s','d')) {
#ifdef DEBUG_IT
	    printf("handling stsd\n");
#endif
	    return mp4StsdFile(&size);
#ifdef DEBUG_IT
	    printf("stsd<-\n");
#endif
	} else if (mp4AtomHasSubAtoms(header>>16, header)) {
	    /*
	      atom_read/trak
	        stsd
	          mp4a
	            esds
	              decoderconfig
		  alac
	    */
#ifdef DEBUG_IT
	    printf("** reading %c%c%c%c\n",
		   (u_int16)(header>>24), (u_int16)(header>>16)&255,
		   (u_int16)(header>>8)&255, (u_int16)header&255);
#endif
	    if ((fmt = mp4ParseSubAtomsFile(&size)) != auDecFGuess) {
	      return fmt;
	    }
#ifdef DEBUG_IT
	    printf("sub<-%c%c%c%c\n",
		   (u_int16)(header>>24), (u_int16)(header>>16)&255,
		   (u_int16)(header>>8)&255, (u_int16)header&255);
#endif
	}
	/* Check that it is something sensible. */
	if (size >= 0x01000000) {
	    return auDecFGuess;
	}

	if (size) {
#ifdef DEBUG_IT
	    printf("a: skipping %ld\n", size);
	    fflush(stdout);
#endif
	    fseek(fp, size, SEEK_CUR);
	}
    }

 finally:
    return fmt;
}













DLLENTRY(AlacDecodeMp4File)
enum AuDecFormat AlacDecodeMp4File(FILE *fpParam) {
  u_int32 origPos, size, header;
  enum AuDecFormat fmt = auDecFGuess;
    fp = fpParam;
    origPos = ftell(fp);
    fseek(fp, -8, SEEK_CUR);
    size = ReadUInt();
    header = ReadUInt();

#if 0
    printf("SIZE %08lx, HEAD %08lx\n", size, header);
#endif

    while (1) {
#if 0
	if (header == MKID('m','d','a','t')) {
#ifdef DEBUG_IT
	    printf("got mdat\n");
#endif
	    break;
	}
#endif

	if (header == 0) {
#ifdef DEBUG_IT
	    printf("got 0 header! Leaving!\n");
#endif
	    break;
	}
	size -= 8;
	if (header == MKID('m','o','o','v')) {
#ifdef DEBUG_IT
	    printf("parse subatoms %ld %c%c%c%c\n",
		   size,
		   (u_int16)(header>>24), (u_int16)(header>>16) & 255,
		   (u_int16)(header>>8)&255, (u_int16)header&255);
#endif
	    if ((fmt = mp4ParseSubAtomsFile(&size)) != auDecFGuess) {
	      goto finally;
	    }

#ifdef DEBUG_IT
	    printf("<-subatoms %ld\n", size);
#endif
	}
	if (size >= 0x01000000) {
#ifdef DEBUG_IT
	    printf("too large! %ld\n", size);
#endif
	    return;
	}
	if (size) {
#ifdef DEBUG_IT
	    printf("b: skipping %ld\n", size);
#endif
	    fseek(fp, size, SEEK_CUR);
	}
	size   = ReadUInt();
	header = ReadUInt();
#ifdef DEBUG_IT
	//printf("sz %08lx hdr %08lx\n", size, header);
#endif
    }
#ifdef DEBUG_IT
    printf("Done with header, got %08lx\n", header);
#endif

 finally:
    fseek(fp, origPos, SEEK_SET);
    return fmt;
}







/* atoms with subatoms */
const u_int32 mp4SubAtomTypes[] = {
    MKID('m','o','o','v'),
    MKID('t','r','a','k'),
    MKID('m','d','i','a'),
    MKID('m','i','n','f'),
    MKID('s','t','b','l'),
//    MKID('u','d','t','a'),
    0
};


