#ifndef __STRINGS_H__
#define __STRINGS_H__

#include <stdlib.h>
#include <vstypes.h>

#define bcmp(s1,s2,n) memcmp((s1),(s2),(n))
#define bcopy(s1,s2,n) memcpy((s2),(s1),(n))
#define bzero(s,n) memset((s),0,(n))
#define ffs(i) ffsl((u_int16)(i))
auto int ffsl(register __reg_b long int i);
#define index(s,c) strchr((s),0,(c))
#define rindex(s,c) strrchr((s),0,(c))
//auto int strcasecmp(register const char *s1, register const char *s2);
auto int strncasecmp(register const char *s1, register const char *s2,
		     register size_t n);
#define strcasecmp(a,b) strncasecmp((a),(b),-1)

#endif /* !__STRINGS_H__ */
