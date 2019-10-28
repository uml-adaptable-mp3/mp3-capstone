#include <vo_stdio.h>
#include <vsos.h>
#include <string.h>
#include <unistd.h>
#include <vo_fat.h>
#include <vo_fatdirops.h>
#include <audio.h>
#include <timers.h>
#include <sysmemory.h>
#include <copy.h>

#define MIN(a,b) (((a)<(b))?(a):(b))

#define BUFFER_BYTES 2048

ioresult CopyFile(const char *d, const char *s) {
  FILE *rp=NULL, *wp=NULL;
  ioresult errCode = S_ERROR;
  u_int32 bytesLeft;
  u_int16 *buf = malloc(BUFFER_BYTES/2);

  if (!buf) {
    errCode = E_OUT_OF_MEMORY;
    goto finally;
  }

  if (!(rp = fopen(s, "rb"))) {
    errCode = E_FILE_NOT_FOUND;
    goto finally;
  }

  if (fseek(rp, 0, SEEK_END)) {
    errCode = S_ERROR;
    goto finally;
  }
  bytesLeft = ftell(rp);
  if (fseek(rp, 0, SEEK_SET)) {
    errCode = S_ERROR;
    goto finally;
  }

  if (!(wp = fopen(d, "wb"))) {
    errCode = E_CANNOT_CREATE_FILE;
    goto finally;
  }

  while (bytesLeft > 1) {
    u_int16 words = ((bytesLeft > BUFFER_BYTES) ?
		     BUFFER_BYTES : (u_int16)bytesLeft) >> 1;
    if (fread(buf, sizeof(buf[0]), words, rp) != words ||
	fwrite(buf, sizeof(buf[0]), words, wp) != words) {
    errCode = S_ERROR;
    goto finally;
  }
    bytesLeft -= words*2;
  }
  if (bytesLeft) {
    int c = fgetc(rp);
    if (c == EOF || fputc(c, wp) == EOF) {
      errCode = S_ERROR;
      goto finally;
    }
  }

  errCode = S_OK;

 finally:
  if (wp) {
    fclose(wp);
    wp = NULL;
  }

  if (rp) {
    fclose(rp);
    rp = NULL;
  }

  if (buf) {
    free(buf);
  }

  if (errCode != S_OK) {
    unlink(d);
  }

  return errCode;
}
