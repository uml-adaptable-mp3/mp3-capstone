#include <vo_stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <vstypes.h>
#include <vs1005h.h>
#include <clockspeed.h>
#include <audio.h>
#include <timers.h>
#include <crc32.h>
#include <apploader.h>
#include <kernel.h>


#define MIN(a,b) (((a)<(b))?(a):(b))




u_int16 buf[256];

ioresult CalcCrc32File(register char *fileName, register u_int16 verbose,
		       register u_int16 calc) {
  ioresult ret = S_ERROR;
  FILE *fp = fopen(fileName, "rb");
  u_int32 bytes = 0, bytesLeft = 0;
  u_int32 crc32 = 0;
  u_int32 startTime, totTime;

  if (!fp) {
    printf("E: File \"%s\" not found\n", fileName);
    goto finally;
  }

  fseek(fp, 0, SEEK_END);
  bytes = bytesLeft = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  printf("BYTES 0x%08lx\n", bytes);

  startTime = ReadTimeCount();

  while (bytesLeft > 1) {
    u_int16 toRead = (u_int16)MIN(bytesLeft, 512) >> 1;
    fread(buf, 1, toRead, fp);
    if (calc) crc32 = CalcCrc32(crc32, buf, 2*toRead);
    bytesLeft -= 2*toRead;
  }
  if (bytesLeft && calc) {
    char c = fgetc(fp);
    crc32 = CalcCrc32Byte(crc32, c);
  }

  totTime = ReadTimeCount() - startTime;
  if (!totTime) totTime++;

  if (calc) printf("CRC32 0x%08lx\n", crc32);
  if (verbose) {
    printf("TIME  %d.%03d s\n", (int)(totTime/1000), (int)(totTime%1000));
    printf("SPEED %d KB/s\n", (int)(bytes/totTime));
  }
  ret = S_OK;

 finally:
  if (fp) {
    fclose(fp);
    fp = NULL;
  }
  return ret;
}


ioresult main(char *parameters) {
  int nParam, i;
  char *p = parameters;
  ioresult ret = S_ERROR;
  int verbose = 0, calc = 1;

  nParam = RunProgram("ParamSpl", parameters);
  for (i=0; i<nParam; i++) {
    if (!strcmp(p, "-h")) {
      printf("Usage: Crc32 [-v|+v|-i|+i|-h] file1 [...]\n"
	     "-v|+v\tVerbose on|off\n"
	     "-i|+i\tIgnore CRC32 calculation on|off\n"
	     "-h\tShow this help\n");
      ret = S_OK;
      goto finally;
    } else if (!strcmp(p, "-v")) {
      verbose = 1;
    } else if (!strcmp(p, "+v")) {
      verbose = 0;
    } else if (!strcmp(p, "-i")) {
      calc = 0;
    } else if (!strcmp(p, "+i")) {
      calc = 1;
    } else {
      if (CalcCrc32File(p, verbose, calc)) {
	goto finally;
      }
    }
    p += strlen(p)+1;
  }

  ret = S_OK;

 finally:
  return ret;
}
