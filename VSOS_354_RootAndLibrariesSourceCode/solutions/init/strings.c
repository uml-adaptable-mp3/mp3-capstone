#include <vstypes.h>
#include <ctype.h>
#include "strings.h"

#if 0
int ffs(int t) {
  return ffsl(t);
}
#endif

#if 0
int ffsl(register __reg_b long int t) {
  int res = 1;
  if (!t)
    return 0;
  while (!(t&1)) {
    t>>=1;
    res++;
  }
  return res;
}
#endif

auto int strcasecmp(register const char *s1, register const char *s2) {
  u_int16 c1;
  u_int16 c2;
  do {
    c1 = tolower(*s1);
    s1++;
    c2 = tolower(*s2);
    s2++;
    if (c1 != c2)
      return (int)c1-(int)c2;
  } while (c1);
  return 0;
}

auto int strncasecmp(register const char *s1, register const char *s2,
		     register size_t n) {
  u_int16 c1;
  u_int16 c2;
  if (n <= 0)
    return 0;
  do {
    c1 = tolower(*s1);
    s1++;
    c2 = tolower(*s2);
    s2++;
    if (c1 != c2) {
      return (int)c1-(int)c2;
    }
  } while (c1 && --n);
  return 0;
}
