#ifndef ITYPE_H
#define ITYPE_H

auto int iisalnum(int register __i0 c);
auto int iisalpha(int register __i0 c);
auto int iiscntrl(int register __i0 c);
auto int iisdigit(int register __i0 c);
auto int iisgraph(int register __i0 c);
auto int iislower(int register __i0 c);
auto int iisprint(int register __i0 c);
auto int iispunct(int register __i0 c);
auto int iisspace(int register __i0 c);
auto int iisupper(int register __i0 c);
auto int iisxdigit(int register __i0 c);
auto int itolower(int register __i0 c);
auto int itoupper(int register __i0 c);

#define isalnum(a) iisalnum(a)
#define isalpha(a) iisalpha(a)
#define iscntrl(a) iiscntrl(a)
#define isdigit(a) iisdigit(a)
#define isgraph(a) iisgraph(a)
#define islower(a) iislower(a)
#define isprint(a) iisprint(a)
#define ispunct(a) iispunct(a)
#define isspace(a) iisspace(a)
#define isupper(a) iisupper(a)
#define isxdigit(a) iisxdigit(a)
#define tolower(a) itolower(a)
#define toupper(a) itoupper(a)

#endif /* ITYPE_H */
