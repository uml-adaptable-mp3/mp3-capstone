#include <vo_stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysmemory.h>
#include <lists.h>
#include <timers.h>
#include <exec.h>
#include "memstore.h"


struct LIST *myiFlist = (void *)0x28fc;
struct LIST *myxFlist = (void *)0x593;
struct LISTY __mem_y *myyFlist = (void *)0x402;

struct MEMORYNODE {
	struct MINNODE node;
	u_int16 size;
};

struct MEMORYNODEY {
	struct MINNODEY node;
	u_int16 size;
};

#if 0
void PrintFreeMem(void) {
	u_int16 elemTot = 0;
#if 0
	myIFlist = *pIFlist;
	printf("  # PrintFreeMem I %p, x %p %p, y %p %p, 1234 %d\n",
	       myIFlist, &xFlist, xFlist, &yFlist, yFlist, 1234);
#endif
	{
		void *m = HeadNodeI(myiFlist);
		while (m) {
			printf("I %04x .. %04x\n", IMEMADDR(m), IMEMADDR(m)+GetSizeNodeI(m)-1);
			m = NextNodeI(m);			
		}
	}
	{
		struct MEMORYNODE *m = (void *)HeadNode(myxFlist);
		while (m) {
			printf("X %04x .. %04x\n", XMEMADDR(m), XMEMADDR(m)+m->size-1);
			m = (void *)NextNode((void *)m);
		}
	}
	{
		struct MEMORYNODEY __mem_y *m = (void __mem_y *)HeadNodeY(myyFlist);
		while (m) {
			printf("Y %04x .. %04x\n", YMEMADDR(m), YMEMADDR(m)+m->size-1);
			m = (void __mem_y *)NextNodeY((void __mem_y *)m);
		}
	}
	//	Delay(1000);
}
#endif
