#include <vo_stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysmemory.h>
#include <lists.h>
#include <timers.h>
#include <exec.h>
#include "memstore.h"


extern struct LIST xFlist;
extern struct LISTY __mem_y yFlist;
void iFlist(void);

struct MEMORYNODE {
	struct MINNODE node;
	u_int16 size;
};

struct MEMORYNODEY {
	struct MINNODEY node;
	u_int16 size;
};

int ActualStoreMemMap(register struct MemMap __mem_y *memMap) {
	u_int16 elemTot = 0;
	struct MemElement __mem_y *me = NULL;
	if (memMap) {
		me = memMap->elem;
	}
	{
		void *m = HeadNodeI(&iFlist);
		while (m) {
			if (me) {
				me->addr = IMEMADDR(m);
				me->size = GetSizeNodeI(m);
				me++;
				memMap->iElems++;
			}
//			printf("I %04x .. %04x\n", IMEMADDR(m), IMEMADDR(m)+GetSizeNodeI(m)-1);
			elemTot++;
			m = NextNodeI(m);			
		}
	}
	{
		struct MEMORYNODE *m = (void *)HeadNode(&xFlist);
		while (m) {
			if (me) {
				me->addr = XMEMADDR(m);
				me->size = m->size;
				me++;
				memMap->xElems++;
			}
//			printf("X %04x .. %04x\n", XMEMADDR(m), XMEMADDR(m)+m->size-1);
			elemTot++;
			m = (void *)NextNode((void *)m);
		}
	}
	{
		struct MEMORYNODEY __mem_y *m = (void __mem_y *)HeadNodeY(&yFlist);
		while (m) {
			if (me) {
				me->addr = YMEMADDR(m);
				me->size = m->size;
				me++;
				memMap->yElems++;
			}
//			printf("Y %04x .. %04x\n", YMEMADDR(m), YMEMADDR(m)+m->size-1);
			elemTot++;
			m = (void __mem_y *)NextNodeY((void __mem_y *)m);
		}
	}
//	printf("elemTot %d\n", elemTot);
	return elemTot;
}


#if 1
void PrintFreeMem(void) {
	u_int16 elemTot = 0;
	{
		void *m = HeadNodeI(&iFlist);
		while (m) {
			printf("I %04x .. %04x\n", IMEMADDR(m), IMEMADDR(m)+GetSizeNodeI(m)-1);
			m = NextNodeI(m);			
		}
	}
	{
		struct MEMORYNODE *m = (void *)HeadNode(&xFlist);
		while (m) {
			printf("X %04x .. %04x\n", XMEMADDR(m), XMEMADDR(m)+m->size-1);
			m = (void *)NextNode((void *)m);
		}
	}
	{
		struct MEMORYNODEY __mem_y *m = (void __mem_y *)HeadNodeY(&yFlist);
		while (m) {
			printf("Y %04x .. %04x\n", YMEMADDR(m), YMEMADDR(m)+m->size-1);
			m = (void __mem_y *)NextNodeY((void __mem_y *)m);
		}
	}
	Delay(1000);
}
#endif


struct MemMap __mem_y *StoreMemMap(void) {
	struct MemMap __mem_y *mm = NULL;
	u_int16 n;
	Forbid();
	n = ActualStoreMemMap(NULL);
	if (!n) {
		return NULL;
	}
	mm = callocy(1, sizeof(struct MemMap) + n*(sizeof(struct MemElement)));
	if (!mm) {
		return NULL;
	}
//	printf("mmSize1.1 = %d\n", *(((int __mem_y *)mm)-1));
	ActualStoreMemMap(mm);
//	printf("mmSize1.2 = %d\n", *(((int __mem_y *)mm)-1));
	Permit();
	return mm;
}

int RestoreMemMap(register struct MemMap __mem_y *memMap) {
	struct MemElement __mem_y *me;
	int i;
	if (!memMap) {
//		printf("No memmap\n");
		return -1;
	}
	Forbid();
//	ActualStoreMemMap(NULL);
	me = memMap->elem;
	__InitMemAllocI((void *)(me->addr), me->size);
	me++;
	for (i=1; i<memMap->iElems; i++) {
		void *mn = (void *)(me->addr);
		SetSizeNodeI(mn, me->size);
		AddTailI(&iFlist, (void *)mn);
		me++;
	}
	InitMemAlloc((void *)(me->addr), me->size, (__mem_y void *)(me[memMap->xElems].addr), me[memMap->xElems].size);
	me++;
	for (i=1; i<memMap->xElems; i++) {
		struct MEMORYNODE *mn = (void *)(me->addr);
		mn->size = me->size;
		AddTail(&xFlist, (void *)mn);
		me++;
	}
	me++;
	for (i=1; i<memMap->yElems; i++) {
		struct MEMORYNODE __mem_y *mn = (void __mem_y *)(me->addr);
		mn->size = me->size;
		AddTailY(&yFlist, (void __mem_y *)mn);
		me++;
	}
//	ActualStoreMemMap(NULL);
	freey(memMap);
	Permit();
//	ActualStoreMemMap(NULL);
	return 0;
}
