#ifndef MEM_STORE_H
#define MEM_STORE_H

#include <vstypes.h>

struct MemElement {
	u_int16 addr;
	u_int16 size;
};

struct MemMap {
	u_int16 iElems;
	u_int16 xElems;
	u_int16 yElems;
	struct MemElement elem[1];
};

struct MemMap __mem_y *StoreMemMap(void);
int RestoreMemMap(register struct MemMap __mem_y *memMap);

/* Debug info: print current memory free areas */
void PrintFreeMem(void);

#endif /* MEM_STORE_H */
