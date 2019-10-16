#include "codMpg.h"

struct newhuff {
  u_int16 const linbits;
  const s_int16 __y *table;
};

extern const struct newhuff __y codMpgHt[];
extern const struct newhuff __y codMpgHtc[];
