#include <vo_stdio.h>
#include <string.h>
#include <stdlib.h>

/* parseFileParam.c */
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


/* extSymbols.c */
u_int16 __mem_y extSymbolSearchRom = 0;
u_int16 extSymbolRomSize = 0;
