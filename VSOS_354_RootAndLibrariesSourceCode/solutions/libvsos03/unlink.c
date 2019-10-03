#include <vo_stdio.h>
#include <vo_fat.h>
#include <unistd.h>

/* unlink returns 0 on success, -1 on failure */
int unlink(const char *pathname) {
  FILE *fp;

  u_int16 t;

  if (!(fp = fopen(pathname, "rb"))) {
    return -1;
  }
  t = fp->ungetc_buffer;
  fclose(fp);

  if (t & __ATTR_DIRECTORY) {
    /* Don't allow removing a directory (it may have resources
       allocated to it). */
    return -1;
  }

  fp = fopen(pathname, "U");
  if (fp) {
    fclose(fp);
    return 0;
  }
  return -1;
}
