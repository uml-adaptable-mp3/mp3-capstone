#include <vo_stdio.h>
#include <volink.h>     // Linker directives like DLLENTRY
#include <apploader.h>  // RunLibraryFunction etc
#include <string.h>
#include <stdlib.h>
#include <copyfile.h>
#include <sysmemory.h>
#include <kernel.h>


#define MIN(a,b) (((a)<(b))?(a):(b))

/* Returns position of first differing byte, or -1 if buffers are similar. */
s_int16 CompareBuffers(register u_int16 *b1, register u_int16 *b2,
		       register u_int16 bytes) {
  u_int16 diffPos = 0;

  /* Quick check */
  if (!memcmp(b1, b2, (bytes+1)/2)) {
    return -1;
  }

  /* Ok, there may be a difference; let's check it out. Do full word
     comparisons. */
  while (bytes > 1) {
    u_int16 diff = *b1++ ^ *b2++;
    if (diff) {
      if (diff & 0xFF00U) {
	return diffPos;
      } else {
	return diffPos+1;
      }
    }
    diffPos += 2;
    bytes -= 2;
  }

  /* Check potential dangling byte */
  if (bytes) {
    if ((*b1++ ^ *b2++) & 0xFF00) {
      return diffPos;
    }
  }

  return -1;
}



DLLENTRY(main)
ioresult main(char *parameters) {
  int nParam = RunProgram("ParamSpl", parameters);
  char *p = parameters, *f1Name=NULL, *f2Name=NULL;
  int i, verbose = 0, f1r, f2r;
  ioresult retVal = S_ERROR;
  FILE *f1=NULL, *f2=NULL;
  int minR;
  u_int16 bufferSize=24576;
  u_int16 *buffer=NULL;

  for (i=0; i<nParam; i++) {
    if (!strcmp(p, "-h")) {
      printf("Usage: Cmp [-h] [f1 f2]\n"
	     "f1\tFile 1\n"
	     "f2\tFile 2\n"
	     "-h\tShow this help\n");
      retVal = S_OK;
      goto finally;
    } else if (!strcmp(p, "-v")) {
      verbose = 1;
    } else {
      if (!f1Name) {
	f1Name = p;
      } else if (!f2Name) {
	f2Name = p;
      } else {
	printf("E: Extraneous parameter \"%s\"\n", p);
	goto finally;
      }
    }
    p += strlen(p)+1;
  }

  if (!f2Name) {
    printf("E: File name(s) missing\n");
    goto finally;
  }

  if (!(f1 = fopen(f1Name, "rb"))) {
    printf("E: Couldn't open \"%s\"\n", f1Name);
    goto finally;
  }

  if (!(f2 = fopen(f2Name, "rb"))) {
    printf("E: Couldn't open \"%s\"\n", f2Name);
    goto finally;
  }

  /* Try to allocate all available memory to make file comparisons faster. */
  while (!(buffer = malloc(bufferSize))) {
    if ((bufferSize-=128) < 1024) {
      printf("E: Couldn't reserve buffers\n");
      goto finally;
    }
  }

  /* Free 0.5 KiW for VSOS for opening files */
  bufferSize -= 512;
  buffer = realloc(buffer, bufferSize);

  do {
    u_int32 oldPos = f1->pos;
    s_int16 diffPos;
    f1r = f1->op->Read(f1, buffer,          0, bufferSize);
    f2r = f2->op->Read(f2, buffer, bufferSize, bufferSize);
    minR = MIN(f1r, f2r);
    if ((diffPos = CompareBuffers(buffer, buffer+bufferSize/2, minR)) >= 0) {
      printf("cmp: \"%s\" and \"%s\" differ: byte %ld\n",
	     f1Name, f2Name, oldPos+diffPos+1);
      retVal = S_OK;
      goto finally;
    }
    //printf("diffPos = %d\n", diffPos);
  } while (minR >= bufferSize);

  if (f1r < f2r) {
    printf("cmp: EOF on \"%s\": byte %ld\n", f1Name, f1->pos);
  }

  if (f2r < f1r) {
    printf("cmp: EOF on \"%s\": byte %ld\n", f2Name, f2->pos);
  }

  retVal = S_OK;
 finally:
  if (buffer) {
    free(buffer);
    buffer = NULL;
  }
  if (f2) {
    fclose(f2);
    f2 = NULL;
  }
  if (f1) {
    fclose(f1);
    f1 = NULL;
  }
  return retVal;
}
