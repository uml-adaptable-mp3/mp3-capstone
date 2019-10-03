#include <vo_stdio.h>
#include <vstypes.h>
#include <string.h>
#include <stdlib.h>
#include "parseFileParam.h"



auto const char *FileParamSeek(register const char *haystack,
			       register const char *needle,
			       register s_int16 *n) {
  int needleLen = strlen(needle);
  while (strlen(haystack)) {
    u_int16 doLook = 1;
    u_int32 newVal;
    const char *div;
    const char *equal;
    u_int16 toAdd, len;

    div = strchr(haystack, '/');
    if (div) {
      toAdd = div-haystack+1;
      len = div-haystack;
    } else {
      toAdd = len = strlen(haystack);
    }
    equal = strchr(haystack, '=');
    if (equal && equal-haystack == needleLen &&
	!memcmp(haystack, needle, equal-haystack)) {
      if (n)
	*n = len-needleLen-1;
      return equal+1;
    }
    haystack += toAdd;
  }

  return NULL;
}




auto u_int32 FileParamInt(register const char *haystack,
			  register const char *needle,
			  register u_int32 defaultVal) {
  const char *c = FileParamSeek(haystack, needle, NULL);
  return c ? strtol(c, NULL, 10) : defaultVal;
}




auto s_int16 FileParamStr(register const char *haystack,
			  register const char *needle,
			  register char *dst,
			  register s_int16 n) {
  const char *c;
  s_int16 nNeeded = 0;
  s_int16 elems = 0;
  s_int16 mult = 1;

  *dst = '\0';

  c = FileParamSeek(haystack, needle, &nNeeded);

  // Because we initialized nNeeded and FileParamSeek() updates it only
  // if it succeeds, we don't need to check for if c == NULL.

  if (nNeeded) {
    char *tp;
    elems = 1;
    if (nNeeded >= n) {
      n--;
      mult = -1;
    } else {
      n = nNeeded;
    }
    strncpy(dst, c, n);
    dst[n] = '\0';
    while (tp = strrchr(dst, ',')) {
      *tp = '\0';
      elems++;
    }
  }

  return mult * elems;
}
