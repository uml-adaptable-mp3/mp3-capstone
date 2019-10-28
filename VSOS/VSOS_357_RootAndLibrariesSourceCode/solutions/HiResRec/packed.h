#ifndef PACKED_STRING_H
#define PACKED_STRING_H

#include <vstypes.h>

register __i0 *memcpypack(register __i0 u_int16 *d, const register __i1 u_int16 *s, size_t register __a0 n);
register __i0 *memcpyunpack(register __i0 u_int16 *d, const register __i1 u_int16 *s, size_t register __a0 n);
size_t register __a0 strlenpacked(const char register __i0 *s);

#define strcpypack(d,s) (memcpypack((d),(s),strlen(s)+1))
#define strcpyunpack(d,s) (memcpyunpack((d),(s),strlenpacked(s)+1))

#endif /* !PACKED_STRING_H */
